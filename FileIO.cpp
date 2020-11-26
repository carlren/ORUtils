#include "FileIO.h"

namespace SCFUSION{
    namespace IO {
        template <> void FileIO::assignValue(bool *value, const std::vector<std::string> &map) {
            for (size_t i = 0; i < map.size(); ++i)
                value[i] = std::atoi(map[i].c_str());
        }


        template <> void FileIO::assignValue(size_t *value, const std::vector<std::string> &map) {
            for (size_t i = 0; i < map.size(); ++i)
                value[i] = std::atoi(map[i].c_str());
        }

        template <> void FileIO::assignValue(int *value, const std::vector<std::string> &map) {
            for (size_t i = 0; i < map.size(); ++i)
                value[i] = std::atoi(map[i].c_str());
        }

        template <> void FileIO::assignValue(float *value, const std::vector<std::string> &map) {
            for (size_t i = 0; i < map.size(); ++i)
                value[i] = std::atof(map[i].c_str());
        }

        template <> void FileIO::assignValue(std::string *value, const std::vector<std::string> &map) {
            value->clear();
            for (size_t i = 0; i < map.size(); ++i) {
//                    value[i] = map[i].c_str();
                *value += map[i];
                if(i+1 != map.size()) *value += ",";
            }
        }
    }
}
