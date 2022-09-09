#include "configask.h"
#include "configparser.h"
#include <cstdio>
#include <cstring>
#include <iostream>

static bool text_askYN(bool pr = true) {
    if(pr) { 
        printf("[y/n]?");
    }
    char c = getc(stdin);
    if(c == 'y' || c == 'Y') {
        while(c != '\n') {
            c = getc(stdin);
        }
        return true;
    } else if(c == 'n' || c == 'N') {
        while(c != '\n') {
            c = getc(stdin);
        }
        return false;
    }
    return text_askYN(false);
}

void configText() {
    for(std::map<std::string, configoption_t>::iterator i = config_map.begin(); i != config_map.end(); i++) {
        if((i->second.conftype == config_type::CONFIG_BOOL) && (!i->second.preset)) {
            printf("bool %s => %s\n", i->first.c_str(), i->second.info.c_str());
            if(text_askYN()) {
                i->second.content = "1";
                continue;
            }
            i->second.content = "0";
        }
        if((i->second.conftype == config_type::CONFIG_STRING) && (!i->second.preset)) {
            printf("string %s => %s\n", i->first.c_str(), i->second.info.c_str());
            char c = getc(stdin);
            while(c != '\n') {
                i->second.content += c;
                c = getc(stdin);
            }
            continue;
        }
    }
}