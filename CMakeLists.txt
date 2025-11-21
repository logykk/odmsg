cmake_minimum_required(VERSION 3.16)
project(odmsg C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(odmsg main.c
        odz_util.c
        lz_hashchain.c
        compress.c
        decompress.c)

if (MSVC)
    target_compile_options(odmsg PRIVATE /O2 /W4)
else()
    target_compile_options(odmsg PRIVATE -O2 -Wall -Wextra -pedantic -march=native -flto)
    target_link_options(odmsg PRIVATE -flto)
endif()

add_custom_target(run
        COMMAND echo "Hello World" > original
        COMMAND echo "ODZ!@!+!!!!!!!!%(&^&-&-&}#!^*&}!!*#&-&%" > odmsgd
        COMMAND $<TARGET_FILE:odmsg> c Hello World > encoded
        COMMAND $<TARGET_FILE:odmsg> d "ODZ!@!+!!!!!!!!%(&^&-&-&}#!^*&}!!*#&-&%" > decoded
        COMMAND ${CMAKE_COMMAND} -E compare_files original decoded
        COMMAND ${CMAKE_COMMAND} -E compare_files odmsgd encoded
        DEPENDS odmsg
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Encode/Decode compare (Hello world)"
)

add_custom_target(tidy
        COMMAND ${CMAKE_COMMAND} -E rm -f original odmsgd encoded decoded
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Remove generated output files"
)

install(TARGETS odmsg RUNTIME DESTINATION bin)
