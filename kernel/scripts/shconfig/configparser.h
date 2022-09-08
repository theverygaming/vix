#pragma once
#include <string>
#include <map>
#include <vector>

enum class config_type {CONFIG_BOOL, CONFIG_STRING};

typedef struct {
    std::vector<std::string> submenu_path;
    config_type conftype;
    std::string content;
    std::string info;
    bool preset;
} configoption_t;

extern std::map<std::string, configoption_t> config_list;

void processFile(std::string filename);
