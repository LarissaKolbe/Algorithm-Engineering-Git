#include <iomanip>
#include <iostream>
#include <omp.h>

using namespace std;

int main() {
  int num_steps = 100000000; // amount of rectangles
  double width = 1.0 / double(num_steps); // width of a rectangle
  double start_time = omp_get_wtime(); // wall clock time in seconds
  double pi = 0.0;
#pragma omp parallel num_threads(4)
  {
    int num_threads = omp_get_num_threads();
    int thread_id = omp_get_thread_num();
    for (int i = thread_id; i < num_steps; i += num_threads) {
      double x = (i + 0.5) * width; // midpoint
#pragma omp atomic
      pi = pi + (4.0 / (1.0 + x * x)) * width;
    }
  }
  double run_time = omp_get_wtime() - start_time;
  cout << "pi with " << num_steps << " steps is " << setprecision(17)
       << pi << " in " << setprecision(6) << run_time << " seconds\n";
}
