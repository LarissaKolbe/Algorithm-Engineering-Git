#include <iostream>
#include <omp.h>
#include <vector>

using namespace std;

int main() {
  int n = 100000000;
  // create a vector of size n, all values 1.0
  // long double is "usually" a 128 bit float data type
  // L indicates that 1.0 is a long double literal
  vector<long double> vec(n, 1.0L);
  long double sum = 0.0L;

  const double start = omp_get_wtime();

#pragma omp parallel for reduction(+ : sum)
  for (int i = 0; i < n; ++i) {
    sum += vec[i]; // sum up all values of vec
  }

  cout << fixed << "sum: " << sum << endl;
  cout << omp_get_wtime() - start << " seconds" << endl;
}
