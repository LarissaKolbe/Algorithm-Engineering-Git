#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

int main() {
#pragma omp parallel num_threads(4)
    { std::cout << "Hello World!" << std::endl; }
}
