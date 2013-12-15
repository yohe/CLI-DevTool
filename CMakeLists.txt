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

ADD_EXECUTABLE(console
              sample.cc
              console.cc
              key/key_seq.cc
              key/key_map.cc
              command/command.cc
              command/param_comple/file_behavior.cc
              command/builtin/help.cc
              command/builtin/history.cc
              command/builtin/script.cc
              command/builtin/editor.cc
              )
ADD_EXECUTABLE(key_trace
              sample.cc
              console.cc
              key/key_seq.cc
              key/key_map.cc
              command/command.cc
              command/param_comple/file_behavior.cc
              command/builtin/help.cc
              command/builtin/history.cc
              command/builtin/script.cc
              command/builtin/editor.cc
              )
#ADD_EXECUTABLE(test/test_main test/main.cpp key_seq.cc key_map.cc)

set_directory_properties(PROPERTIES
                         ADDITIONAL_MAKE_CLEAN_FILES "test/test_main")

set_target_properties(console PROPERTIES LINK_FLAGS -lncurses)
set_target_properties(console PROPERTIES COMPILE_FLAGS "-Wall")
set_target_properties(key_trace PROPERTIES COMPILE_FLAGS "-Wall -DKEY_TRACE")
set_target_properties(key_trace PROPERTIES LINK_FLAGS -lncurses)
#set_target_properties(test/test_main PROPERTIES COMPILE_FLAGS "-Wall -I ../ -I ../iunit/cpp")
#set_target_properties(test/test_main PROPERTIES LINK_FLAGS -lncurses)


add_custom_target(test
                  COMMAND cmake .
                  COMMAND make
                  COMMAND make run
                  WORKING_DIRECTORY test
)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
