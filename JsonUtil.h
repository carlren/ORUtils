//
// Created by sc on 9/1/20.
//

#ifndef GRAPHSLAM_JSONUTIL_H
#define GRAPHSLAM_JSONUTIL_H
#ifdef COMPILE_WITH_JSON
#include <json11.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Logging.h"
#include <cassert>

static size_t indent_increment=4;

class JsonUtils {
public:
    static json11::Json LoadJson(const std::string &path){
        std::ifstream is(path, std::ios::in);
        std::string dataset((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        std::string err;
        auto json = json11::Json::parse(dataset, err);
        if (!err.empty())
            SCLOG(ERROR) << "Error reading " << path << " " << err;
        return json;
    }

    static std::string GetType(const json11::Json &json){
        switch (json.type()) {
            case json11::Json::NUL:
                return "NUL";
            case json11::Json::NUMBER:
                return "NUMBER";
            case json11::Json::BOOL:
                return "BOOL";
            case json11::Json::STRING:
                return "STRING";
            case json11::Json::ARRAY:
                return "ARRAY";
            case json11::Json::OBJECT:
                return "OBJECT";
            default:
                return "UNKNOWN";
        }
    }
    static void GetLayoutInfo(const json11::Json &json, size_t depth){
        printf("[%zu]Current type %s\n",depth, GetType(json).c_str());
        switch (json.type()) {
            case json11::Json::NUL:
                break;
            case json11::Json::NUMBER:
                printf("[%zu] number value: %f\n",depth, json.number_value());
                break;
            case json11::Json::BOOL:
                printf("[%zu] bool: %d\n",depth, json.bool_value());
                break;
            case json11::Json::STRING:
                printf("[%zu] string: %s\n",depth, json.string_value().c_str());
                break;
            case json11::Json::ARRAY:
                printf("[%zu] array size: %zu\n",depth, json.array_items().size());
                if(depth>0)
                    for(auto &arr:json.array_items()) GetLayoutInfo(arr,depth-1);
                break;
            case json11::Json::OBJECT:
                printf("[%zu] object size: %zu\n",depth, json.object_items().size());
                if(depth>0)
                    for(auto &arr:json.object_items()){
                        printf("[%zu] obj[%s]: \n",depth,arr.first.c_str());
                        GetLayoutInfo(arr.second,depth-1);
                    }
                break;
        }
    }
    static void PrintValueType(const json11::Json &json) {
        switch (json.type()) {
            case json11::Json::NUMBER:
                std::cout << json.number_value();
                return;
            case json11::Json::BOOL:
                std::cout << json.bool_value();
                return;
            case json11::Json::STRING:
                std::cout << json.string_value();
                return;
            default:
                std::cout << "unprintable type: " << GetType(json);
        }
    }

    static void write_name(const std::string &name, std::fstream &file){
        file << "\"" << name << "\"";
    }
    static void _dump(const json11::Json &json, size_t indent, std::fstream &file){
        if(json.is_object()){
            auto iter = json.object_items().begin();
            auto end  = json.object_items().end();
            file << "{\n";
            while(iter != end) {
                if(iter != json.object_items().begin())  file << ",\n";
                file << std::setw(indent);
                write_name(iter->first,file);
                file << ": ";
                _dump(iter->second,indent+indent_increment,file);
                std::advance(iter,1);
            }
            file << "\n";
            file << std::setw(indent);
            file << "}";
        } else if (json.is_array()) {
            auto iter = json.array_items().begin();
            auto end = json.array_items().end();
            file << "[";
            while(iter != end) {
                file << "\n";
                file << std::setw(indent);
                _dump(*iter,indent+indent_increment,file);
                std::advance(iter,1);
                if(iter != end)  file << ",";
                else {
                    file << "\n";
                    file << std::setw(indent);
                }
            }
            file << "]";
        } else if (json.is_bool()) {
            if(json.bool_value())
                file << "true";
            else
                file << "false";
        } else if (json.is_string() ) {
            write_name(json.string_value(), file);
        } else if (json.is_number()){
            file << json.number_value();
        }
    }

    static void Dump(const json11::Json &json, const std::string &path){
        std::fstream file(path, std::ios::out);
        assert(file.is_open());
        _dump(json,4,file);
    }
};

#endif
#endif //GRAPHSLAM_JSONUTIL_H
