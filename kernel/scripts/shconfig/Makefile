all: shconfig

shconfig: shconfig.o configparser.o
	@g++ $^ -o $@

%.o: %.cpp
	@echo "-> compiling kernel/scripts/$^"
	@g++ $^ -c -o $@ -Wall
