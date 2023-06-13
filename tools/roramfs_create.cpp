#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>

struct __attribute__((packed)) file_entry {
    char name[100];
    uint32_t size;
    uint32_t offset;
};

struct __attribute__((packed)) header {
    char name[100];
    uint32_t filecount;
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: %s outputfile fsname [inputfiles]\n", argv[0]);
        return 1;
    }

    std::ofstream output(argv[1], std::ios::binary);
    if (!output.is_open()) {
        printf("couldn't open %s\n", argv[1]);
        return 1;
    }

    int filecount = argc - 3;

    struct header hdr;
    memset(hdr.name, 0, sizeof(hdr.name));
    strcpy(hdr.name, argv[2]);
    hdr.filecount = filecount;

    output.write((char *)&hdr, sizeof(struct header));

    uint32_t currentHeaderOffset = sizeof(struct header);

    uint32_t Offset = sizeof(struct header) + (sizeof(struct file_entry) * filecount);

    for (int i = 0; i < filecount; i++) {
        struct file_entry fileentry;
        memset(fileentry.name, 0, sizeof(fileentry.name));
        std::string path_cpp_str = argv[i + 3];
        strcpy(fileentry.name, path_cpp_str.substr(path_cpp_str.find_last_of("/\\") + 1).insert(0, "/").c_str());
        fileentry.offset = Offset;

        std::ifstream input(argv[i + 3], std::ios::binary | std::ios::ate);
        if (!input.is_open()) {
            printf("couldn't open %s\n", argv[i + 3]);
            return 1;
        }

        fileentry.size = input.tellg();
        output.seekp(currentHeaderOffset);
        output.write((char *)&fileentry, sizeof(fileentry));

        input.seekg(0);
        char *buf = new char[fileentry.size];
        input.read(buf, fileentry.size);
        output.seekp(Offset);
        output.write(buf, fileentry.size);
        delete[] buf;
        input.close();
        Offset += fileentry.size;
        currentHeaderOffset += sizeof(struct file_entry);
    }

    output.close();
    return 0;
}
