#include <iostream>
#include <atomic>
#include <omp.h>

using namespace std;

bool is_solution(int number) { // test if number solves the problem
  for (volatile int i = 10000000; i--;) {} // mock computation
  return number > 133 && number < 140;
}

int main() {
  constexpr int biggest_possible_number = 10000;
  atomic<int> final_solution(INT32_MAX); // now we use an atomic
  const double start = omp_get_wtime();

#pragma omp parallel for schedule(dynamic) // start parallel region
  for (int i = 0; i < biggest_possible_number; ++i) {
    if (final_solution < i) // we found a smaller solution, just continue iterating
      continue;
    if (is_solution(i)) { // one solution found, update final_solution if it's smaller
      int previous = final_solution.load();
      while (previous > i && !final_solution.compare_exchange_weak(previous, i)) {}
      // http://www.cplusplus.com/reference/atomic/atomic/compare_exchange_weak/
      // atomic.compare_exchange_weak(T& expected, T desired);
      // 1. Compare a given value expected with the value stored in atomic.
      // 2. If both values stored in expected and atomic coincide then set atomic to a
      // given value desired, otherwise write the actual value stored in atomic to expected.
      // 3. Return true if the swap in 2. was successful, otherwise return false.
    }
  } // end parallel region
  // check if we've found a solution at all is omitted, you can add the check
  cout << "The solution is: " << final_solution << endl;
  cout << omp_get_wtime() - start << " seconds" << endl;
}
