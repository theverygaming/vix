all:
	@$(MAKE) stage1
	@$(MAKE) --no-print-directory -C stage2
	@cat stage1 stage2/stage2 > boot

stage1: stage1.asm
	@echo "-> compiling boot/$<"
	@nasm $< -f bin -o $@

clean:
	@rm -f boot stage1 *.oasm *.ocpp *.o
	@$(MAKE) --no-print-directory -C stage2 clean