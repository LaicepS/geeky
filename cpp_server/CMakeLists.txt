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
    -Wunused-variable \
    -Woverloaded-virtual \
    -Wpedantic -Wconversion \
    -Wsign-conversion \
    -Wdouble-promotion \
    -Wformat=2 \
")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERNAL_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/third_party/src/include")

set(CMAKE_CXX_FLAGS "-std=c++17 \
  -isystem ${EXTERNAL_INCLUDE} -I${EXTERNAL_INCLUDE} \
  -g \
  -pedantic -Werror ${CPP_WARNINGS} \
   ${CMAKE_CXX_FLAGS}")

add_executable(geeky "src/geeky.cpp" "src/http_helpers.cpp" "src/filesystem.cpp"
  "src/http_session.cpp" "src/html.cpp" "src/http_request_handler.cpp")
target_link_libraries(geeky LINK_PUBLIC pthread)
