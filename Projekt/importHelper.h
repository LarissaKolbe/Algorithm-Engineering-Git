#include "datatypes.h"

using namespace std;

#ifndef PROJEKT_INPUTHELPER_H
#define PROJEKT_INPUTHELPER_H

/**
 * Das Interface, über das der User Dateien hochladen oder
 *  aus den vorgegebenen Dateien wählen kann.
 * @param type Inputbild oder Zielform
 * @return Information über Bildmaße, Pixeldaten und Dateiformat
 */
FileInformation readImageData(InputType type);


/**
 * Erstellt Vektor anhand der übergebenen Bildinformationen.
 * Der Vektor enthält die Koordinaten aller Punkte, die im Bild schwarz waren.
 * @param info Bildinformationen
 * @return Vektor mit Koordinaten
 */
aligned_vector<Coordinate> createVectorFromImage(const char *imageData, int width, int height);


#endif //PROJEKT_INPUTHELPER_H
