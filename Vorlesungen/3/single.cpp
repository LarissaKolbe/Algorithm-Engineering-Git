#include <iostream>
#include <omp.h>

using namespace std;

int main() {
#pragma omp parallel
  {
#pragma omp single // only one thread executes the code in the single
    {
      cout << "from single: " << omp_get_thread_num() << "\n";
    } // implicit barrier after single
    cout << "after single\n";
  };
}
