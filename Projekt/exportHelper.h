#include <string>
#include "datatypes.h"

using namespace std;

#ifndef PROJEKT_FILEHELPER_H
#define PROJEKT_FILEHELPER_H

/**
 * Das Interface, über das der User auswählt, wo der Output gespeichert werden soll.
 * Achtung: Es wird nicht geprüft, ob der Pfad tatsächlich valide ist!
 * @return Outputpfad und -name
 */
string readExportPath();

/**
 * Exportiert ppm-Bild mit schwarzen Pixeln an den in data angegebenen Koordinaten.
 * @param fileName Outputpfad und -name
 * @param data     Koordinaten, an denen ein shcwarzer Punkt sein soll
 * @param maxColor maximaler Farbwert
 * @param height   Höhe des Bildes
 * @param width    Breite des Bildes
 */
void exportImage(const string fileName, aligned_vector<Coordinate> data, const int maxColor, const int height, const int width);

#endif //PROJEKT_FILEHELPER_H
