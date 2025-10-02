import os

# https://stackoverflow.com/a/7392391
textchars = bytearray({7,8,9,10,12,13,27} | set(range(0x20, 0x100)) - {0x7f})
is_binary_string = lambda bytes: bool(bytes.translate(None, textchars))

all_files = []

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

def loc_dir(path, exclude_dirs):
    files = list_dir(path, exclude_dirs)
    lines = 0
    for file in files:
        if not is_binary_string(open(file, 'rb').read(1024)):
            flines = sum([1 for line in open(file) if len(line) > 0 and line != "\n"])
            all_files.append((file, flines))
            lines += flines
    if lines == 0:
        return None
    return path, lines

def gen_chart(title, paths, exclude_dirs):
    counts = []
    for path in paths:
        loc = loc_dir(path, exclude_dirs)
        if loc != None:
            counts.append(loc)
    counts.sort(key=lambda x:x[1], reverse=True)
    str = ""
    str += f"```mermaid\npie\n    title {title}\n"
    total = 0
    for count in counts:
        str += f"    \"{count[0]}\" : {count[1]}\n"
        total += count[1]
    str += f"```\ntotal: {total}\n"
    str += "| Directory | LOC |\n|---|---|\n"
    for count in counts:
        str += f"| {count[0]} | {count[1]} |\n"
    str += "\n"

    return str

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
    "*.oc",
    "*.oS",
    "*.oasm",

    "*Makefile",
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

print(gen_chart("Kernel LOC", kernel_paths, excludes))

all_files.sort(key=lambda x:x[1], reverse=True)
print("\nTop 50 files with most LOC:\n\n| Filename | LOC |\n| -------- | --- |")
for file in all_files[:50]:
    print(f"|{file[0]}|{file[1]}|")
