int main() {
  const int N = 10;
  const int M = 12;

  /**
   * will form a single loop of length N * M = 120
   * and then parallelize that
   *
   * can be useful for balancing the work
   */
#pragma omp parallel for collapse(2)
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      // do useful work with i and j
    }
  }
}
