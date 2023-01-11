#include <omp.h>
#include <atomic>
#include <iostream>

using namespace std;

bool is_solution(int number) { // test if number solves the problem
  for (volatile int i = 10000000; i--;) {} // mock computation
  return number > 133 && number < 140;
}

int main() { // find smallest solution with the ordered clause
  constexpr int biggest_possible_number = 10000;
  atomic<bool> solution_found(false); // if true than we found the solution
  int final_solution = INT32_MAX;
  const double start = omp_get_wtime();
#pragma omp parallel for ordered schedule(dynamic) // start parallel region
  for (int i = 0; i < biggest_possible_number; ++i) {
    if (solution_found) // we found the solution, just continue iterating
      continue;
    if (is_solution(i)) {
#pragma omp ordered // ordered region
      if (!solution_found) { // ordered execution of if statement
        solution_found = true; // no race condition 
        final_solution = i;
      }
    }
  } // end parallel region
  cout << "The solution is: " << final_solution << endl;
  cout << omp_get_wtime() - start << " seconds" << endl;
}
