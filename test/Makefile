
CINCLUDES = -I ../submod/iunit/cpp \
			-I ../

default:
	make test_main

test_main: main.cpp
	g++ $(CINCLUDES) main.cpp -o test_main

run:
	./test_main
	
clean:
	rm -f test_main
