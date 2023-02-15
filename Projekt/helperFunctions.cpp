
#include <cmath>
#include <limits>
#include <iostream>
#include "datatypes.h"
#include "helperFunctions.h"

using namespace std;

/**
 * Rundet den übergebenen Wert auf die angegebene Anzahl an Nachkommastellen.
 * @param value zu rundender Wert
 * @param decimals Anzahl an Nachkommastellen
 * @return gerundeten Wert
 */
float roundValue(float value, int decimals){
    //TODO: schöner machen
    auto roundingValue = (float)pow(10, decimals);
    value = round(value * roundingValue) / roundingValue;
    return value;
}

bool compareByY(const Coordinates &a, const Coordinates &b) {
    if(a.y == b.y){
        return a.x < b.x;
    } else {
        return a.y < b.y;
    }
}

bool incorrectInput(){
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return true;
    } else {
        return false;
    }
}

