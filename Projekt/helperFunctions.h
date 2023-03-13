#include "datatypes.h"

using namespace std;

#ifndef PROJEKT_HELPERFUNCTIONS_H
#define PROJEKT_HELPERFUNCTIONS_H

/**
 * Rundet den übergebenen Wert auf die angegebene Anzahl an Nachkommastellen
 * @param value zu rundender Wert
 * @param n Anzahl an Nachkommastellen
 * @return gerundeter Wert
 */
float roundToNDecimals(const float value, const int n);

/**
 * Gibt den größeren der beiden übergebenen Werte zurück
 * @param a Wert 1
 * @param b Wert 2
 */
int getGreaterValue(const int a, const int b);

/**
 * Vergleicht die y-Koordinaten von a und b.
 * Ist y gleich, werden die x-Koordinaten verglichen.
 * @param a Koordinate (x, y)
 * @param b Koordinate (x, y)
 * @return True, wenn a kleiner als b ist, false sonst
 */
bool compareByY(const Coordinate &a, const Coordinate &b);

/**
 * Prüft, ob der zuvor vom User eingegebene Wert den richtigen Typ hat.
 * Resettet Input bei Fehler.
 * @return True bei Fehler, false sonst
 */
bool incorrectInput();

#endif //PROJEKT_HELPERFUNCTIONS_H
