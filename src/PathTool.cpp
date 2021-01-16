//
//  Tools.cpp
//  pointnet_cpp
//
//  Created by Shun-Cheng Wu on 18/10/2017.
//
//

#include <ORUtils/PathTool.hpp>
#include <utility>
#include <vector>
#include <cassert>
#include <fstream>
#include <string>

using namespace tools;

[[maybe_unused]] std::string PathTool::get_executable_path()
{
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);

#if defined(_WIN32)
    ::GetModuleFileName(NULL, &buffer[0], bufferSize);

#elif defined(__linux__)
    // Get the process ID.
    int pid = getpid();

    // Construct a path to the symbolic link pointing to the process executable.
    // This is at /proc/<pid>/exe on Linux systems (we hope).
    std::ostringstream oss;
    oss << "/proc/" << pid << "/exe";
    std::string link = oss.str();

    // Read the contents of the link.
    int count = readlink(link.c_str(), &buffer[0], bufferSize);
    if(count == -1) throw std::runtime_error("Could not read symbolic link");
    buffer[count] = '\0';

#elif defined(__APPLE__)
    if(_NSGetExecutablePath(&buffer[0], &bufferSize))
    {
        buffer.resize(bufferSize);
        _NSGetExecutablePath(&buffer[0], &bufferSize);
    }

#else
#error Cannot yet find the executable on this platform
#endif

    std::string s = &buffer[0];
    return s;
}

[[maybe_unused]] std::string PathTool::find_parent_folder(std::string input, int times){
    if(input.empty()) return "";
    std::string output;
    if(input[input.size()-1] == '/') //input.pop_back();
        input = input.substr(0, input.length()-1);
    output = input;
    for (int i = 0; i < times; ++i){
        size_t temp = output.find_last_of('/');
        if (temp == std::string::npos){
//                printf("[warning] Reach the most previous folder.");
//                return "";
            if (i==0) output = "";
            break;
        }
        size_t first = output.find_first_of('/');
        if(first == temp) return output.substr(0,first+1); // no file name
        output.assign(output, 0, temp);
    }
    return output;
}

[[maybe_unused]] int PathTool::remove_directory(const char *path)
{
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d)
    {
        struct dirent *p;

        r = 0;

        while (!r && (p=readdir(d)))
        {
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
            {
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = (char*)malloc(len);

            if (buf)
            {
                struct stat statbuf{};

                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!stat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        r2 = remove_directory(buf);
                    }
                    else
                    {
                        r2 = unlink(buf);
                    }
                }

                free(buf);
            }

            r = r2;
        }

        closedir(d);
    }

    if (!r)
    {
        r = rmdir(path);
    }

    return r;
}

[[maybe_unused]] bool PathTool::checkfileexist(const std::string& filename)
{
    struct stat buf{};
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}

[[maybe_unused]] bool PathTool::checkfolderexist(const std::string& output_db_name){
#ifdef WIN32
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
#else
    //        const char* folderr = output_db_name;
    //        folderr = output_db_name;
    struct stat sb{};
    return stat(output_db_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
#endif
}

[[maybe_unused]] std::string PathTool::find_parent_folder_name(const std::string& path){
    std::string output, tmp;
    tmp = find_parent_folder(path, 1);
    output = path.substr(tmp.size()+1, path.size() - tmp.size());
    return output;
}

[[maybe_unused]] std::string PathTool::get_current_dir_name(std::string path){
    std::string output, tmp;
    if(path[path.size()-1] == '/') //path.pop_back();
        path = path.substr(0, path.length()-1);

    tmp = find_parent_folder(path, 1);
    if(tmp.empty()) return ""; // no parent
    if(tmp=="/") return path.substr(1,path.length());
    if(tmp.size() == path.size()) return path; // is name already.

    output = path.substr(tmp.size(), path.length());
    return output;
}

[[maybe_unused]] void PathTool::get_files_include_name(std::string path, const std::string &name,
                                      std::vector<std::string> &files_with_name,
                                      bool return_full, bool sort) {
    auto files_all = get_files_in_folder(std::move(path), "", return_full, sort);
    for (const auto& file: files_all) {
        auto boo = file.find(name);
        if (boo != std::string::npos) files_with_name.push_back(file);
    }
}

[[maybe_unused]] void PathTool::get_files_include_name_recursively (const std::string& path, const std::string& name, std::vector<std::string>& files_with_name) {
    std::vector<std::string> folders;
    folders.push_back(path);
    while(isFolder(folders.back())) {
        auto files = get_files_in_folder(folders.back(),"", true, false);
        folders.pop_back();

        for(const auto &p : files) {
            if(getFileType(p).empty()) { //folder
                folders.push_back(p);
                continue;
            }
            if(p.find(name) != std::string::npos) { // bin
                files_with_name.push_back(p);
                continue;
            }
        }
        if(folders.empty()) break;
    }
}

[[maybe_unused]] void PathTool::get_folders_include_name_recursively (const std::string& path, const std::string& name, std::vector<std::string>& folders_with_name){
    std::vector<std::string> folders;
    folders.push_back(path);
    while(isFolder(folders.back())) {
        auto files = get_files_in_folder(folders.back(),"", true, false);

        if(get_current_dir_name(folders.back()).find(name) != std::string::npos) { // bin
            folders_with_name.push_back(folders.back());
        }
        folders.pop_back();

        for(const auto &p : files) {
            if(isFolder(p)) { //folder
                folders.push_back(p);
                continue;
            }
        }
        if(folders.empty()) break;
    }
}
[[maybe_unused]] void PathTool::get_targetFile_in_targetFolder_recursively (const std::string& path, const std::string& folderName, const std::string& fileName, std::vector<std::string>& target_files_in_target_folders){
    std::vector<std::string> files;
    get_files_include_name_recursively(path, fileName, files);
    for(const auto& f : files) {
        auto tmp = find_parent_folder(f);
        if(getFileName(tmp) == folderName) target_files_in_target_folders.push_back(f);
    }
}

[[maybe_unused]] std::vector<std::string> PathTool::get_files_include_name (std::string path, const std::string& name){
    std::vector<std::string> files_with_name;
    auto files_all = get_files_in_folder(std::move(path), "");
    for (const auto& file: files_all) {
        auto boo = file.find(name);
        if (boo != std::string::npos) files_with_name.push_back(file);
    }
    return files_with_name;
}



[[maybe_unused]] void PathTool::check_and_delete_folder (const std::string& path){
    std::string tmp = path;
    bool isFolder = PathTool::isFolder(tmp);
    if(!isFolder) tmp = PathTool::find_parent_folder(tmp);
    if(checkfolderexist(tmp)) remove_directory(tmp.c_str());
}

[[maybe_unused]] void PathTool::check_and_create_folder (const std::string& path){
    if(path.empty())
        return;
    std::string tmp = path;
    bool isFolder = PathTool::isFolder(tmp);
    if(!isFolder) tmp = PathTool::find_parent_folder(tmp);
    if(!checkfolderexist(tmp))create_folder(tmp);
}

[[maybe_unused]] void PathTool::create_folder(std::string name){
    bool isFolder = PathTool::isFolder(name);
    if(!isFolder) {
        printf("[Warning][PathTool::create_folder] Input path has file type! Remove file type first! You can use remove_file_type() function.\n");
        return;
    }
    name = PathTool::CheckEnd(name);

//#ifdef WIN32
//        _mkdir(name.c_str());
//#else
//        mkdir(name.c_str(), 0755);
//#endif
    auto first = name.find_first_of('/');
    auto second = name.find_last_of('/');
    if(first != second) {
        std::string motherFolder = name.substr(0, first+1);
        std::string rest = name.substr(first+1, name.length());
        while(true){
            auto f = rest.find_first_of('/');
            std::string folder;
            if(f == std::string::npos){
                folder = motherFolder + rest;
            } else {
                folder = motherFolder + rest.substr(0, f+1);
            }
            rest = rest.substr(f+1, rest.length());
#if defined _MSC_VER
            _mkdir(folder.c_str());
#else
            mkdir(motherFolder.c_str(), 0777);
#endif
            motherFolder = folder;
            if(f == std::string::npos){
                break;
            }
        }
    } else {
        mkdir(name.substr(0, name.find_first_of('/')).c_str(), 0777);
    }
}

[[maybe_unused]] char* PathTool::string2char (const std::string& string){
    char *cstr = new char[string.length() + 1];
    strcpy(cstr, string.c_str());
    // do stuff
    return cstr;
    //delete [] cstr;//TOOD: memory leak
}

[[maybe_unused]] inline bool PathTool::isdigit(const std::string &string) {
    return std::count_if(string.begin(), string.end(), [](unsigned char c){ return std::isdigit(c); }) == (int) string.length();
}

struct digit_sort
{
    inline bool operator() (const std::string& struct1, const std::string& struct2)
    {
        auto getFileName = [](const std::string &path) ->std::string {
            // get filename
            auto lastDot = path.find_last_of('.');
            auto lastRightSlash = path.find_last_of('/');
            if(lastRightSlash == std::string::npos && lastDot == std::string::npos) // input is a name (100)
                return path;
            if(lastRightSlash == std::string::npos) // input is a name with type (100.cpp)
                return path.substr(0, lastDot);
            if(lastDot == std::string::npos) // input is a path without type (something/100)
                return path.substr(lastRightSlash+1, path.length());
            if((int) lastDot == path.front() && (int) lastRightSlash == path.front()+1) // input is ./100.cpp
                return path.substr(lastRightSlash+1, path.length());

            return path.substr(lastRightSlash+1, lastRightSlash-lastDot-1);
        };
        auto name1 = std::stoi(getFileName(struct1));
        auto name2 = std::stoi(getFileName(struct2));
        return (name1 < name2);
    }
};

[[maybe_unused]] std::vector<std::string> PathTool::get_files_in_folder (std::string path, const std::string& type, bool return_full, bool sort){
    std::vector<std::string> file_vec;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) != nullptr) {
        path = CheckEnd(path);
        while ((ent = readdir (dir)) != nullptr) {
            if (ent->d_name[0] != '.') {
                /* print all the files and directories within directory */
                //printf ("%s\n", ent->d_name);
                file_vec.push_back(return_full? path+ent->d_name:ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        std::stringstream ss;
        ss << "Cannot open file at " << path << "\n";
        throw std::runtime_error(ss.str());
    }
    if (sort) {
        bool allDigit = true;
        for(const auto& p:file_vec){
            // get filename
            auto name = getFileName(p);

            if(!isdigit(name)) {
                allDigit = false;
                break;
            }
        }

        if(allDigit)
            std::sort(file_vec.begin(),file_vec.end(), digit_sort());
        else
            std::sort(file_vec.begin(),file_vec.end());
    }

    if(type.empty()) return file_vec;

    std::vector<std::string> filtered;
    for (const auto& name: file_vec) {
        if(name.size() > type.size()) {
            std::string tmp = name.substr(name.size()-type.size(), type.size());
            if (tmp == type) filtered.push_back(name);
        }
    }
    return filtered;
}

[[maybe_unused]] void PathTool::erase_chracter(std::string& input, const std::string& charact){
    size_t a;// = input.find(charact);
    do {
        a = input.find(charact);
        if (a != std::string::npos){
            input.erase(a, charact.size());
        }
    } while (a != std::string::npos);


}

[[maybe_unused]] void erase_charecter(std::string &input, char ch){
    input.erase(std::remove(input.begin(),input.end(), ch), input.end()); /// remove space
}

[[maybe_unused]] void PathTool::replace_chracter(std::string& input, const std::string& charact, const std::string& with_this){
    size_t a;// = input.find(charact);
    do {
        a = input.find(charact);
        if (a != std::string::npos){
            input.replace(a, charact.size(), with_this);
        }
    } while (a != std::string::npos);
}

[[maybe_unused]] std::string PathTool::remove_file_type (std::string path, const std::string& type) {
    if (!type.empty()) {
        //std::string::size_type has_type = path.find_last_of(type);
        //assert(has_type != std::string::npos);
        return path.substr(0, path.size()-type.size());
    } else {
        std::string::size_type has_type = path.find_last_of('.');
        if(has_type != std::string::npos && has_type != 0)
            return path.substr(0, has_type);
        return path;
    }
}

[[maybe_unused]] std::string PathTool::getFileType(std::string pathIn){
    if(pathIn.back() == '/') return ""; // is folder
    std::string::size_type last_dot = pathIn.find_last_of('.');
    std::string::size_type last_slash = pathIn.find_last_of('/');
    if(last_slash != std::string::npos && last_dot != std::string::npos) { // middle path contains '.' in name
        if(last_slash > last_dot) return  "";
    } else if(last_dot == std::string::npos || last_dot == 0) { // cannot find '.' or . is the last
        return "";
    }
    return pathIn.substr(last_dot, pathIn.size());
}

[[maybe_unused]] int PathTool::GetTotalLines(const std::string& file_path){
    std::string line;
    std::fstream myfile;
    myfile.open(file_path.c_str());
    int counter=-1;
    assert(myfile.is_open() && "Unable to open file");
    do {
        counter++;
        getline(myfile,line,'\n');
    } while (!line.empty());
    return counter;
}

[[maybe_unused]] std::vector<std::string> PathTool::splitbychar(std::string line){
    std::vector<std::string> data;
    char* token;
    erase_chracter(line, "\r");
    token = strtok(string2char(line), "\t ,");
    while (token!=nullptr) {
        data.emplace_back(token);
        token = strtok(nullptr, "\t ,");
    }
    return data;
}

[[maybe_unused]] std::string PathTool::getFileName(std::string pthIn){
    return remove_file_type(get_current_dir_name(std::move(pthIn)));
}

[[maybe_unused]] std::string PathTool::CheckEnd(std::string path){
    if(strcmp(&path[path.length()-1], "/")!=0){
        return path + "/";
    } else {
        return path;
    }
}


std::vector<std::string> PathTool::splitLine(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        if(!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

std::string PathTool::addWaterSerialNumber(const std::string& path){
    std::string path_new = path;
    bool exist = false;
    std::string type = getFileType(path);
    bool isFolder = type.empty();

    size_t counter=0;
    if(isFolder) {
        exist = checkfolderexist(path_new);
        while (exist) {
            path_new = CheckEnd(path); // check and add "/"
            path_new = path_new.substr(0, path_new.length() - 1); // remove "/"
            path_new += std::to_string(counter++);
            exist = checkfolderexist(path_new);
        }
    } else {
        exist = checkfileexist(path);
        while(exist) {
            path_new = remove_file_type(path);
            path_new += std::to_string(counter++) + type;
            exist = checkfileexist(path_new);
        }
    }
    return  path_new;
}

bool PathTool::isFolder(const std::string &path){
    std::string type = getFileType(path);
    return type.empty();
}

bool PathTool::isNumber(const std::string &s){
    // if contains '.'
    if(s.empty())return false;

    // is negative
    if(s[0] == '-'){
        if(s.find('-') != s.find_last_of('-')) return false; // contain two '-'
    }

    if(s.find('.') != std::string::npos) {
        if(s.find('.') != s.find_last_of('.')) return false; // contain two '.'
//            return std::find_if(s.begin(),s.end(), [](char c) {
//                return !(std::isdigit(c) || c == '.'); }) == s.end();
    }
//        else
//            return std::find_if(s.begin(),s.end(), [](char c) {
//                return !std::isdigit(c); }) == s.end();

    return std::find_if(s.begin(),s.end(), [](char c) {
        return !(std::isdigit(c) || c == '.' || c == '-'); }) == s.end();
}