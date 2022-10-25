#include <iostream>
#include <thread>

void thunk() { std::cout << "Hello World!" << std::endl; }

int main() {
  std::thread threads[4];
  for (int i = 1; i < 4; ++i)
    threads[i] = std::thread(thunk); // fork threads
  thunk(); // master thread
  for (int i = 1; i < 4; ++i)
    threads[i].join(); // join threads
}
