CC = aarch64-linux-gnu-g++
LD = aarch64-linux-gnu-ld
CFLAGS = -I. -O3 -nostdlib -nostartfiles -nostdinc -fno-pie -fno-stack-protector -ffreestanding -fno-exceptions -Wall -Wno-pointer-arith

HEADERS_CPP = $(shell find . -name "*.h")

SOURCES_CPP = $(shell find . -name "*.cpp")

OBJECTS_CPP = $(patsubst %.cpp, %.ocpp, $(SOURCES_CPP))

SOURCES_ASM = $(shell find . -name "*.s")

OBJECTS_ASM = $(patsubst %.s, %.os, $(SOURCES_ASM))

all:
	@$(MAKE) startup
	@#dd status=none if=/dev/zero of=startup bs=1 count=1 seek=65535

startup: $(OBJECTS_ASM) $(OBJECTS_CPP)
	@$(LD) -nostdlib -T linker.ld $^ -o startup.elf
	@aarch64-linux-gnu-objcopy -O binary startup.elf $@

%.ocpp: %.cpp $(HEADERS_CPP)
	@echo "-> compiling startup/$<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.os: %.s
	@echo "-> compiling startup/$<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f startup startup.elf
	@find . \( -name '*.os' -o -name '*.ocpp' \) -type f -delete

proper: clean
