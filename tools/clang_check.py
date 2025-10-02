import os

# https://stackoverflow.com/a/7392391
textchars = bytearray({7,8,9,10,12,13,27} | set(range(0x20, 0x100)) - {0x7f})
is_binary_string = lambda bytes: bool(bytes.translate(None, textchars))

def list_dir(path, exclude_dirs):
    files_l = []
    for subdir, dirs, files in os.walk(path):
        for file in files:
            fname = os.path.join(subdir, file)
            for exclude in exclude_dirs:
                if exclude[0] == '*':
                    if fname.endswith(exclude[1:]):
                        break
                else:
                    if fname.startswith(exclude):
                        break
            else:
                files_l.append(fname)
    return files_l

def clang_check_dir(paths, exclude_dirs):
    for path in paths:
        for file in list_dir(path, exclude_dirs):
            if is_binary_string(open(file, 'rb').read(1024)):
                continue
            os.system(f"clang-format --style=file --dry-run -Werror {file}")

# path must be passed with trailing /
def append_subdirs(path, lst):
    for dir in next(os.walk(path))[1]:
        lst.append(path + dir)
    return lst

excludes = [
    "kernel/scripts/",
    "kernel/include/config",
    "kernel/include/generated/",
    "kernel/arch/example/",

    "*.o",
    "*.oasm",

    "*.asm",
    "*.S",
    "*.s",
    "*.inc",

    "*Makefile",
    "*Makefile.arch",
    "*Kconfig",
    "*.ld",
    "*.py",
    "*.sh",
    "*.config"
    "*.config_old"
]

kernel_paths = [
    "kernel/abi",
    "kernel/stdlibs",
    "kernel/include",
    "kernel/drivers",
    "kernel/fs",
    "kernel/kernel",
    "kernel/mm",
    "kernel/net",
    "kernel/tests",
]
kernel_paths = append_subdirs("kernel/arch/", kernel_paths)

clang_check_dir(kernel_paths, excludes)
