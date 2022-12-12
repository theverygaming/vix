CC = g++
CFLAGS = -I../kernel/include -I../kernel/arch/x86/include -O3 -nostdlib -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding -fno-exceptions -Wall -Wno-pointer-arith

HEADERS_CPP = $(shell find . -name "*.h") # fuck it, lets just do this..

SOURCES_CPP = $(shell find . -name "*.cpp")

OBJECTS_CPP = $(patsubst %.cpp, %.ocpp, $(SOURCES_CPP))

HEADERS_ASM = $(shell find . -name "*.inc")

SOURCES_ASM = $(shell find . -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, %.oasm, $(SOURCES_ASM))

all: module.o

module.o: $(OBJECTS_ASM) $(OBJECTS_CPP)
	@ld -nostdlib -m elf_i386 -z noexecstack -r  $^ -o $@

%.ocpp: %.cpp $(HEADERS_CPP)
	@echo "-> compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	@echo "-> compiling $<"
	@nasm -f elf -o $@ $< 

clean:
	@find . \( -name '*.o' -o -name '*.ocpp' \) -type f -delete
