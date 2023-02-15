
#include <cmath>
#include <limits>
#include <iostream>
#include "datatypes.h"
#include "helperFunctions.h"

using namespace std;

/**
 * Rundet den übergebenen Wert auf n Nachkommastellen.
 * @param value zu rundender Wert
 * @param n     Anzahl an Nachkommastellen
 * @return auf n Nachkommastellen gerundeter Wert
 */
float roundToNDecimals(float value, int n){
    auto roundingValue = (float)pow(10, n);
    return round(value * roundingValue) / roundingValue;
}

/**
 * Gibt den größeren der beiden übergebenen Werte zurück
 * @param a Wert 1
 * @param b Wert 2
 */
int getGreaterValue(int a, int b){
    if (a > b){
        return a;
    } else {
        return b;
    }
}

/**
 * Vergleicht die y-Koordinaten von a und b.
 * Ist y gleich, werden die x-Koordinaten verglichen.
 * @param a Koordinate (x, y)
 * @param b Koordinate (x, y)
 * @return True, wenn a kleiner als b ist, false sonst
 */
bool compareByY(const Coordinate &a, const Coordinate &b) {
    if(a.y == b.y){
        return a.x < b.x;
    } else {
        return a.y < b.y;
    }
}

/**
 * Prüft, ob der zuvor vom User eingegebene Wert den richtigen Typ hat.
 * Resettet Input bei Fehler.
 * @return True bei Fehler, false sonst
 */
bool incorrectInput(){
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return true;
    } else {
        return false;
    }
}
