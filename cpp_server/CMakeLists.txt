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

set(GEEKY_SRC "src/geeky.cpp")

set(BOOST_ROOT "/home/dorian/opt/boost/boost_1_73_0")
find_package(Boost 1.73 REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++17 -g -pedantic -Werror -isystem ${Boost_INCLUDE_DIRS} ${CPP_WARNINGS} ${CMAKE_CXX_FLAGS}")

add_executable(geeky ${GEEKY_SRC})

target_link_libraries(geeky LINK_PUBLIC ${Boost_LIBRARIES} pthread)
