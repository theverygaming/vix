CC = g++
CFLAGS = -I. -O3 -nostdlib -m32 -march=i386 -fno-pie -fno-stack-protector -ffreestanding -fno-exceptions -Wall -Wno-pointer-arith

HEADERS_CPP = $(shell find . -name "*.h") # fuck it, lets just do this..

SOURCES_CPP = $(shell find . -name "*.cpp")

OBJECTS_CPP = $(patsubst %.cpp, %.ocpp, $(SOURCES_CPP))

HEADERS_ASM = $(shell find . -name "*.inc")

SOURCES_ASM = $(shell find . -name "*.asm")

OBJECTS_ASM = $(patsubst %.asm, %.oasm, $(SOURCES_ASM))

all:
	@$(MAKE) startup
	@dd status=none if=/dev/zero of=startup bs=1 count=1 seek=65535

startup: $(OBJECTS_ASM) $(OBJECTS_CPP)
	@ld -m elf_i386 --oformat binary -T linker.ld -nostdlib $^ -o $@
	@

%.ocpp: %.cpp $(HEADERS_CPP)
	@echo "-> compiling prekernel/$<"
	@$(CC) $(CFLAGS) -c $< -o $@

%.oasm: %.asm
	@echo "-> compiling prekernel/$<"
	@nasm -f elf -o $@ $< 

clean:
	@rm -f startup
	@find . \( -name '*.oasm' -o -name '*.ocpp' \) -type f -delete

distclean: clean
