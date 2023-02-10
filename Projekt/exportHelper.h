#include <vector>
#include "datatypes.h"
#include "aligned_allocator.h"

template<class T>
using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

using namespace std;

#ifndef PROJEKT_FILEHELPER_H
#define PROJEKT_FILEHELPER_H

string readExportPath();

/**
 * Exportiert ppm-File mit schwarzen Pixeln an den in data angegebenen Koordinaten.
 * @param fileName
 * @param data
 * @param maxColor Maximal color value
 * @param outputHeight
 * @param outputWidth
 */
void exportImage(string fileName, aligned_vector<Coordinates> data, int maxColor, int outputHeight, int outputWidth);

#endif //PROJEKT_FILEHELPER_H
