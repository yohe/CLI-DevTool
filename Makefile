CC=clang++

all: console

console: sample.cc console.h command.h command_selector.h
	$(CC) sample.cc -lncurses -o console

clean:
	rm console
