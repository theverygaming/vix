all: config_gen
	@g++ scripts/shbuild.cpp -o scripts/shbuild -Wall
	@scripts/shbuild shbuild
  
config: 
	@$(MAKE) --no-print-directory -C scripts/shconfig
	@mkdir -p include/generated/
	@scripts/shconfig/shconfig -f shconfig -D CONFIG_ARCH $(MAKE_ARCH)

config_gen:
	@$(MAKE) --no-print-directory -C scripts/shconfig
	@mkdir -p include/generated/
	@scripts/shconfig/shconfig -g config_default_$(MAKE_ARCH)

clean:
	@rm -f kernel.bin
	@find . \( -name '*.oasm' -o -name '*.ocpp' -o -name '*.o' \) -type f -delete
	@$(MAKE) --no-print-directory -C scripts/shconfig clean

proper: clean
	@rm -f config_gen
	@rm -f scripts/shbuild
