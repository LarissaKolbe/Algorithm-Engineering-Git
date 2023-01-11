
# set minimum cmake version
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# project name and language
# setzt direkt die Sprache auf C++ und schließt damit bspw C aus
project(test LANGUAGES CXX)

# fügt die Unittests hinzu
add_subdirectory(catch)

#wird global für alle Targets gesetzt
include_directories(/usr/local/Cellar/libomp/15.0.3/include)
link_directories(/usr/local/Cellar/libomp/15.0.3/lib)

#Mit denen klappt es iwie nicht...
#include_directories(/usr/local/opt/gcc/lib/gcc/include)
#link_directories(/usr/local/opt/gcc/lib/gcc/lib)

# hiermit muss OpenMP verfügbar sein
# klappt hiermit aber iwie nicht...
#find_package(OpenMP REQUIRED)

set(CMAKE_CXX_FLAGS "-Xclang -fopenmp")

add_executable(Test main.cpp catch_main.cpp catch_main.h)

# wird nur für das angegebene Target "Test" gesetzt
target_link_libraries(Test PUBLIC -lomp)
# verlinkt das importierte target, dass vom FindOpenMP-Module zur Verfügung gestellt wird
#k lappt hiermit nicht, weil das >find_package(OpenMP REQUIRED)< oben nicht geht...
#target_link_libraries(Test PUBLIC OpenMP::OpenMP_CXX)

#so könnte man bspw -Ofast als Compileroption einstellen
#target_compile_options(Test PUBLIC -Ofast)

# Compilieren in Kommandozeile am Bsp "Vorlesungen/1/hello.cpp":
# clang++ -std=c++14 -I/usr/local/Cellar/libomp/15.0.3/include/ -L/usr/local/Cellar/libomp/15.0.3/lib/ -Xclang -fopenmp Vorlesungen/1/hello.cpp -o hello -lomp
