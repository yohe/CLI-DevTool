CC=clang++
#CC=g++

CFLAGS=

CC += $(CFLAGS)

all: console key_trace

debug: 
	make all "CFLAGS+=-DDEBUG"

console: sample.cc key_map.cc key_stroke.cc console.h command/command.h command/command_selector.h 
	$(CC) -Wall -pedantic -g sample.cc key_map.cc key_stroke.cc -lncurses -o console -I ./

key_trace: sample.cc key_stroke.cc key_map.cc console.h command/command.h command/command_selector.h
	$(CC) -DKEY_TRACE sample.cc key_map.cc key_stroke.cc -lncurses -o key_trace -I ./

clean:
	rm -f console
	rm -f key_trace
