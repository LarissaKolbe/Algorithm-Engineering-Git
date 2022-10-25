#include <iostream>
#include <omp.h>
#include <random>

using namespace std;

int main() {
  int n = 100000000; // amount of points to generate
  int counter = 0; // counter for points in the first quarter of a unit circle
  auto start_time = omp_get_wtime(); // omp_get_wtime() is an OpenMP library routine

  #pragma omp parallel num_threads(10)
	{
		int threadNum = omp_get_num_threads();
		long unsigned int threadId  = omp_get_thread_num(); // I get a warning if I use normal int
		default_random_engine re{threadId};
		uniform_real_distribution<double> zero_to_one{0.0, 1.0};
		int counterLocal = 0;
		// compute n points and test if they lie within the first quadrant of a unit circle
		for (int i = threadId; i < n; i+=threadNum) {
			auto x = zero_to_one(re); // generate random number between 0.0 and 1.0
			auto y = zero_to_one(re); // generate random number between 0.0 and 1.0
			if (x * x + y * y <= 1.0) { // if the point lies in the first quadrant of a unit circle
				++counterLocal;
			}
		}
		#pragma omp atomic
		counter += counterLocal;
	}

  auto run_time = omp_get_wtime() - start_time;
  auto pi = 4 * (double(counter) / n);

  cout << "pi: " << pi << endl;
  cout << "run_time: " << run_time << " s" << endl;
  cout << "n: " << n << endl; }
