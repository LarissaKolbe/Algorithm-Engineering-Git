#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
#include <cmath>
#include <vector>
#include <iostream>
#include "datatypes.h"
#include "helperFunctions.h"
//#include "aligned_allocator.h"
//
//template<class T>
//using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

using namespace std;

/**
 * Rundet den übergebenen Wert auf die angegebene Anzahl an Nachkommastellen.
 * @param value zu rundender Wert
 * @param decimals Anzahl an Nachkommastellen
 * @return gerundeten Wert
 */
float roundValue(float value, int decimals){
    //TODO: schöner machen
    float roundingValue = pow(10, decimals);
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
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return true;
    } else {
        return false;
    }
}


/**
 * Erstellt Vektor anhand der übergebenen Bildinformationen.
 * Der Vektor enthält die Koordinaten aller Punkte, die im Bild schwarz waren.
 * @param info Bildinformationen
 * @return Vektor mit Koordinaten
 */
vector<Coordinates> createVectorFromImage(FileInformation info){
    vector<Coordinates> vec = {};
    //zählt/sucht alle nahezu schwarzen Punkte
    //TODO: beim input einlesen iwie dafür sorgen, dass jeder Punkt nur wirklich einmal registriert wird!
    // (hat halt meistens noch gräuliche Randpunkte, die jetzt auch eingelesen werden...)
#pragma omp parallel for
    for (int i=1;i<info.width * info.height * 3;i=i+3){
        unsigned char red = info.imageData[i];
        unsigned char green = info.imageData[i+1];
        unsigned char blue = info.imageData[i+2];
        if (red < 150 && green < 150 && blue < 150){
            double currentY = (i / 3 / info.width);
            double currentX = (i / 3 - currentY * info.width);
#pragma omp critical
            vec.push_back({currentX, currentY});
        }
    }
    return vec;
}
#pragma clang diagnostic pop