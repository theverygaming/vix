CC = g++
CFLAGS = -Wall -m32 -fno-pie -fno-stack-protector 

HEADERS_CPP = $(wildcard *.h) \
			$(wildcard */*.h) \
			$(wildcard */*/*.h) \
			$(wildcard */*/*/*.h)

SOURCES_CPP = $(wildcard *.cpp) \
			$(wildcard */*.cpp) \
			$(wildcard */*/*.cpp) \
			$(wildcard */*/*/*.cpp)

OBJECTS_CPP = $(patsubst %.cpp, %.ocpp, $(SOURCES_CPP))

HEADERS_ASM = $(wildcard *.inc) \
			  $(wildcard */*.inc) \
			  $(wildcard */*/*.inc) \
			  $(wildcard */*/*/*.inc)

SOURCES_ASM = $(wildcard *.asm) \
			  $(wildcard */*.asm) \
			  $(wildcard */*/*.asm) \
			  $(wildcard */*/*/*.asm)

OBJECTS_ASM = $(patsubst %.asm, %.oasm, $(SOURCES_ASM))

all: kernel

kernel: kernel.ocpp $(OBJECTS_ASM) $(OBJECTS_CPP)
	ld -m elf_i386 --oformat binary -Ttext 1000 $^ -o $@

%.ocpp: %.cpp $(HEADERS_CPP)
	$(CC) $(CFLAGS) -c $< -o $@ 

%.oasm: %.asm
	nasm -f elf -o $@ $^ 

clean:
	rm -f kernel *.oasm *.ocpp *.o