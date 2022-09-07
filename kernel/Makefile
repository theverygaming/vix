all:
	@g++ scripts/shbuild.cpp -o scripts/shbuild.o
	@scripts/shbuild.o shbuild
  

clean:
	@rm -f kernel.bin
	@find . \( -name '*.oasm' -o -name '*.ocpp' -o -name '*.o' \) -type f -delete