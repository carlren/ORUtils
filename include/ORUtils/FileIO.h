#pragma once

#include <cstdlib>
#include <map>
#include <vector>
#include <fstream>
#include <utility>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "Logging.h"

#define packageName(var, s) SCFUSION::IO::PACKAGE_VAR(#var, var, s)
#define packageName2(var,name,s) SCSLAM::IO::PACKAGE_VAR(name, var, s)

namespace SCFUSION{
    namespace IO {
        template<typename T>
        struct PACKAGE {
            std::string name;
            T *var;
            size_t size;
            PACKAGE(std::string name, T *var, size_t size) : name(std::move(name)), var(var), size(size){}
        };

        template<typename T>
        PACKAGE<T> PACKAGE_VAR(std::string name, T *var, size_t size) {
            if (name.find_first_of("&", 0, 1) != std::string::npos) {
                name.substr(1);
                name.erase(name.begin());
            }
            return PACKAGE<T>(name, var, size);
        }

        class FileIO {
            public:
            FileIO()= default;
            protected:
            static std::vector<std::string> splitLine(std::string s, char delimiter) {
                std::vector<std::string> tokens;
                std::string token;
                std::istringstream tokenStream(s);
                while (std::getline(tokenStream, token, delimiter)) {
                    if (!token.empty())
                        tokens.push_back(token);
                }
                return tokens;
            }
            static std::map<std::string, std::vector<std::string>>  readFileToMap(const std::string& path) {
                std::fstream file(path, std::ios::in);
                if (!file.is_open()) {
                    char buffer[512];
                    sprintf(buffer,"[%s][%s] Cannot open file [%s] for loading parameters.\n", __FILE__,__FUNCTION__, path.c_str());
                    throw std::runtime_error(buffer);
                }
                std::string line;
                std::map<std::string, std::vector<std::string>> inputData;
                while (std::getline(file, line, '\n')) {
                    if (line[0] == '#') continue;
                    if (line.empty()) continue;
                    /// Remove all spaces
                    line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
                    if (line.empty()) continue;

                    std::vector<std::string> tokens = splitLine(line, ':');
                    if (tokens.size() != 2) {
                        tokens = splitLine(line, '=');
                        if (tokens.size() != 2)
                            throw std::runtime_error("Wrong Format! ");
                    }
                    std::string name = tokens[0];
                    std::vector<std::string> values = splitLine(tokens[1], ',');
                    inputData[name] = std::move(values);

//            SCLOG(INFO) << name << ": " << tokens[1] << "\n";
                }
                return inputData;
            }
            template<typename T>
            void saveParam(const IO::PACKAGE<T> &pkg, std::fstream &file) {
                file << pkg.name << ": ";
                for (size_t i = 0; i < pkg.size; ++i)
                    file << pkg.var[i] << ", ";
                file << "\n";
            }
            template<typename T>
            int loadParam(const IO::PACKAGE<T> &pkg, std::map<std::string, std::vector<std::string>> &map) {
                /// Find target
                if (map.find(pkg.name) == map.end()) {
                    SCLOG(WARNING) << "[" << pkg.name <<"] Cannot be found in the given file. Skip!\n";
                } else {
                    if (map[pkg.name].size() == 0) SCLOG(WARNING) << "[" << pkg.name << "] has size 0";
                    if (map[pkg.name].size() != pkg.size)
                        SCLOG(WARNING) << "[" << pkg.name << "] has un expacted size. (" << map[pkg.name].size() << ","
                                       << pkg.size << ")";
                    assignValue(pkg.var, map[pkg.name]);
                }
                return 1;
            }
            template <class T> static void assignValue(T *value, const std::vector<std::string> &map);
//            template<>  void assignValue(bool *value, const std::vector<std::string> &map){}
        };
        template <> void FileIO::assignValue(bool *value, const std::vector<std::string> &map);


        template <> void FileIO::assignValue(size_t *value, const std::vector<std::string> &map);

        template <> void FileIO::assignValue(int *value, const std::vector<std::string> &map);

        template <> void FileIO::assignValue(float *value, const std::vector<std::string> &map);

        template <> void FileIO::assignValue(std::string *value, const std::vector<std::string> &map);
    }
}
