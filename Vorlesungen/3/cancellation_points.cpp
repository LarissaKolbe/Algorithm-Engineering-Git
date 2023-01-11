#include <omp.h>
#include <zconf.h>
#include <atomic>
#include <iostream>

using namespace std;

bool is_solution(int number) { // test if number solves the problem
  for (volatile int i = 10000000; i--;) {} // mock computation
  return number > 133 && number < 140;
}

// actually, we can stop a parallel for loop with cancellation points, but it's a little complicated
int main(int argc, char **argv) { // since OpenMP 4.0 we have cancellation points
  if (!omp_get_cancellation()) { // if no cancellations enabled, enable it
    cout << "Enabling cancellation and rerunning program\n" << endl;
    const char* enable_cancellation = "OMP_CANCELLATION=true";
    // const_cast can be used to pass const data to a function that doesn't receive const
    putenv(const_cast<char *>(enable_cancellation));  // set cancellation environment variable
    execv(argv[0], argv); // rerun program, because can't enable cancellations in the program itself
  } // execv replaces the current process image with a new process image
  constexpr int biggest_possible_number = 10000;
  atomic<int> final_solution(INT32_MAX);
  const double start = omp_get_wtime();

#pragma omp parallel // start parallel region
  {
#pragma omp for schedule(dynamic)
    for (int i = 0; i < biggest_possible_number; ++i) {
      if (is_solution(i)) { // find some solution, not necessary the smallest
        final_solution = i;
#pragma omp cancel for // signal cancellation, because we found a solution
      }
#pragma omp cancellation point for // check for cancellations signalled from other threads
      // cancel for loop if cancellations signalled
    }
  } // end parallel region

  // check if we've found a solution at all is omitted, you can add the check
  cout << "The solution is: " << final_solution << endl;
  cout << omp_get_wtime() - start << " seconds" << endl;
}
