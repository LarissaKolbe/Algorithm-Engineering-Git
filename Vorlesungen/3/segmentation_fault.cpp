#include <iostream>
#include <vector>

using namespace std;

int main() {
  vector<int> my_vector{1, 2, 3};
#pragma omp parallel default(none) shared(cout) private(my_vector)
  {
    // program crashes since my_vector is uninitialized (size = 0)
    cout << my_vector[0]; // segmentation fault
  }
}
