CC = g++
CFLAGS = -Iinclude -O3 -nostdlib -m32 -march=i386 -fno-pie -mno-red-zone -fno-stack-protector -ffreestanding -fno-exceptions -Wall -Wextra -Wno-pointer-arith -Wno-unused-parameter

HEADERS_CPP = $(shell find . -name "*.h") # fuck it, lets just do this..

SOURCES_CPP = $(shell find . -name "*.cpp")

OBJECTS_CPP = $(patsubst %.cpp, %.ocpp, $(SOURCES_CPP))

HEADERS_ASM = $(shell find . -name "*.inc")

SOURCES_ASM = $(shell find . -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, %.oasm, $(SOURCES_ASM))

all: kernel.bin

kernel.bin: $(OBJECTS_ASM) $(OBJECTS_CPP)
	@ld -m elf_i386 --oformat binary -T linker.ld -nostdlib $^ -o $@

%.ocpp: %.cpp $(HEADERS_CPP)
	@echo "-> compiling kernel/$<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	@echo "-> compiling kernel/$<"
	@nasm -f elf -o $@ $< 

clean:
	@rm -f kernel.bin
	@find . \( -name '*.oasm' -o -name '*.ocpp' \) -type f -delete