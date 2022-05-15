all: boot

boot: boot.asm
	nasm $< -f bin -o $@ 

clean:
	rm -f boot *.o