# set minimum cmake version
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# project name and language
# setzt direkt die Sprache auf C++ und schließt damit bspw C aus
project(Projekt LANGUAGES CXX)

# fügt die Unittests hinzu
add_subdirectory(catch)

# war vorher 14 statt 11
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Xclang -fopenmp")
#mit optimierungen compilen:
#set(CMAKE_CXX_FLAGS "-O2 -march=native -Ofast -ffast-math")

# hiermit muss OpenMP verfügbar sein
#find_package(OpenMP REQUIRED)
# klappt hiermit aber iwie nicht...
find_package(Catch2 REQUIRED)

# damit er OpenMP (libomp) findet
#include_directories(/usr/local/Cellar/libomp/15.0.3/include)
#link_directories(/usr/local/Cellar/libomp/15.0.3/lib)
include_directories(/usr/local/opt/libomp/include)
link_directories(/usr/local/opt/libomp/lib)
# aus Terminal dazu:
#libomp is keg-only, which means it was not symlinked into /usr/local,
#because it can override GCC headers and result in broken builds.
#For compilers to find libomp you may need to set:
#  export LDFLAGS="-L/usr/local/opt/libomp/lib"
#  export CPPFLAGS="-I/usr/local/opt/libomp/include"

#set(CPPFLAGS "-I/usr/local/opt/llvm/include -fopenmp")
#set(LDFLAGS "-L/usr/local/opt/llvm/lib")

# für clang compiler
add_executable(Projekt_exec_clang main.cpp aligned_allocator.h helperFunctions.cpp helperFunctions.h datatypes.h configurationHelpers.cpp configurationHelpers.h exportHelper.cpp exportHelper.h importHelper.cpp importHelper.h fitnessHelper.cpp fitnessHelper.h)
target_link_libraries(Projekt_exec_clang PUBLIC -lomp)
target_include_directories(Projekt_exec_clang PUBLIC /usr/local/Cellar/libomp/15.0.3/include)
target_link_directories(Projekt_exec_clang PUBLIC /usr/local/Cellar/libomp/15.0.3/lib)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Projekt_exec_clang PUBLIC OpenMP::OpenMP_CXX)
else()
    target_compile_options(Projekt_exec_clang PUBLIC -Xclang -fopenmp)
endif()
# oben schon einmal für alle targets gemacht:
#target_include_directories(Projekt_exec_clang PUBLIC /usr/local/opt/libomp/include)
#target_link_directories(Projekt_exec_clang PUBLIC /usr/local/opt/libomp/lib)
#unterstützt -fopenmp nicht
#target_link_options(Projekt_exec_clang PUBLIC -fopenmp)
#target_link_libraries(Projekt_exec_clang PUBLIC OpenMP::OpenMP_CXX)
#target_compile_options(Projekt_exec_clang PUBLIC ${OpenMP_CXX_FLAGS})
#set_target_properties(Projekt_exec_clang PROPERTIES LINK_FLAGS ${OpenMP_CXX_FLAGS})


# für gcc compiler
# gcc unterstützt OpenMP nicht
add_executable(Projekt_exec_gcc main.cpp aligned_allocator.h helperFunctions.cpp helperFunctions.h datatypes.h configurationHelpers.cpp configurationHelpers.h exportHelper.cpp exportHelper.h importHelper.cpp importHelper.h fitnessHelper.cpp fitnessHelper.h)
# die Zeile geht evtl nicht:
target_link_options(Projekt_exec_gcc PUBLIC -fopenmp)
# verlinkt das importierte target, dass vom FindOpenMP-Module zur Verfügung gestellt wird
# klappt hiermit nicht, weil das >find_package(OpenMP REQUIRED)< oben nicht geht...
#target_link_libraries(Projekt_exec_gcc PUBLIC OpenMP::OpenMP_CXX)
#target_compile_options(Projekt_exec_gcc PUBLIC ${OpenMP_CXX_FLAGS})
#set_target_properties(Projekt_exec_gcc PROPERTIES LINK_FLAGS ${OpenMP_CXX_FLAGS})

# executable für tests
# clang compiler
enable_testing()
add_executable(Projekt_test_exec_clang catch_main.cpp catch_tests.cpp aligned_allocator.h helperFunctions.cpp helperFunctions.h datatypes.h configurationHelpers.cpp configurationHelpers.h exportHelper.cpp exportHelper.h importHelper.cpp importHelper.h fitnessHelper.cpp fitnessHelper.h)
target_link_libraries(Projekt_test_exec_clang PUBLIC catch)
target_include_directories(Projekt_test_exec_clang PUBLIC /usr/local/Cellar/libomp/15.0.3/include)
target_link_directories(Projekt_test_exec_clang PUBLIC /usr/local/Cellar/libomp/15.0.3/lib)
# die Zeile geht evtl nicht:
target_link_options(Projekt_test_exec_clang PUBLIC -fopenmp)
# macht das executable zu nem test
add_test(NAME PROJEKT_TEST_CLANG COMMAND Projekt_test_exec_clang)

# gcc compiler
enable_testing()
add_executable(Projekt_test_exec_gcc catch_main.cpp catch_tests.cpp aligned_allocator.h helperFunctions.cpp helperFunctions.h datatypes.h configurationHelpers.cpp configurationHelpers.h exportHelper.cpp exportHelper.h importHelper.cpp importHelper.h fitnessHelper.cpp fitnessHelper.h)
target_link_libraries(Projekt_test_exec_gcc PUBLIC catch)
# die Zeile geht evtl nicht:
target_link_options(Projekt_test_exec_gcc PUBLIC -fopenmp)
# macht das executable zu nem test
add_test(NAME PROJEKT_TEST_GCC COMMAND Projekt_test_exec_gcc)


#so könnte man bspw -Ofast als Compileroption einstellen
#target_compile_options(Test PUBLIC -Ofast)

# Compilieren in Kommandozeile am Bsp "Vorlesungen/1/hello.cpp":
# clang++ -std=c++14 -I/usr/local/Cellar/libomp/15.0.3/include/ -L/usr/local/Cellar/libomp/15.0.3/lib/ -Xclang -fopenmp Vorlesungen/1/hello.cpp -o hello -lomp
