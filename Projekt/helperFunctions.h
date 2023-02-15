#include "datatypes.h"

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
 * Berechnet die euklidische Distancz zwischen zweit Punkten
 * @param p1 Koordinaten von Punkt 1
 * @param p2 Koordinaten von Punkt 2
 * @return Distanz zwischen p1 und p2
 */
float getDistance(Coordinates p1, Coordinates p2);

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

#endif //PROJEKT_HELPERFUNCTIONS_H
