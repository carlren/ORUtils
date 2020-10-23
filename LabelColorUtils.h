#pragma once
#include <random>
#include <algorithm>
#include <fstream>
#include "MemoryBlock.h"
#include "Vector.h"

namespace ORUtils {
    class LabelColorGenerator {
    public:
        static void generateList(ORUtils::MemoryBlock<ORUtils::Vector4<float>> *labelColorList, size_t size, bool useCPU, bool useGPU,
                                 bool normalize = true) {
            if(labelColorList->dataSize != size) labelColorList->Resize(size);
            ORUtils::Vector4<float> *labelColorData = labelColorList->GetData(MEMORYDEVICE_CPU);

            std::random_device rand;
            std::default_random_engine e1(rand());
            std::uniform_int_distribution<int> dist(0, 255);
            labelColorData[0] = ORUtils::Vector4<float>(255, 255, 255, 255);
            for (size_t i = 1; i < size; i++) {
                labelColorData[i].x = dist(e1);
                labelColorData[i].y = dist(e1);
                labelColorData[i].z = dist(e1);
                labelColorData[i].w = 255;
                while (labelColorData[i].x + labelColorData[i].y + labelColorData[i].z < 255) {
                    labelColorData[i].x = dist(e1);
                    labelColorData[i].y = dist(e1);
                    labelColorData[i].z = dist(e1);
                }
//                printf("LabelColor[%zu]: %f %f %f\n", i, labelColorData[i].x, labelColorData[i].y, labelColorData[i].z);
            }
//            labelColorData[size-1] = ORUtils::Vector4<float>(255, 255, 255, 255);
            if (normalize)
                for (size_t i = 0; i < size; ++i)
                    labelColorData[i] /= 255;
        }

        static bool
        loadList(ORUtils::MemoryBlock<ORUtils::Vector4<float>> *labelColorList, const std::string &path, bool normalize = true) {
            std::ifstream file(path);
            if (!file.is_open()) return false;
            size_t size = std::count(std::istreambuf_iterator<char>(file),
                                     std::istreambuf_iterator<char>(), '\n');
            file.clear();
            file.seekg(0, std::ios::beg);

            labelColorList->Resize(size);
            ORUtils::Vector4<float> *labelColorData = labelColorList->GetData(MEMORYDEVICE_CPU);

            std::string label, r, g, b, a;
            while (file >> label >> r >> g >> b >> a) {
                size_t label_ = std::stoi(label);
                if(label_ >= size) throw std::runtime_error("Found label larger than the total amount of given colors!\n");

                labelColorData[label_].x = std::atof(r.c_str());
                labelColorData[label_].y = std::atof(g.c_str());
                labelColorData[label_].z = std::atof(b.c_str());
                labelColorData[label_].w = std::atof(a.c_str());

                bool needNormalization = true;
                for(size_t i=0;i<4;++i)if(labelColorData[label_][i] > 0 && labelColorData[label_][i] < 1) needNormalization = false;
                if(needNormalization) labelColorData[label_] /= 255.f;
            }
            return true;
        }

        static bool saveList(ORUtils::MemoryBlock<ORUtils::Vector4<float>> *labelColorList, const std::string &path) {
            std::fstream file(path, std::ios::out);
            if (!file.is_open()) {
                printf("Failed to open file to save label colors.\n");
                return false;
            }

            ORUtils::Vector4<float> *labelColorData = labelColorList->GetData(MEMORYDEVICE_CPU);
            std::string label, r, g, b, a;
            for (size_t i = 0; i < labelColorList->dataSize; i++) {
                r = std::to_string(labelColorData[i].x);
                g = std::to_string(labelColorData[i].y);
                b = std::to_string(labelColorData[i].z);
                a = std::to_string(labelColorData[i].w);
                file << i << " " << r << " " << g << " " << b << " " << a << std::endl;
            }
            file.close();
            return true;
        }

        /**
         Generate Label color.
         Try to load first if cannot load or the loaded size does not match, generate one
         */
        static void Run(ORUtils::MemoryBlock<ORUtils::Vector4<float>> *labelColorList, size_t size, bool useCPU, bool useGPU,
                        const std::string& pathToLabelColorList = "./LabelColorList.txt", bool normalize = true) {
            bool loaded = loadList(labelColorList, pathToLabelColorList, normalize);
#ifndef NDEBUG
            if(labelColorList->dataSize != size) printf("Label color list loaded but do not have enough size.\n");
#endif
            if (!loaded) {
                printf("Cannot Load label color list from path: %s. Generate one.\n", pathToLabelColorList.c_str());
                generateList(labelColorList, size, useCPU, useGPU, normalize);

                saveList(labelColorList, pathToLabelColorList);
            }
//            else if (size > 0)
//                if (labelColorList->dataSize != size) {
//                    generateList(labelColorList, size, useCPU, useGPU, normalize);
//                }

            if (useGPU) labelColorList->UpdateDeviceFromHost();
        }
    };
}
