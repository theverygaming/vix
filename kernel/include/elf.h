#pragma once
#include <types.h>

class elf_reader {
public:
    elf_reader(bool (*_file_read)(size_t pos)) {
        file_read = _file_read;
    }

    bool is_valid();

private:
    bool (*file_read)(size_t pos) = nullptr;
};
