#include <vector>
#include "datatypes.h"
//#include "aligned_allocator.h"
//
//template<class T>
//using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

using namespace std;

#ifndef PROJEKT_HELPERFUNCTIONS_H
#define PROJEKT_HELPERFUNCTIONS_H

/**
 * Rundet den übergebenen Wert auf die angegebene Menge an Nachkommastellen
 * @param value zu rundender Wert
 * @param decimals Menge an Nachkommastellen
 * @return gerundeter Wert
 */
float roundValue(float value, int decimals);

/**
 * Vergleicht die y-Koordinaten von a und b.
 * Ist y gleich werden die x-Koordinaten verglichen.
 * @param a Koordinate (x, y)
 * @param b Koordinate (x, y)
 * @return True, wenn a kleiner als b ist, false sonst
 */
bool compareByY(const Coordinates &a, const Coordinates &b);

/**
 * Prüft, ob der zuvor vom user eingegebene Wert den richtigen Typ hat.
 * @return True bei Fehler, false sonst
 */
bool incorrectInput();

/**
 * Erstellt Vektor anhand der übergebenen Bildinformationen.
 * Der Vektor enthält die Koordinaten aller Punkte, die im Bild schwarz waren.
 * @param info Bildinformationen
 * @return Vektor mit Koordinaten
 */
vector<Coordinates> createVectorFromImage(FileInformation info);

#endif //PROJEKT_HELPERFUNCTIONS_H
