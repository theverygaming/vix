#include "configparser.h"
#include <fstream>
#include <functional>
#include <regex>
#include <stdexcept>

std::map<std::string, configoption_t> config_map;

configoption_t &getConfig(std::string name) {
    if (config_map.find(name) != config_map.end()) {
        return config_map[name];
    } else {
        fprintf(stderr, "config %s does not exist\n", name.c_str());
        throw std::runtime_error("could not find config");
    }
}

void setConfig(std::string name, configoption_t config) {
    config_map[name] = config;
}

// helper functions
static bool stringStartsWith(std::string str, std::string start) {
    if (str.length() < start.length()) {
        return false;
    }
    return (str.find(start) != std::string::npos) && str.find(start) == 0;
}

static bool stringEndsWith(std::string str, std::string end) {
    if (str.length() < end.length()) {
        return false;
    }
    return str.find(end) == (str.length() - end.length());
}

static std::string stringReplaceEnd(std::string str, std::string endBefore, std::string endAfter) {
    if ((str.length() < endBefore.length()) || (!stringEndsWith(str, endBefore))) {
        throw std::runtime_error("replaceEnd issue");
    }
    str.replace(str.length() - endBefore.length(), str.length() - 1, endAfter);
    return str;
}

static std::vector<std::string> splitString(std::string str, std::string split) {
    std::vector<std::string> ret;
    size_t pos = 0;
    while ((pos = str.find(split)) != std::string::npos) {
        ret.push_back(str.substr(0, pos));
        str.erase(0, pos + split.length());
    }
    ret.push_back(str.substr(0, pos));
    return ret;
}

void processFile(std::string filename) {
    static std::string lastFolder = "";
    static std::vector<std::string> currentConfig_submenupath;
    std::string savedFolder = lastFolder;
    filename = lastFolder + filename;
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        fprintf(stderr, "could not open file %s\n", filename.c_str());
        throw std::runtime_error("could not open file");
    }

    // get folder
    if (filename.find_last_of("/") != std::string::npos) {
        lastFolder = filename.substr(0, filename.find_last_of("/")) + "/";
    }

    std::vector<std::string> lines;
    {
        std::string currentLine;
        while (std::getline(infile, currentLine)) {
            lines.push_back(currentLine);
        }
    }

    size_t currentLine = 0;
    while (currentLine < lines.size()) {
        std::string line = lines[currentLine];

        //  remove comments
        if (line.find("//") != std::string::npos) {
            line.erase(line.find("//"), line.length());
        }
        if (line.length() == 0) {
            currentLine++;
            continue;
        }

        // remove trailing spaces
        if (line.find_last_not_of(' ') != std::string::npos) {
            line.erase(line.find_last_not_of(' ') + 1);
        }

        // replace CONFINSERT(configname) --------------------------------------------------------------- TODO: make confinsert work with multiple statements on a single line
        {
            std::smatch match;
            if (std::regex_search(line, match, std::regex("CONFINSERT\\((.*?)\\)"))) {
                line = std::regex_replace(line, std::regex("CONFINSERT\\((.*?)\\)"), getConfig(match.str(1)).content);
            }
        }

        if (stringStartsWith(line, "include ")) {
            line.replace(0, 8, "");
            line.erase(line.find('"'), line.find('"') + 1);
            line.erase(line.find('"'), line.length() - 1);
            processFile(line.c_str());
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "setsubmenupath ")) {
            line.replace(0, 15, "");
            currentConfig_submenupath = splitString(line, "/");
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "addconfigbool ")) {
            line.replace(0, 14, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            configoption_t newcfg;
            newcfg.preset = false;
            newcfg.submenu_path = currentConfig_submenupath;
            newcfg.conftype = config_type::CONFIG_BOOL;
            newcfg.info = line;
            setConfig(name, newcfg);
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "addconfigstring ")) {
            line.replace(0, 16, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            configoption_t newcfg;
            newcfg.preset = false;
            newcfg.submenu_path = currentConfig_submenupath;
            newcfg.conftype = config_type::CONFIG_STRING;
            newcfg.info = line;
            setConfig(name, newcfg);
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "presetconfigstring ")) {
            line.replace(0, 19, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            configoption_t newcfg;
            newcfg.preset = true;
            newcfg.submenu_path = currentConfig_submenupath;
            newcfg.conftype = config_type::CONFIG_STRING;
            newcfg.info = "";
            newcfg.content = line;
            setConfig(name, newcfg);
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "presetconfigbool ")) {
            line.replace(0, 17, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            configoption_t newcfg;
            newcfg.preset = true;
            newcfg.submenu_path = currentConfig_submenupath;
            newcfg.conftype = config_type::CONFIG_BOOL;
            newcfg.info = "";
            if(!((line.length() == 1) && ((line.find('0') != std::string::npos) || line.find('1') != std::string::npos))) {
                fprintf(stderr, "%s:%zu -> bool issue\n", filename.c_str(), currentLine + 1);
                throw std::runtime_error("syntax error");
            }
            newcfg.content = line;
            setConfig(name, newcfg);
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "setconfigoptions ")) {
            line.replace(0, 17, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            config_map[name].conftype = config_type::CONFIG_OPTIONS;
            config_map[name].choose_options = splitString(line, " ");
            currentLine++;
            continue;
        }

        fprintf(stderr, "%s:%zu -> couldn't parse %s\n", filename.c_str(), currentLine + 1, line.c_str());
        throw std::runtime_error("syntax error");
    }

    lastFolder = savedFolder;
}
