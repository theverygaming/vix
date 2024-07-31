#pragma once
#include <string>
#include <vector>

namespace fs::path {
    bool starts_with(std::vector<std::string> *path, std::vector<std::string> *start);
    bool equals(std::vector<std::string> *path1, std::vector<std::string> *path2);

    std::vector<std::string> split_path(std::string *path);
    std::string unsplit_path(std::vector<std::string> *path);
}
