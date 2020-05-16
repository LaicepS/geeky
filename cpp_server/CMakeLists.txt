cmake_minimum_required(VERSION 3.0)

project(geeky)

set(CMAKE_CXX_COMPILER clang++)

set(CPP_WARNINGS " \
    -Wall \
    -Wextra \
    -Wshadow \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Wcast-align \
    -Wunused \
    -Woverloaded-virtual \
    -Wpedantic -Wconversion \
    -Wsign-conversion \
    -Wdouble-promotion \
    -Wformat=2 \
")


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERNAL_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/third_party/src/include")

set(CMAKE_CXX_FLAGS "-std=c++17 -g -pedantic -Werror -isystem ${EXTERNAL_INCLUDE} -I${EXTERNAL_INCLUDE} ${CPP_WARNINGS} ${CMAKE_CXX_FLAGS}")

add_executable(geeky "src/geeky.cpp" "src/http_helpers.cpp")
target_link_libraries(geeky LINK_PUBLIC pthread)
