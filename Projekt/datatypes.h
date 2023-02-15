#include <vector>
#include "aligned_allocator.h"

using namespace std;

#ifndef PROJEKT_DATATYPES_H
#define PROJEKT_DATATYPES_H

struct Coordinates {
    float x;
    float y;
};

template<class T>
using aligned_vector = vector<T, alligned_allocator<T, 64>>;

struct FileInformation {
    int width;
    int height;
    int maxColor;
    string fileFormat;
    char *imageData;
};

struct statisticalProperties {
    //TODO: füg evtl mehr Eigenschaften hinzu? Aber was?
    //TODO: einbauen, dass man sich aussuchen kann welche gleich sein sollen
    float meanX;
    float meanY;
    float varianceX;
    float varianceY;
    float stdDeviationX;
    float stdDeviationY;
};

struct Configurations {
    /** Wie genau müssen die Werte übereinstimmen bzw wie sehr dürfen sie abweichen*/
    double accuracy = 0;
    /** Wie viele Nachkommastellen werden berücksichtigt*/
    int decimals = 2;
    /** Maximale Bewegung, die ein Punkt mit einem Mal machen kann */
    double maxMovement = 1;
    /** Maximale Temperatur */
    double maxTemp = 0.4;
    /** Minimale Temperatur */
    double minTemp = 0.01;
    /** Anzahl an Schritten */
    int iterations = 400000;
};

enum InputType {
    initialData,
    target,
};

enum InputSize {
    small,
    big,
};

#endif //PROJEKT_DATATYPES_H
