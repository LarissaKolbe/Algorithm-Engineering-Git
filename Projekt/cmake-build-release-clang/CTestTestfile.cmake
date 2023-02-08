# CMake generated Testfile for 
# Source directory: /Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt
# Build directory: /Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/cmake-build-release-clang
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(PROJEKT_TEST_CLANG "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/cmake-build-release-clang/Projekt_test_exec_clang")
set_tests_properties(PROJEKT_TEST_CLANG PROPERTIES  _BACKTRACE_TRIPLES "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/CMakeLists.txt;82;add_test;/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/CMakeLists.txt;0;")
add_test(PROJEKT_TEST_GCC "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/cmake-build-release-clang/Projekt_test_exec_gcc")
set_tests_properties(PROJEKT_TEST_GCC PROPERTIES  _BACKTRACE_TRIPLES "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/CMakeLists.txt;91;add_test;/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/CMakeLists.txt;0;")
subdirs("catch")
