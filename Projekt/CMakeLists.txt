# set minimum cmake version
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

# project name and language
project(Projekt LANGUAGES CXX)

# fügt die Unittests hinzu
add_subdirectory(catch)

# war vorher 14 statt 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# linkt auf die standard library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lstdc++ ")
# verlinkt m-Library
link_libraries(m)

# Compilerflags um CPU voll auszunutzen und Vector Intrinsics zu nutzen
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -mavx -msse2")

# Debuggerflags:
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -fsanitize=address -fsanitize=undefined ")

# Optimierungsflags:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Ofast -ffast-math")

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    link_libraries(OpenMP::OpenMP_CXX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# damit er OpenMP (libomp) findet
if(APPLE)
    include_directories(/usr/local/opt/libomp/include)
    link_directories(/usr/local/opt/libomp/lib)
    link_libraries(-lomp)
elseif(WINDOWS)
    link_libraries(/openmp)
endif()


# executable
add_executable(Projekt_exec main.cpp aligned_allocator.h helperFunctions.cpp helperFunctions.h datatypes.h configurationHelpers.cpp configurationHelpers.h exportHelper.cpp exportHelper.h importHelper.cpp importHelper.h fitnessHelper.cpp fitnessHelper.h)

# executable für tests
enable_testing()
add_executable(Projekt_test_exec catch_main.cpp aligned_allocator.h helperFunctions.cpp helperFunctions.h datatypes.h configurationHelpers.cpp configurationHelpers.h exportHelper.cpp exportHelper.h importHelper.cpp importHelper.h fitnessHelper.cpp fitnessHelper.h
        catch_tests_importHelper.cpp catch_tests_helperFunctions.cpp catch_tests_configurationHelper.cpp catch_tests_fitnessHelper.cpp)
target_link_libraries(Projekt_test_exec PUBLIC catch)
add_test(NAME PROJEKT_TEST COMMAND Projekt_test_exec)
