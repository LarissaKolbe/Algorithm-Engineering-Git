#include <iostream>
#include <omp.h>
#include <vector>

using namespace std;

// mock k-nearest neighbors classification
void knn(int k, const string &weight, const string &metric) {
  // larger k needs more runtime
  for (volatile int i = 0; i < 10000000 * k; ++i) {}
  // computing accuracy of classification is omitted here
#pragma omp critical // output which thread did what
  cout << "k: " << k << ", weight: " << weight << ", metric: " << metric
       << ", computed with thread: " << omp_get_thread_num() << endl;
}

int main() {
  vector<int> ks{1, 3, 5, 7, 9, 11};
  vector<string> weights{"uniform", "distance"};
  vector<string> metrics{"euclidean", "manhattan"};
  const double start = omp_get_wtime();
  // parallel grid search for tuning the hyperparameters in knn
#pragma omp parallel for collapse(3) schedule(dynamic)
  for (uint64_t i = 0; i < ks.size(); ++i)
    for (uint64_t j = 0; j < weights.size(); ++j)
      for (uint64_t k = 0; k < metrics.size(); ++k)
        knn(ks[i], weights[j], metrics[k]);

  cout << omp_get_wtime() - start << " seconds" << endl;
}
