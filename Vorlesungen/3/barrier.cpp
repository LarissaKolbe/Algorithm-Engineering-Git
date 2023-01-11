#include <iostream>
#include <omp.h>
#include <sstream>

using namespace std;

int main() {
#pragma omp parallel
  {
    stringstream info;
    info << "Hello from thread " << omp_get_thread_num() << endl;
    cout << info.str();
#pragma omp barrier // move on as soon as all threads printed Hello ...
    info.str(""); // "clear" stringstream variable
    info << "Goodbye from thread " << omp_get_thread_num() << endl;
    cout << info.str();
  }
}
