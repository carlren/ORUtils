#pragma once
#include "PlatformIndependence.h"
#include "Matrix.h"
#include "Vector.h"

namespace ORUtils{
    enum MatrixDataOrder{
        RowMajor, ColMajor
    };

    template<MatrixDataOrder Order>
    _CPU_AND_GPU_CODE_ inline Vector3f RotMatTransform(const Matrix4f &rot, const Vector3f& pos);
    template<>
    _CPU_AND_GPU_CODE_ inline Vector3f RotMatTransform<MatrixDataOrder::RowMajor>(const Matrix4f &rot, const Vector3f& pos){
        Vector3f dst;
        for(size_t i=0;i < 3; ++i)
            dst[i] = rot.m[i*4] * pos.x + rot.m[i*4+1] * pos.y + rot.m[i*4+2] * pos.z + rot.m[i*4+3];
        return dst;
    }
    template<>
    _CPU_AND_GPU_CODE_ inline Vector3f RotMatTransform<MatrixDataOrder::ColMajor>(const Matrix4f &rot, const Vector3f& pos){
        Vector3f dst;
        for(size_t i=0;i < 3; ++i)dst[i] = rot.m[i] * pos.x + rot.m[i+4] * pos.y + rot.m[i+8] * pos.z + rot.m[i+12];
        return dst;
    }
//    template<class T>
//    _CPU_AND_GPU_CODE_ inline void RotMatTransform (const Matrix4f &rot, const Vector3f& pos, Vector3f& dst)
//    {
//        dst.x = rot.m[0] * pos.x + rot.m[1] * pos.y + rot.m[2] * pos.z + rot.m[3];
//        dst.y = rot.m[4] * pos.x + rot.m[5] * pos.y + rot.m[6] * pos.z + rot.m[7];
//        dst.z = rot.m[8] * pos.x + rot.m[9] * pos.y + rot.m[10] * pos.z + rot.m[11];
//    }

    template<MatrixDataOrder Order>
    _CPU_AND_GPU_CODE_ inline Vector3f invRotMatTransform(const Matrix4f &rot, const Vector3f& pos);
    template<>
    _CPU_AND_GPU_CODE_ inline Vector3f invRotMatTransform<MatrixDataOrder ::RowMajor>(const Matrix4f &rot, const Vector3<float>& pos)
    {
        Vector3<float> dst;
        Vector3<float> tmp;
        for(size_t i=0; i < 3; ++i)
            tmp[i] = pos[i] - rot.m [i+12];
        for(size_t i=0; i < 3; ++i)
            dst[i] = rot.m[i] * tmp.x + rot.m[i+4] * tmp.y + rot.m[i+8] * tmp.z;
        return dst;
    }
    template<>
    _CPU_AND_GPU_CODE_ inline Vector3f invRotMatTransform<MatrixDataOrder::ColMajor>(const Matrix4f &rot, const Vector3<float>& pos){
        Vector3<float> dst;
        Vector3<float> tmp;
        for(size_t i=0; i < 3; ++i)
            tmp[i] = pos[i] - rot.m [3*4+i];
        for(size_t i=0; i < 3; ++i)
            dst[i] = rot.m[i*4] * tmp.x + rot.m[i*4+1] * tmp.y + rot.m[i*4+2] * tmp.z;
        return dst;
    }
}