#pragma once
#include <cppstd/string.h>
#include <cppstd/vector.h>

namespace fs::path {
    /* TODO: remove old stuff */
    bool path_compare(char *path1, char *path2);
    bool startswith_path(char *subpath, char *path);
    int path_depth(char *path);
    /* warning: this uses malloc, don't forget to free */
    char *rm_prefix(char *prefix, char *path);

    /* new stuff */
    bool starts_with(std::vector<std::string> *path, std::vector<std::string> *start);

    std::vector<std::string> split_path(std::string *path);
}
