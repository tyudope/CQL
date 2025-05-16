cmake_minimum_required(VERSION 3.30)
project(2425S_36_s33894_UD)

set(CMAKE_CXX_STANDARD 20)

# Add your source files here
add_executable(dbProject main.cpp)


include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         11.0.2
)

FetchContent_MakeAvailable(fmt)


target_link_libraries(dbProject fmt)

