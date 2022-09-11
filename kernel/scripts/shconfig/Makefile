all: shconfig

shconfig: shconfig.o configparser.o configgen.o configask.o
	@g++ $^ -o $@

%.o: %.cpp
	@echo "-> compiling kernel/scripts/$^"
	@g++ $^ -c -o $@ -Wall

clean:
	@rm -f shconfig
