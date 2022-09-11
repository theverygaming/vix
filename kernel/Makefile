all: config_gen
	@g++ scripts/shbuild.cpp -o scripts/shbuild.o -Wall
	@scripts/shbuild.o shbuild
  
config: 
	@$(MAKE) --no-print-directory -C scripts/shconfig
	@mkdir -p include/generated/
	@scripts/shconfig/shconfig -f shconfig -D CONFIG_ARCH $(MAKE_ARCH)

config_gen:
	@$(MAKE) --no-print-directory -C scripts/shconfig
	@mkdir -p include/generated/
	@scripts/shconfig/shconfig -g config_default

clean:
	@rm -f kernel.bin
	@find . \( -name '*.oasm' -o -name '*.ocpp' -o -name '*.o' \) -type f -delete
