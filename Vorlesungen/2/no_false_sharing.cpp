#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  const int threads = 4; // number of threads
  const int PAD = 8; // padding value to avoid false sharing
  volatile long arr[threads * PAD]; // no compiler optimizations on arr
  double start_time = omp_get_wtime(); // wall clock time in seconds
#pragma omp parallel num_threads(threads)
  {
    const int thread_id = omp_get_thread_num();
    for (long i = 0; i < 1000000000; ++i) {
      // each thread increments a value on a different cache line
      ++arr[thread_id * PAD];
    }
  }
  cout << omp_get_wtime() - start_time << " seconds\n";
}
