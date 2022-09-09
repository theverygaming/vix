#include "configgen.h"
#include "configparser.h"
#include <fstream>
#include <stdexcept>

void genConfigHeader(std::string filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        fprintf(stderr, "could not open file %s\n", filename.c_str());
        throw std::runtime_error("could not open file");
    }

    outfile << "#pragma once\n";

    for(std::map<std::string, configoption_t>::iterator i = config_map.begin(); i != config_map.end(); i++) {
        if(i->second.content.length() == 0) {
            continue;
        }
        outfile << "#define " << i->first << " " << i->second.content << "\n";
    }

    outfile.close();
}

void genConfigFile(std::string filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        fprintf(stderr, "could not open file %s\n", filename.c_str());
        throw std::runtime_error("could not open file");
    }

    for(std::map<std::string, configoption_t>::iterator i = config_map.begin(); i != config_map.end(); i++) {
        if(i->second.content.length() == 0) {
            continue;
        }
        outfile << i->first << " " << i->second.content << "\n";
    }

    outfile.close();
}