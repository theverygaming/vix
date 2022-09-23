#include <fs/path.h>
#include <memory_alloc/memalloc.h>
#include <stdlib.h>

static void format_path(char *path) {
    // remove / at end if there is one
    size_t pathlen = stdlib::strlen(path);
    if (pathlen > 1 && (path[pathlen - 1] == '/')) {
        path[pathlen - 1] = '\0';
    }
}

bool fs::path::path_compare(char *path1, char *path2) {
    char *p1 = (char *)__builtin_alloca((stdlib::strlen(path1) + 1));
    char *p2 = (char *)__builtin_alloca((stdlib::strlen(path2) + 1));
    stdlib::strcpy(p1, path1);
    stdlib::strcpy(p2, path2);
    format_path(p1);
    format_path(p2);

    if (stdlib::strcmp(p1, p2) != 0) {
        return false;
    }
    return true;
}

bool fs::path::startswith_path(char *subpath, char *path) {
    char *subpath_a = (char *)__builtin_alloca((stdlib::strlen(subpath) + 1));
    char *path_a = (char *)__builtin_alloca((stdlib::strlen(path) + 1));
    stdlib::strcpy(subpath_a, subpath);
    stdlib::strcpy(path_a, path);
    format_path(subpath_a);
    format_path(path_a);

    char *strstr_result = stdlib::strstr(path_a, subpath_a);

    if ((strstr_result != nullptr) && (stdlib::strlen(subpath_a) > 0) && (strstr_result == path_a)) {
        return true;
    }
    return false;
}

int fs::path::path_depth(char *path) {
    char *path_a = (char *)__builtin_alloca((stdlib::strlen(path) + 1));
    stdlib::strcpy(path_a, path);
    format_path(path_a);

    int count = 0;
    int extra_c = 0;
    while (*path_a) {
        extra_c++;
        if (*path_a++ == '/') {
            extra_c = 0;
            count++;
        }
    }
    if (extra_c > 0) {
        count++;
    }
    return count;
}

char *fs::path::rm_prefix(char *prefix, char *path) {
    char *prefix_a = (char *)__builtin_alloca((stdlib::strlen(prefix) + 1));
    char *path_a = (char *)__builtin_alloca((stdlib::strlen(path) + 1));
    stdlib::strcpy(prefix_a, prefix);
    stdlib::strcpy(path_a, path);
    format_path(prefix_a);
    format_path(path_a);

    char *ret = nullptr;

    if ((stdlib::strstr(path_a, prefix_a) != nullptr) && (stdlib::strlen(prefix_a) > 1)) {
        char *path_a_n = path_a + stdlib::strlen(prefix_a);
        ret = (char *)memalloc::single::kmalloc(stdlib::strlen(path_a_n) + 1);
        stdlib::strcpy(ret, path_a_n);
        if (*ret != '/') {
            memalloc::single::kfree(ret);
            ret = (char *)memalloc::single::kmalloc(stdlib::strlen(path_a) + 1);
            stdlib::strcpy(ret, path_a);
        }
    } else {
        ret = (char *)memalloc::single::kmalloc(stdlib::strlen(path_a) + 1);
        stdlib::strcpy(ret, path_a);
    }

    return ret;
}
