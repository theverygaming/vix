all:
	$(MAKE) stage1
	$(MAKE) -C stage2
	cat stage1 stage2/stage2 > boot

stage1: stage1.asm
	nasm $< -f bin -o $@

clean:
	rm -f boot stage1 *.oasm *.ocpp *.o
	$(MAKE) -C stage2 clean