#include <stdlib.h>
#include <vix/fs/path.h>
#include <vix/mm/kheap.h>
#include <vix/panic.h>
#include <vix/types.h>

bool fs::path::starts_with(std::vector<std::string> *path, std::vector<std::string> *start) {
    if (path->size() < start->size()) {
        return false;
    }
    for (size_t i = 0; i < start->size(); i++) {
        if (i >= path->size()) {
            return false;
        }
        if ((*start)[i] != (*path)[i]) {
            return false;
        }
    }
    return true;
}

bool fs::path::equals(std::vector<std::string> *path1, std::vector<std::string> *path2) {
    if (path1->size() != path2->size()) {
        return false;
    }
    for (size_t i = 0; i < path1->size(); i++) {
        if ((*path1)[i] != (*path2)[i]) {
            return false;
        }
    }
    return true;
}

// TODO: this 3am code needs cleanup
std::vector<std::string> fs::path::split_path(std::string *path) {
    std::vector<std::string> vec;

    if (path->size() == 0 || (*path)[0] != '/') {
        DEBUG_PRINTF("weird path: [%s] -- processing anyway\n", path->c_str());
    }
    for (size_t i = 0; i < path->size(); i++) {
        while (i < path->size() && (*path)[i] == '/') {
            i++;
        }
        if (i < path->size()) {
            vec.push_back("");
        }
        while (i < path->size() && (*path)[i] != '/') {
            vec[vec.size() - 1] += (*path)[i];
            i++;
        }
    }
    return vec;
}

std::string fs::path::unsplit_path(std::vector<std::string> *path) {
    std::string unsplit;
    for (size_t i = 0; i < path->size(); i++) {
        unsplit += "/";
        unsplit += (*path)[i];
    }
    return unsplit;
}
