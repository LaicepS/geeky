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

set(CMAKE_CXX_FLAGS "-std=c++17 -pedantic -Werror ${CPP_WARNINGS} ${CMAKE_CXX_FLAGS}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GEEKY_SRC "src/geeky.cpp")

find_package(Boost 1.62 COMPONENTS system REQUIRED)

add_executable(geeky ${GEEKY_SRC})

target_link_libraries(geeky LINK_PUBLIC ${Boost_LIBRARIES} pthread)
