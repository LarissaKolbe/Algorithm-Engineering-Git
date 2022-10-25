#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  const int threads = 4; // number of threads
  volatile long arr[threads]; // no compiler optimizations on arr
  double start_time = omp_get_wtime(); // wall clock time in seconds
#pragma omp parallel num_threads(threads)
  {
    const int thread_id = omp_get_thread_num();
    for (long i = 0; i < 1000000000; ++i) {
      // each thread uses (probably) the same cache line
      ++arr[thread_id];
    }
  }
  // cout the byte where arr starts in the cache line
  cout <<  ((long) &arr[0]) % 64 << endl;
  cout << omp_get_wtime() - start_time << " seconds\n";
}
