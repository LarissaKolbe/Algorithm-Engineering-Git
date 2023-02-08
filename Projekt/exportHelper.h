#include <vector>
#include "datatypes.h"

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
void exportImage(string fileName, vector<Coordinates> data, int maxColor, int outputHeight, int outputWidth);

#endif //PROJEKT_FILEHELPER_H
