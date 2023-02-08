#ifndef PROJEKT_DATATYPES_H
#define PROJEKT_DATATYPES_H

struct Coordinates {
    double x;
    double y;
};

struct FileInformation {
    int width;
    int height;
    int maxColor;
    char *imageData;
};

struct statisticalProperties {
    //TODO: füg evtl mehr Eigenschaften hinzu? Aber was?
    //TODO: einbauen, dass man sich aussuchen kann welche gleich sein sollen
    double meanX;
    double meanY;
    double varianceX;
    double varianceY;
    double stdDeviationX;
    double stdDeviationY;
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
    int iterations = 100000;
};

enum InputType {
    initialData,
    targetShape,
};

#endif //PROJEKT_DATATYPES_H
