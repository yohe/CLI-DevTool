cmake_minimum_required(VERSION 2.6)

PROJECT(cli-devtool-test)

IF(UNIX)
    include_directories(/usr/include ./)
    link_directories(/usr/lib)
ENDIF(UNIX)
IF(APPLE)
    include_directories(/opt/local/include /usr/include ./)
    link_directories(/opt/local/lib /usr/lib)
ENDIF(APPLE)

ADD_EXECUTABLE(test_main main.cpp ../key/key_seq.cc ../key/key_map.cc ../parser/input_parser.cc)

set_target_properties(test_main PROPERTIES COMPILE_FLAGS "-Wall -I ../ -I ../submod/iunit/cpp")
set_target_properties(test_main PROPERTIES LINK_FLAGS -lncurses)

add_custom_target(run
                  COMMAND ./test_main
)

set(CMAKE_CXX_FLAGS_DEBUG "-g")

