
# set minimum cmake version
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# project name and language
project(test LANGUAGES CXX)

include_directories(/usr/local/Cellar/libomp/15.0.3/include)
link_directories(/usr/local/Cellar/libomp/15.0.3/lib)

set(CMAKE_CXX_FLAGS "-Xclang -fopenmp")

add_executable(Test main.cpp)

target_link_libraries(Test PUBLIC -lomp)

# Compilieren in Kommandozeile am Bsp "Vorlesungen/1/hello.cpp":
# clang++ -std=c++14 -I/usr/local/Cellar/libomp/15.0.3/include/ -L/usr/local/Cellar/libomp/15.0.3/lib/ -Xclang -fopenmp Vorlesungen/1/hello.cpp -o hello -lomp
