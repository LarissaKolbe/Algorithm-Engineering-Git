#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  int x = 10;
  omp_set_num_threads(4);

  cout << "private(x): ";
#pragma omp parallel private(x)
  { // x is an uninitialized local copy in each thread
    x += omp_get_thread_num(); // (probably) 0 + thread_num
#pragma omp critical
    cout << x << " "; // (probably) something like: 3 2 1 0
  }

  cout << "\nfirstprivate(x): ";
#pragma omp parallel firstprivate(x)
  { // x is an initialized local one-to-one copy in each thread
    x += omp_get_thread_num(); // 10 + thread_num
#pragma omp critical
    cout << x << " "; // something like: 10 12 11 13
  }

  cout << "\n'global' x: " << x << endl; // x = 10 (global x didn't change)
}
