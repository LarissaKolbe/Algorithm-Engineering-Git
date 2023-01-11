#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;
using Clock = chrono::steady_clock;

// thunk is called from different threads
void thunk(int num_threads, int thread_id, int num_steps, double width,
           double &sum, mutex &m) {
  double sum_local = 0.0; // for summing up heights locally
  for (int i = thread_id; i < num_steps; i += num_threads) {
    double x = (i + 0.5) * width; // midpoint
    sum_local = sum_local + (1.0 / (1.0 + x * x)); // add new height
  }
  lock_guard<mutex> lg(m); // avoids race condition on sum
  sum += sum_local; // update global sum
}

int main() {
  int num_steps = 100000000;              // amount of rectangles
  double width = 1.0 / double(num_steps); // width of a rectangle
  double sum = 0.0; // for summing up all heights of rectangles
  auto start = Clock::now();
  int num_threads = (int) thread::hardware_concurrency(); // amount of logical cores
  mutex m;
  vector<thread> threads;
  threads.reserve(num_threads);

  // start threads
  for (int i = 1; i < num_threads; ++i) {
    thread th(thunk, num_threads, i, num_steps, width, ref(sum), ref(m));
    threads.push_back(move(th));
  }
  // run function thunk on master thread
  thunk(num_threads, 0, num_steps, width, sum, m);

  // join threads
  for (std::thread &th : threads) {
    th.join();
  }
  
  double pi = sum * 4 * width; // compute pi
  double run_time = (Clock::now() - start).count() / 1e9;
  cout << "pi with " << num_steps << " steps is " << setprecision(17) << pi
       << " in " << setprecision(4) << run_time << " seconds\n";
  return 0;
}
