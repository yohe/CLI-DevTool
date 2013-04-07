
CFLAGS = -g 
#-DDEBUG
CINCLUDES = -I ../submod/iunit/cpp \
			-I ../
TARGET_SRC = ../key_seq.cc \
			 ../key_map.cc \

TEST_SRC = main.cpp \
		   key_seq.cpp \
		   key_map.cpp \

default:
	make test_main

test_main: $(TARGET_SRC) $(TEST_SRC)
	g++ $(CFLAGS) $(CINCLUDES) main.cpp $(TARGET_SRC) -o test_main

run: test_main
	@./test_main

clean:
	rm -f test_main

