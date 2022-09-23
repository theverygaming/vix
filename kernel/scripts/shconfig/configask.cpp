#include "configask.h"
#include "configparser.h"
#include <cstdio>
#include <iostream>
#include <vector>

static bool text_askYN() {
    printf("[y/n]?");
    std::string str;
    std::getline(std::cin, str);
    if ((str.find('y') != std::string::npos) || (str.find('Y') != std::string::npos)) {
        return true;
    } else if ((str.find('n') != std::string::npos) || (str.find('N') != std::string::npos)) {
        return false;
    }
    return text_askYN();
}

static int text_chooseOptions(std::vector<std::string> options) {
    for (size_t i = 0; i < options.size(); i++) {
        printf("%lu - %s\n", i + 1, options[i].c_str());
    }
    size_t chosen = 0;
    std::string str;
    while ((chosen > options.size()) || (chosen < 1)) {
        std::getline(std::cin, str);
        try {
            chosen = std::stoi(str);
        } catch (...) {}
    }
    return chosen - 1;
}

void configText() {
    for (std::map<std::string, configoption_t>::iterator i = config_map.begin(); i != config_map.end(); i++) {
        if ((i->second.conftype == config_type::CONFIG_BOOL) && (!i->second.preset)) {
            printf("bool %s => %s\n", i->first.c_str(), i->second.info.c_str());
            if (text_askYN()) {
                i->second.content = "1";
                continue;
            }
            i->second.content = "0";
        }
        if ((i->second.conftype == config_type::CONFIG_STRING) && (!i->second.preset)) {
            printf("string %s => %s\n", i->first.c_str(), i->second.info.c_str());
            std::getline(std::cin, i->second.content);
            continue;
        }
        if ((i->second.conftype == config_type::CONFIG_OPTIONS) && (i->second.choose_options.size() > 0)) {
            printf("options %s => %s\n", i->first.c_str(), i->second.info.c_str());
            i->second.content = i->second.choose_options[text_chooseOptions(i->second.choose_options)];
            continue;
        }
    }
}
