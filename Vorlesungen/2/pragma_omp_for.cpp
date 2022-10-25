#include <iostream>
#include <omp.h>
#include <vector>
using namespace std;

struct image;

image *denoise_image(image *in) { // mock denoise
  image *out = nullptr;
  for (volatile int i = 0; i < 20000000; ++i) {}
  return out;
}

int main() {
  int amount_images = 64;
  vector<image *> images(amount_images, nullptr); // mock images
  vector<image *> denoised_images(amount_images); // mock output
  double start = omp_get_wtime();
#pragma omp parallel
  {
#pragma omp for // denoise a bunch of images
    for (int i = 0; i < amount_images; ++i) {
      denoised_images[i] = denoise_image(images[i]);
    }
  }
  cout << omp_get_wtime() - start << " seconds" << endl;
}
