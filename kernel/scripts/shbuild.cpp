#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

void processFile(std::string filename);

std::map<std::string, std::vector<std::string>> variablemap;

std::vector<std::string> &getVariable(std::string name) {
    if (variablemap.find(name) != variablemap.end()) {
        return variablemap[name];
    } else {
        fprintf(stderr, "variable %s does not exist\n", name.c_str());
        throw std::runtime_error("could not find variable");
    }
}

void setVariable(std::string name, std::vector<std::string> value) {
    variablemap[name] = value;
}

void setVariable(std::string name, std::string value) {
    variablemap[name].clear();
    variablemap[name].push_back(value);
}

void appendVariable(std::string name, std::string value) {
    if (variablemap.find(name) != variablemap.end()) {
        variablemap[name].push_back(value);
    } else {
        // fprintf(stderr, "append: creating new variable %s\n", name.c_str());
        variablemap[name].push_back(value);
    }
}

void appendVariable(std::string name, std::vector<std::string> value) {
    if (variablemap.find(name) != variablemap.end()) {
        variablemap[name].insert(variablemap[name].end(), value.begin(), value.end());
    } else {
        // fprintf(stderr, "append: creating new variable %s\n", name.c_str());
        variablemap[name] = value;
    }
}

bool isVarDefined(std::string name) {
    return variablemap.find(name) != variablemap.end();
}

// helper functions
bool stringStartsWith(std::string str, std::string start) {
    return (str.find(start) != std::string::npos) && str.find(start) == 0;
}

std::vector<std::string> splitString(std::string str, std::string split) {
    std::vector<std::string> ret;
    size_t pos = 0;
    while ((pos = str.find(split)) != std::string::npos) {
        ret.push_back(str.substr(0, pos));
        str.erase(0, pos + split.length());
    }
    ret.push_back(str.substr(0, pos));
    return ret;
}

// parser functions
bool parseInclude(std::string line) {
    if (stringStartsWith(line, "include")) {
        line.replace(0, 8, "");
        line.erase(line.find('"'), line.find('"') + 1);
        line.erase(line.find('"'), line.length() - 1);
        processFile(line.c_str());
        return true;
    }
    return false;
}

bool parseVar(std::string line) {
    if (stringStartsWith(line, "varappend_tx")) {
        line.replace(0, 13, "");
        std::string name = line.substr(0, line.find(' '));
        line.replace(0, line.find(' ') + 1, "");
        appendVariable(name, splitString(line, " "));
        return true;
    } else if (stringStartsWith(line, "varprefix_tx")) {
        line.replace(0, 13, "");
        std::string name = line.substr(0, line.find(' '));
        line.replace(0, line.find(' ') + 1, "");
        std::vector<std::string> &var = getVariable(name);
        for (int i = 0; i < var.size(); i++) {
            var[i].insert(0, line.c_str());
        }
        return true;
    } else if (stringStartsWith(line, "varappend")) {
        char buf_dst[400];
        char buf_src[400];
        if (line.length() > 400) {
            throw std::runtime_error("insane line length");
        }
        if (sscanf(line.c_str(), "varappend %s %s", buf_dst, buf_src) != 2) {
            throw std::runtime_error("syntax error");
        }
        appendVariable(buf_dst, getVariable(buf_src));
        return true;
    }
    return false;
}

std::string lastFolder = "";
void processFile(std::string filename) {
    std::string savedFolder = lastFolder;
    filename = lastFolder + filename;
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        fprintf(stderr, "could not open file %s\n", filename.c_str());
        throw std::runtime_error("could not find variable");
    }

    // get folder
    if (filename.find_last_of("/") != std::string::npos) {
        lastFolder = lastFolder + filename.substr(0, filename.find_last_of("/")) + "/";
    }

    // set CURRENTPATH variable
    setVariable("CURRENTPATH", lastFolder);

    std::vector<std::string> lines;
    {
        std::string currentLine;
        while (std::getline(infile, currentLine)) {
            lines.push_back(currentLine);
        }
    }

    int currentLine = 0;
    while (currentLine < lines.size()) {
        std::string line = lines[currentLine];

        // fprintf(stderr, "%s:%d -> %s\n", filename.c_str(), currentLine+1, line.c_str());
        //  remove comments
        if (line.find("//") != std::string::npos) {
            line.erase(line.find("//"), line.length());
        }
        if (line.length() == 0) {
            currentLine++;
            continue;
        }
        // fprintf(stderr, "%s:%d -> %s\n", filename.c_str(), currentLine+1, line.c_str());

        // is this an include?
        if (parseInclude(line)) {
            currentLine++;
            continue;
        }

        // replace VARINSERT(variablename) --------------------------------------------------------------- TODO: make varinsert work with multiple statements on a single line
        {
            std::smatch match;
            if (std::regex_search(line, match, std::regex("VARINSERT\\((.*?)\\)"))) {
                std::string all = "";
                for (int i = 0; i < getVariable(match.str(1)).size(); i++) {
                    all = all + getVariable(match.str(1))[i] + " ";
                }
                if (all.length() >= 1) {
                    all.erase(all.length() - 1, 1);
                }
                line = std::regex_replace(line, std::regex("VARINSERT\\((.*?)\\)"), all);
            }
        }
        // fprintf(stderr, "%s:%d -> %s\n", filename.c_str(), currentLine+1, line.c_str());

        // is this an ifdef?
        if (stringStartsWith(line, "ifvardef")) {
            int lineSkipCount;
            char buf[200];
            if (line.length() > 200) {
                fprintf(stderr, "insane line length at %s:%d -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("insane line length");
            }
            if (sscanf(line.c_str(), "ifvardef %s %d", buf, &lineSkipCount) != 2) {
                fprintf(stderr, "syntax error at %s:%d -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("syntax error");
            }
            if (!isVarDefined(buf)) {
                currentLine += lineSkipCount;
            }
            currentLine++;
            continue;
        }

        // is this an ifvarisnot?
        if (stringStartsWith(line, "ifvarisnot")) {
            int lineSkipCount;
            char buf[200];
            char value[200];
            if (line.length() > 200) {
                fprintf(stderr, "insane line length at %s:%d -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("insane line length");
            }
            if (sscanf(line.c_str(), "ifvarisnot %s %s %d", buf, value, &lineSkipCount) != 3) {
                fprintf(stderr, "syntax error at %s:%d -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("syntax error");
            }
            if (getVariable(buf).size() > 0) {
                if (!strcmp(getVariable(buf)[0].c_str(), value)) {
                    currentLine += lineSkipCount;
                }
            }
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "arraylength")) {
            line.replace(0, 12, "");
            std::string srcarray = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            std::string dstvar = line;
            printf("src: %s dst: %s\n", srcarray.c_str(), dstvar.c_str());
            setVariable(dstvar, std::to_string(getVariable(srcarray).size()));
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "getarrindex")) {
            line.replace(0, 12, "");
            std::string array = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            std::string index = getVariable(line.substr(0, line.find(' ')))[0];
            line.replace(0, line.find(' ') + 1, "");
            std::string dstvar = line;
            if (getVariable(array).size() > std::stoi(index) - 1) {
                setVariable(dstvar, getVariable(array)[std::stoi(index) - 1]);
            } else {
                fprintf(stderr, "array out of range at %s:%d -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("array out of range");
            }
            currentLine++;
            continue;
        }

        // is this some variable operation?
        if (parseVar(line)) {
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "subtract")) {
            line.replace(0, 9, "");
            std::string dstvar = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            std::string amount = line;
            setVariable(dstvar, std::to_string(std::stoi(getVariable(dstvar)[0]) - std::stoi(amount)));
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "shellcmd")) {
            line.replace(0, 9, "");
            std::string cmd = line;
            system(cmd.c_str());
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "print")) {
            line.replace(0, 6, "");
            printf("%s\n", line.c_str());
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "jump")) {
            sscanf(line.c_str(), "jump %d", &currentLine);
            currentLine--; // jump line number starts at 1
            continue;
        }

        fprintf(stderr, "%s:%d -> couldn't parse\n", filename.c_str(), currentLine + 1);
        throw std::runtime_error("syntax error");
    }

    lastFolder = savedFolder;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s shbuildfile\n", argv[0]);
        return 1;
    }

    fprintf(stderr, "shbuild: processing file %s\n", argv[1]);
    processFile(argv[1]);
    std::vector<std::string> test = getVariable("objs");
    for (int i = 0; i < test.size(); i++) {
        // printf("%s\n", test[i].c_str());
    }

    return 0;
}