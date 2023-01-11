#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  cout << "num_threads: " << omp_get_num_threads() << "\n"; // prints 1
  // but, if we call omp_get_num_threads in a parallel region
  // then we get the number of threads that are executed inside the parallel region
#pragma omp parallel
#pragma omp single
  cout << "num_threads: " << omp_get_num_threads() << "\n\n"; // e.g 4

  // maximum number of threads in a parallel region (e.g. 4)
  cout << "max_threads: " << omp_get_max_threads() << "\n"; // prints 4
  omp_set_num_threads(2); // change amount of threads in a parallel region
  cout << "max_threads: " << omp_get_max_threads() << "\n\n"; // now it's 2

  // prints 4, if my machine has 4 logical cores
  cout << "num_procs: " << omp_get_num_procs() << "\n\n";
  // logical cores are the number of physical cores times the number
  // of threads (called "hyperthreads") that can run on each core

  cout << "in_parallel: " << omp_in_parallel() << "\n"; // prints 0 ("false")
  // if we call omp_in_parallel in a parallel region then we get 1 ("true")
#pragma omp parallel
#pragma omp single
  cout << "in_parallel: " << omp_in_parallel() << "\n";
}
