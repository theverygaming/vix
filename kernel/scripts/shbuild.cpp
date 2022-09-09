#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <vector>

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
        variablemap[name].push_back(value);
    }
}

void appendVariable(std::string name, std::vector<std::string> value) {
    if (variablemap.find(name) != variablemap.end()) {
        variablemap[name].insert(variablemap[name].end(), value.begin(), value.end());
    } else {
        variablemap[name] = value;
    }
}

bool isVarDefined(std::string name) {
    return variablemap.find(name) != variablemap.end();
}

// helper functions
bool stringStartsWith(std::string str, std::string start) {
    if (str.length() < start.length()) {
        return false;
    }
    return (str.find(start) != std::string::npos) && str.find(start) == 0;
}

bool stringEndsWith(std::string str, std::string end) {
    if (str.length() < end.length()) {
        return false;
    }
    return str.find(end) == (str.length() - end.length());
}

std::string stringReplaceEnd(std::string str, std::string endBefore, std::string endAfter) {
    if ((str.length() < endBefore.length()) || (!stringEndsWith(str, endBefore))) {
        throw std::runtime_error("replaceEnd issue");
    }
    str.replace(str.length() - endBefore.length(), str.length() - 1, endAfter);
    return str;
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

bool doesFileNeedRebuild(std::string source, std::string object) {
    struct stat src_buf;
    struct stat obj_buf;
    if (stat(source.c_str(), &src_buf) != 0) {
        fprintf(stderr, "source file %s does not exist!\n", source.c_str());
        throw std::runtime_error("missing source file");
    }
    if (stat(object.c_str(), &obj_buf) != 0) {
        return true;
    }

    if (src_buf.st_mtime > obj_buf.st_mtime) {
        return true;
    }
    return false;
}

inline void runcmd(std::string cmd) {
    if (system(cmd.c_str())) {
        fprintf(stderr, "failed: %s\n", cmd.c_str());
        throw std::runtime_error("command failed");
    }
}

std::string expandStrVector(std::vector<std::string> in) {
    std::string ret;
    for (size_t i = 0; i < in.size(); i++) {
        ret.append(in[i] + " ");
    }
    return ret;
}

void processFile(std::string filename) {
    static std::string lastFolder = "";
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

    // set CURRENTPATH variable
    setVariable("CURRENTPATH", lastFolder);

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

        // remove comments
        if (line.find("//") != std::string::npos) {
            line.erase(line.find("//"), line.length());
        }
        if (line.length() == 0) {
            currentLine++;
            continue;
        }

        // remove trailing spaces
        if (line.find_last_not_of(' ') != std::string::npos) {
            line.erase(line.find_last_not_of(' ')+1);
        }

        // replace VARINSERT(variablename) --------------------------------------------------------------- TODO: make varinsert work with multiple statements on a single line
        {
            std::smatch match;
            if (std::regex_search(line, match, std::regex("VARINSERT\\((.*?)\\)"))) {
                std::string all = "";
                for (size_t i = 0; i < getVariable(match.str(1)).size(); i++) {
                    all = all + getVariable(match.str(1))[i] + " ";
                }
                if (all.length() >= 1) {
                    all.erase(all.length() - 1, 1);
                }
                line = std::regex_replace(line, std::regex("VARINSERT\\((.*?)\\)"), all);
            }
        }

        // is this an include?
        if (stringStartsWith(line, "include ")) {
            line.replace(0, 8, "");
            line.erase(line.find('"'), line.find('"') + 1);
            line.erase(line.find('"'), line.length() - 1);
            processFile(line.c_str());
            currentLine++;
            continue;
        }

        // is this an ifdef?
        if (stringStartsWith(line, "ifvardef ")) {
            int lineSkipCount;
            char buf[200];
            if (line.length() > 200) {
                fprintf(stderr, "insane line length at %s:%zu -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("insane line length");
            }
            if (sscanf(line.c_str(), "ifvardef %s %d", buf, &lineSkipCount) != 2) {
                fprintf(stderr, "syntax error at %s:%zu -> %s\n", filename.c_str(), currentLine + 1, line.c_str());
                throw std::runtime_error("syntax error");
            }
            if (!isVarDefined(buf)) {
                currentLine += lineSkipCount;
            }
            currentLine++;
            continue;
        }

        // is this some variable operation?
        if (stringStartsWith(line, "varappend_tx_prefix ")) {
            line.replace(0, 20, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            std::string prefix = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            std::vector<std::string> toAppend = splitString(line, " ");
            for (size_t i = 0; i < toAppend.size(); i++) {
                toAppend[i].insert(0, prefix);
            }
            appendVariable(name, toAppend);
            currentLine++;
            continue;
        } else if (stringStartsWith(line, "varset_tx ")) {
            line.replace(0, 10, "");
            std::string name = line.substr(0, line.find(' '));
            line.replace(0, line.find(' ') + 1, "");
            setVariable(name, line);
            currentLine++;
            continue;
        }

        if (stringStartsWith(line, "shellcmd ")) {
            line.replace(0, 9, "");
            std::string cmd = line;
            system(cmd.c_str());
            currentLine++;
            continue;
        }

        fprintf(stderr, "%s:%zu -> couldn't parse %s\n", filename.c_str(), currentLine + 1, line.c_str());
        throw std::runtime_error("syntax error");
    }

    lastFolder = savedFolder;
    setVariable("CURRENTPATH", lastFolder);
}

void readConfigFile(std::string filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        fprintf(stderr, "could not open file %s\n", filename.c_str());
        throw std::runtime_error("could not open file");
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
        
        // remove trailing spaces
        if (line.find_last_not_of(' ') != std::string::npos) {
            line.erase(line.find_last_not_of(' ')+1);
        }

        if(line.find(' ') == std::string::npos) {
            currentLine++;
            continue;
        }

        std::string name = line.substr(0, line.find(' '));
        line.replace(0, line.find(' ') + 1, "");
        setVariable(name, line);

        currentLine++;
    }
}

void build_cpp_gcc(std::string cxx, std::string cxxflags, std::string inputfile, std::string outputfile) {
    std::string command = cxx + " " + cxxflags + " -c " + inputfile + " -o " + outputfile;
    printf("%s %s\n", cxx.c_str(), inputfile.c_str());
    runcmd(command);
}

void build_asm_nasm(std::string nasm, std::string nflags, std::string inputfile, std::string outputfile) {
    std::string command = nasm + " " + nflags + " " + inputfile + " -o " + outputfile;
    printf("%s %s\n", nasm.c_str(), inputfile.c_str());
    runcmd(command);
}

void partial_link(std::string ld, std::string ldflags, std::string linkerscript, std::string inputfiles, std::string outputobject) {
    std::string command = ld + " " + ldflags + " -r -T " + linkerscript + " " + inputfiles + " -o " + outputobject;
    printf("%s %s\n", ld.c_str(), inputfiles.c_str());
    runcmd(command);
}

void final_link(std::string ld, std::string ldflags, std::string linkerscript, std::string inputfiles, std::string outputobject) {
    std::string command = ld + " " + ldflags + " -T " + linkerscript + " " + inputfiles + " -o " + outputobject;
    printf("%s %s\n", ld.c_str(), inputfiles.c_str());
    runcmd(command);
}

void final_objcopy(std::string objcopy, std::string objcopyflags, std::string input, std::string output) {
    std::string command = objcopy + " " + objcopyflags + " " + input + " " + output;
    printf("%s %s -> %s\n", objcopy.c_str(), input.c_str(), output.c_str());
    runcmd(command);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s shbuildfile\n", argv[0]);
        return 1;
    }
    readConfigFile("config_gen");
    processFile(argv[1]);
    std::vector<std::string> objs = getVariable("objs");
    for (size_t i = 0; i < objs.size(); i++) {
        if (stringEndsWith(objs[i], ".ocpp")) {
            std::string sourcefile = stringReplaceEnd(objs[i], ".ocpp", ".cpp");
            if (doesFileNeedRebuild(sourcefile, objs[i])) {
                build_cpp_gcc(
                    "g++",
                    "-Iinclude -O3 -nostdlib -m32 -march=i386 -fno-pie -mno-red-zone -fno-stack-protector -ffreestanding -fno-exceptions -Wall -Wextra -Wno-pointer-arith -Wno-unused-parameter",
                    sourcefile,
                    objs[i]);
            } else {
                printf("%s doesn't need to rebuild\n", objs[i].c_str());
            }
            continue;
        }
        if (stringEndsWith(objs[i], ".oasm")) {
            std::string sourcefile = stringReplaceEnd(objs[i], ".oasm", ".asm");
            if (doesFileNeedRebuild(sourcefile, objs[i])) {
                build_asm_nasm("nasm", "-f elf", sourcefile, objs[i]);
            } else {
                printf("%s doesn't need to rebuild\n", objs[i].c_str());
            }
            continue;
        }
        printf("unknown file type -> %s\n", objs[i].c_str());
        return 1;
    }
    final_link("ld", "-nostdlib -m elf_i386 -z noexecstack", "linker.ld", expandStrVector(objs), "kernel.o");
    final_objcopy("objcopy", "-O binary", "kernel.o", "kernel.bin");

    return 0;
}
