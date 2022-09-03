#pragma once

namespace fs::path {
    bool path_compare(char *path1, char *path2);
    bool startswith_path(char *subpath, char *path);
    int path_depth(char *path);
    /* warning: this uses malloc, don't forget to free */
    char *rm_prefix(char *prefix, char *path);
}