CC = g++
CFLAGS = -Wall -m32 -fno-pie -fno-stack-protector

all: kernel

kernel: kernel.o stdlib.o cpubasics.o
	ld -m elf_i386 --oformat binary -Ttext 1000 $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f kernel *.o