all:
	@g++ scripts/shbuild.cpp -o scripts/shbuild.o -Wall
	@scripts/shbuild.o shbuild
  
config: 
	@$(MAKE) -C scripts/shconfig
	@mkdir -p include/generated/
	@scripts/shconfig/shconfig shconfig

clean:
	@rm -f kernel.bin
	@find . \( -name '*.oasm' -o -name '*.ocpp' -o -name '*.o' \) -type f -delete
