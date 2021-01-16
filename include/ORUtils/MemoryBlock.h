// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM

#pragma once

#include "MemoryDeviceType.h"
#include "PlatformIndependence.h"

#ifndef __METALC__

#ifndef COMPILE_WITHOUT_CUDA
#include "CUDADefines.h"
#endif

#ifdef COMPILE_WITH_METAL
#include "MetalContext.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <memory>

namespace ORUtils
{
	/** \brief
	Represents memory blocks, templated on the data type
	*/
	template <typename T>
	class MemoryBlock
	{
	protected:
		bool isAllocated_CPU, isAllocated_CUDA, isMetalCompatible;
		std::unique_ptr<std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>> lastUpdate_CPU, lastUpdate_CUDA;

		/** Pointer to memory on CPU host. */
		DEVICEPTR(T)* data_cpu;

		/** Pointer to memory on GPU, if available. */
		DEVICEPTR(T)* data_cuda;

#ifdef COMPILE_WITH_METAL
		void *data_metalBuffer;
#endif
	public:
		enum MemoryCopyDirection { CPU_TO_CPU, CPU_TO_CUDA, CUDA_TO_CPU, CUDA_TO_CUDA };

		bool hasCUDA() const {return isAllocated_CUDA;}
		bool hasCPU() const {return isAllocated_CPU;}

		/** Total number of allocated entries in the data array. */
		size_t dataSize;

		/** Get the data pointer on CPU or GPU. */
		inline DEVICEPTR(T)* GetData(MemoryDeviceType memoryType, bool check = true)
		{
#ifndef NDEBUG
            if(check) {
                checkAllocation(memoryType);
                checkIsMemoryUpToDate(memoryType);
            }
            UpdateTimeStamp(memoryType);
#endif
			switch (memoryType)
			{
			case MEMORYDEVICE_CPU: {
				return data_cpu;
			}
			case MEMORYDEVICE_CUDA: {
				return data_cuda;
			}
			}

			return 0;
		}

		/** Get the data pointer on CPU or GPU. */
		inline const DEVICEPTR(T)* GetData(MemoryDeviceType memoryType, bool check = true) const
		{
#ifndef NDEBUG
            if(check) {
                checkAllocation(memoryType);
                checkIsMemoryUpToDate(memoryType);
            }
#endif
			switch (memoryType)
			{
			case MEMORYDEVICE_CPU: return data_cpu;
			case MEMORYDEVICE_CUDA: return data_cuda;
			}

			return 0;
		}

        /** Get the data pointer on CPU or GPU. */
        inline const DEVICEPTR(T)* GetDataConst(MemoryDeviceType memoryType, bool check = true)
        {
#ifndef NDEBUG
            if(check) {
                checkAllocation(memoryType);
                checkIsMemoryUpToDate(memoryType);
            }
#endif
            switch (memoryType)
            {
                case MEMORYDEVICE_CPU: return data_cpu;
                case MEMORYDEVICE_CUDA: return data_cuda;
            }

            return 0;
        }

		/** Get the data pointer on CPU or GPU. */
		inline const DEVICEPTR(T)* GetDataConst(MemoryDeviceType memoryType, bool check = true) const
		{
#ifndef NDEBUG
			if(check) {
				checkAllocation(memoryType);
				checkIsMemoryUpToDate(memoryType);
			}
#endif
			switch (memoryType)
			{
				case MEMORYDEVICE_CPU: return data_cpu;
				case MEMORYDEVICE_CUDA: return data_cuda;
			}

			return 0;
		}
#ifdef COMPILE_WITH_METAL
		inline const void *GetMetalBuffer() const { return data_metalBuffer; }
#endif

		/** Initialize an empty memory block of the given size,
		on CPU only or GPU only or on both. CPU might also use the
		Metal compatible allocator (i.e. with 16384 alignment).
		*/
		MemoryBlock(size_t dataSize, bool allocate_CPU, bool allocate_CUDA, bool metalCompatible = true)
		{
		    auto time = std::chrono::steady_clock::now();
		    lastUpdate_CPU.reset(new std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>(time));
            lastUpdate_CUDA.reset(new std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>(time));

			this->isAllocated_CPU = false;
			this->isAllocated_CUDA = false;
			this->isMetalCompatible = false;

#ifndef NDEBUG // When building in debug mode always allocate both on the CPU and the GPU
			if (allocate_CUDA) allocate_CPU = true;
#endif

			Allocate(dataSize, allocate_CPU, allocate_CUDA, metalCompatible);
			Clear();
		}

		/** Initialize an empty memory block of the given size, either
		on CPU only or on GPU only. CPU will be Metal compatible if Metal
		is enabled.
		*/
		MemoryBlock(size_t dataSize, MemoryDeviceType memoryType)
		{
            auto time = std::chrono::steady_clock::now();
            lastUpdate_CPU.reset(new std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>(time));
            lastUpdate_CUDA.reset(new std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>>(time));
			this->isAllocated_CPU = false;
			this->isAllocated_CUDA = false;
			this->isMetalCompatible = false;

			switch (memoryType)
			{
				case MEMORYDEVICE_CPU: Allocate(dataSize, true, false, true); break;
				case MEMORYDEVICE_CUDA:
				{
#ifndef NDEBUG // When building in debug mode always allocate both on the CPU and the GPU
					Allocate(dataSize, true, true, true);
#else
					Allocate(dataSize, false, true, true);
#endif
					break;
				}
			}

			Clear();
		}

		/** Resize a memory block, losing all old data.
		Essentially any previously allocated data is
		released, new memory is allocated.
		*/
		void Resize(size_t newDataSize, bool forceReallocation = true)
		{
			if(newDataSize == dataSize) return;

			if(newDataSize > dataSize || forceReallocation)
			{
				bool allocate_CPU = this->isAllocated_CPU;
				bool allocate_CUDA = this->isAllocated_CUDA;
				bool metalCompatible = this->isMetalCompatible;

				this->Free();
				this->Allocate(newDataSize, allocate_CPU, allocate_CUDA, metalCompatible);
			}

			this->dataSize = newDataSize;
		}

        /** Transfer data from CPU to GPU, if possible. */
        void UpdateDeviceFromHost(bool async = false, void * stream=nullptr) const {
#ifndef COMPILE_WITHOUT_CUDA
            auto stream_ = static_cast<cudaStream_t>(stream);
            if (isAllocated_CUDA && isAllocated_CPU){
#ifndef NDEBUG
                checkIsMemoryUpToDate(cudaMemcpyHostToDevice);
                UpdateTimeStamp();
#endif
                if(async)
                    ORcudaSafeCall(cudaMemcpyAsync(data_cuda,data_cpu,dataSize * sizeof(T),cudaMemcpyHostToDevice,stream_));
                else
                    ORcudaSafeCall(cudaMemcpy(data_cuda, data_cpu, dataSize * sizeof(T), cudaMemcpyHostToDevice));

            }
            else {
                char str[256];
                sprintf(str,"called UpdateDeviceFromHost but not both memories are allocated: {isCPU:isCUDA} = {%d, %d}\n",
                        isAllocated_CPU, isAllocated_CUDA);
                throw std::runtime_error(str);
            }
#else
            std::runtime_error("called UpdateHostFromDevice but the system is not compiled with CUDA\n");
#endif
        }

        /** Transfer data from GPU to CPU, if possible. */
        void UpdateHostFromDevice(bool async = false, void * stream=nullptr) const {
#ifndef COMPILE_WITHOUT_CUDA
            auto stream_ = static_cast<cudaStream_t>(stream);
            if (isAllocated_CUDA && isAllocated_CPU) {
#ifndef NDEBUG
                checkIsMemoryUpToDate(cudaMemcpyDeviceToHost);
                UpdateTimeStamp();
#endif
                if(async)
                    ORcudaSafeCall(cudaMemcpyAsync(data_cpu, data_cuda, dataSize * sizeof(T), cudaMemcpyDeviceToHost,stream_));
                else
                    ORcudaSafeCall(cudaMemcpy(data_cpu, data_cuda, dataSize * sizeof(T), cudaMemcpyDeviceToHost));
            } else {
                char str[256];
                sprintf(str,"called UpdateHostFromDevice but not both memories are allocated: {isCPU:isCUDA} = {%d, %d}\n",
                        isAllocated_CPU, isAllocated_CUDA);
                throw std::runtime_error(str);
            }
#else
            std::runtime_error("called UpdateHostFromDevice but the system is not compiled with CUDA\n");
#endif
        }

        /** Set all image data to the given @p defaultValue. */
        void Clear(unsigned char defaultValue = 0, bool async = false, void *stream = nullptr, bool cpu=true, bool gpu=true)
        {
#ifndef NDEBUG
            SyncronizeTimeStamp();
#endif
            if(cpu) if (isAllocated_CPU) memset(data_cpu, defaultValue, dataSize * sizeof(T));
#ifndef COMPILE_WITHOUT_CUDA
            auto stream_ = static_cast<cudaStream_t>(stream);
            if(gpu) if (isAllocated_CUDA) {
                if(async)
                    ORcudaSafeCall(cudaMemsetAsync(data_cuda, defaultValue, dataSize * sizeof(T), stream_));
                else
                    ORcudaSafeCall(cudaMemset(data_cuda, defaultValue, dataSize * sizeof(T)));
            }
#endif
        }

        void SetTo(T value, bool async = false, void *stream = nullptr, bool cpu=true, bool gpu = true) {
            if(cpu) if (isAllocated_CPU) memset(data_cpu, value, dataSize * sizeof(T));
#ifndef COMPILE_WITHOUT_CUDA
            auto stream_ = static_cast<cudaStream_t>(stream);
            if(gpu) if (isAllocated_CUDA) {
                    if(async)
                        ORcudaSafeCall(cudaMemsetAsync(data_cuda, value, dataSize * sizeof(T), stream_));
                    else
                        ORcudaSafeCall(cudaMemset(data_cuda, value, dataSize * sizeof(T)));
                }
#endif
        }

        /** Copy data */
        void SetFrom(const MemoryBlock<T> *source, MemoryCopyDirection memoryCopyDirection, void *stream = nullptr)
        {
            Resize(source->dataSize);

#ifndef NDEBUG
            UpdateTimeStamp(memoryCopyDirection);
#endif

#ifndef COMPILE_WITHOUT_CUDA
            auto stream_ = static_cast<cudaStream_t>(stream);
#endif

            switch (memoryCopyDirection)
            {
                case CPU_TO_CPU:
                    memcpy(this->data_cpu, source->data_cpu, source->dataSize * sizeof(T));
                    break;
#ifndef COMPILE_WITHOUT_CUDA
                case CPU_TO_CUDA:
                    ORcudaSafeCall(cudaMemcpyAsync(this->data_cuda, source->data_cpu, source->dataSize * sizeof(T), cudaMemcpyHostToDevice,stream_));
                    break;
                case CUDA_TO_CPU:
                    ORcudaSafeCall(cudaMemcpy(this->data_cpu, source->data_cuda, source->dataSize * sizeof(T), cudaMemcpyDeviceToHost));
                    break;
                case CUDA_TO_CUDA:
                    ORcudaSafeCall(cudaMemcpyAsync(this->data_cuda, source->data_cuda, source->dataSize * sizeof(T), cudaMemcpyDeviceToDevice,stream_));
                    break;
#endif
                default:
                    std::runtime_error("called SetFrom with CopyDirection related to CUDA. But the system is not compiled with CUDA\n");
                    break;

            }
        }

//        /** Set data */
//        void SetTo(const T source, MemoryCopyDirection memoryCopyDirection) //TODO: Test me
//        {
//            switch (memoryCopyDirection)
//            {
//                case CPU_TO_CPU:
//                    memset(this->data_cpu, source, this->dataSize * sizeof(T));
//                    break;
//#ifndef COMPILE_WITHOUT_CUDA
//                case CPU_TO_CUDA:
//                    ORcudaSafeCall(cudaMemsetAsync(this->data_cuda, source, this->dataSize * sizeof(T)));
////                    ORcudaSafeCall(cudaMemcpyAsync(this->data_cuda, source->data_cpu, source->dataSize * sizeof(T), cudaMemcpyHostToDevice));
//                    break;
//                case CUDA_TO_CPU:
//                    ORcudaSafeCall(cudaMemsetAsync(this->data_cuda, source, this->dataSize * sizeof(T)));
////                    ORcudaSafeCall(cudaMemcpy(this->data_cpu, source->data_cuda, source->dataSize * sizeof(T), cudaMemcpyDeviceToHost));
//                    break;
//                case CUDA_TO_CUDA:
//                    ORcudaSafeCall(cudaMemsetAsync(this->data_cuda, source, this->dataSize * sizeof(T)));
////                    ORcudaSafeCall(cudaMemcpyAsync(this->data_cuda, source->data_cuda, source->dataSize * sizeof(T), cudaMemcpyDeviceToDevice));
//                    break;
//#endif
//                default: break;
//            }
//        }

		/** Get an individual element of the memory block from either the CPU or GPU. */
		T GetElement(int n, MemoryDeviceType memoryType) const
		{
			switch(memoryType)
			{
				case MEMORYDEVICE_CPU:
				{
					return this->data_cpu[n];
				}
#ifndef COMPILE_WITHOUT_CUDA
				case MEMORYDEVICE_CUDA:
				{
					T result;
					ORcudaSafeCall(cudaMemcpy(&result, this->data_cuda + n, sizeof(T), cudaMemcpyDeviceToHost));
					return result;
				}
#endif
				default: throw std::runtime_error("Invalid memory type");
			}
		}

		virtual ~MemoryBlock() { this->Free(); }

		/** Allocate image data of the specified size. If the
		data has been allocated before, the data is freed.
		*/
		void Allocate(size_t dataSize, bool allocate_CPU, bool allocate_CUDA, bool metalCompatible)
		{
			Free();

			this->dataSize = dataSize;

			if (allocate_CPU)
			{
				int allocType = 0;

#ifndef COMPILE_WITHOUT_CUDA
				if (allocate_CUDA) allocType = 1;
#endif
#ifdef COMPILE_WITH_METAL
				if (metalCompatible) allocType = 2;
#endif
				switch (allocType)
				{
				case 0:
					if (dataSize == 0) data_cpu = NULL;
					else data_cpu = new T[dataSize];
					break;
				case 1:
#ifndef COMPILE_WITHOUT_CUDA
					if (dataSize == 0) data_cpu = NULL;
					else ORcudaSafeCall(cudaMallocHost((void**)&data_cpu, dataSize * sizeof(T)));
#endif
					break;
				case 2:
#ifdef COMPILE_WITH_METAL
					if (dataSize == 0) data_cpu = NULL;
					else allocateMetalData((void**)&data_cpu, (void**)&data_metalBuffer, (int)(dataSize * sizeof(T)), true);
#endif
					break;
				}

				this->isAllocated_CPU = allocate_CPU;
				this->isMetalCompatible = metalCompatible;
			}

			if (allocate_CUDA)
			{
#ifndef COMPILE_WITHOUT_CUDA
				if (dataSize == 0) data_cuda = NULL;
				else ORcudaSafeCall(cudaMalloc((void**)&data_cuda, dataSize * sizeof(T)));
				this->isAllocated_CUDA = allocate_CUDA;
#endif
			}
		}

		void Free()
		{
			if (isAllocated_CPU)
			{
				int allocType = 0;

#ifndef COMPILE_WITHOUT_CUDA
				if (isAllocated_CUDA) allocType = 1;
#endif
#ifdef COMPILE_WITH_METAL
				if (isMetalCompatible) allocType = 2;
#endif
				switch (allocType)
				{
				case 0:
					if (data_cpu != NULL) delete[] data_cpu;
					break;
				case 1:
#ifndef COMPILE_WITHOUT_CUDA
					if (data_cpu != NULL) ORcudaSafeCall(cudaFreeHost(data_cpu));
#endif
					break;
				case 2:
#ifdef COMPILE_WITH_METAL
					if (data_cpu != NULL) freeMetalData((void**)&data_cpu, (void**)&data_metalBuffer, (int)(dataSize * sizeof(T)), true);
#endif
					break;
				}

				isMetalCompatible = false;
				isAllocated_CPU = false;
			}

			if (isAllocated_CUDA)
			{
#ifndef COMPILE_WITHOUT_CUDA
				if (data_cuda != NULL) ORcudaSafeCall(cudaFree(data_cuda));
#endif
				isAllocated_CUDA = false;
			}
		}

		void Swap(MemoryBlock<T>& rhs)
		{
			std::swap(this->dataSize, rhs.dataSize);
			std::swap(this->data_cpu, rhs.data_cpu);
			std::swap(this->data_cuda, rhs.data_cuda);
#ifdef COMPILE_WITH_METAL
			std::swap(this->data_metalBuffer, rhs.data_metalBuffer);
#endif
			std::swap(this->isAllocated_CPU, rhs.isAllocated_CPU);
			std::swap(this->isAllocated_CUDA, rhs.isAllocated_CUDA);
			std::swap(this->isMetalCompatible, rhs.isMetalCompatible);
		}

#ifndef NDEBUG
        /// This is used for align time steps in both CPU and GPU.
        void SyncronizeTimeStamp(){
            UpdateTimeStamp();
		}
#endif

		// Suppress the default copy constructor and assignment operator
		MemoryBlock(const MemoryBlock&);
		MemoryBlock& operator=(const MemoryBlock&);
	private:

#ifndef NDEBUG
		inline void checkAllocation(MemoryDeviceType memoryType) const {
			switch(memoryType){
				case MEMORYDEVICE_CPU:
					if(!isAllocated_CPU) throw std::runtime_error("Did not allocate CPU memory.\n");
					break;
				case MEMORYDEVICE_CUDA:
					if(!isAllocated_CUDA) throw std::runtime_error("Did not allocate CUDA memory.\n");
					break;
			}
		}
		inline void UpdateTimeStamp(MemoryDeviceType memoryType) {
            switch(memoryType){
                case MEMORYDEVICE_CPU:
                    *lastUpdate_CPU = std::chrono::steady_clock::now();
                    break;
                case MEMORYDEVICE_CUDA:
                    *lastUpdate_CUDA = std::chrono::steady_clock::now();
                    break;
            }
		}
        inline void UpdateTimeStamp() {
            *lastUpdate_CUDA = *lastUpdate_CPU = std::chrono::steady_clock::now();
            if(*lastUpdate_CUDA != *lastUpdate_CPU) throw std::runtime_error("error!\n");
        }
        inline void UpdateTimeStamp() const {
            *lastUpdate_CUDA = *lastUpdate_CPU = std::chrono::steady_clock::now();
            if(*lastUpdate_CUDA != *lastUpdate_CPU) throw std::runtime_error("error!\n");
        }
        inline void UpdateTimeStamp(MemoryCopyDirection memoryCopyDirection) const {
		    switch(memoryCopyDirection){
		        case CPU_TO_CPU:
                case CUDA_TO_CPU:
                    *lastUpdate_CPU = std::chrono::steady_clock::now();
		            break;
		        case CPU_TO_CUDA:
		        case CUDA_TO_CUDA:
                    *lastUpdate_CUDA = std::chrono::steady_clock::now();
		            break;
		    }
        }

		inline void checkIsMemoryUpToDate(MemoryDeviceType memoryType) const {
		    if(isAllocated_CPU && isAllocated_CUDA) {
		        switch(memoryType){
                    case MEMORYDEVICE_CPU:
                        if(*lastUpdate_CUDA > *lastUpdate_CPU) throw std::runtime_error(ErrorMsg(0));
                        break;
                    case MEMORYDEVICE_CUDA:
                        if(*lastUpdate_CPU > *lastUpdate_CUDA) throw std::runtime_error(ErrorMsg(1));
                        break;
                }
		    }
		}

#ifndef COMPILE_WITHOUT_CUDA
        inline void checkIsMemoryUpToDate(cudaMemcpyKind memoryCopyDirection) const {
            if(isAllocated_CPU && isAllocated_CUDA) {
                switch(memoryCopyDirection){
                    case cudaMemcpyHostToDevice:
                        if(*lastUpdate_CUDA > *lastUpdate_CPU)
                            throw std::runtime_error(ErrorMsg(2));
                        break;
                    case cudaMemcpyDeviceToHost:
                        if(*lastUpdate_CPU > *lastUpdate_CUDA)
                            throw std::runtime_error(ErrorMsg(3));
                        break;
                    default:
                        break;
                }
            }
        }
#endif

	private:
	    std::string ErrorMsg(int type) const{
		    char buffer[1024];
		    if(type == 0) sprintf(buffer,"[%p] Trying to access CPU memory while it has older time stamp than CUDA memory!", this);
		    if(type == 1) sprintf(buffer,"[%p] Trying to access CUDA memory while it has older time stamp than CPU memory!", this);
		    if(type == 2) sprintf(buffer,"[%p] Trying to update CUDA memory with CPU memory, but while CPU memory  has older time stamp than CUDA memory!", this);
		    if(type ==3 ) sprintf(buffer,"[%p] Trying to update CPU memory with CUDA memory, but while CUDA memory  has older time stamp than CPU memory!",this);

		    return std::string(buffer);
		}
#endif
	};

}

#endif
