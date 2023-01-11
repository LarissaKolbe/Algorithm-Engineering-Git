#include <iostream>

using namespace std;

int main() {
#pragma omp parallel
  {
#pragma omp sections // add nowait to remove the implicit barrier
    { // Each section is executed once by one of the threads in the team
#pragma omp section // some thread executes this block of code
      { cout << "Hello from section 1\n"; }
#pragma omp section // some thread executes this block of code
      { cout << "Hello from section 2\n"; }
#pragma omp section // some thread executes this block of code
      { cout << "Hello from section 3\n"; }
    } // after the sections construct is an implicit barrier
    cout << "Hello after the sections\n"; // this line is executed by 
                                          // every thread in the team
  }
}
