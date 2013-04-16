cmake_minimum_required(VERSION 2.6)

PROJECT(cli-devtool)

IF(UNIX)
    include_directories(/usr/include ./)
    link_directories(/usr/lib)
ENDIF(UNIX)
IF(APPLE)
    include_directories(/opt/local/include /usr/include ./)
    link_directories(/opt/local/lib /usr/lib)
ENDIF(APPLE)

ADD_EXECUTABLE(console sample.cc key_seq.cc key_map.cc)
ADD_EXECUTABLE(key_trace sample.cc key_seq.cc key_map.cc)

set_target_properties(console PROPERTIES LINK_FLAGS -lncurses)
set_target_properties(console PROPERTIES COMPILE_FLAGS "-Wall")
set_target_properties(key_trace PROPERTIES COMPILE_FLAGS "-Wall -DKEY_TRACE")
set_target_properties(key_trace PROPERTIES LINK_FLAGS -lncurses)

set(CMAKE_CXX_FLAGS_DEBUG "-g")

