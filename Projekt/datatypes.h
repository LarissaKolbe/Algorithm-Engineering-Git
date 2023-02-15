#pragma once

#include <vector>
#include "aligned_allocator.h"

using namespace std;

#ifndef PROJEKT_DATATYPES_H
#define PROJEKT_DATATYPES_H

/**
 * Speichert die x- und y-Koordinaten eines Punktes
 */
struct Coordinate {
    float x;
    float y;
};

/** Auf 64 Byte alignter Vektor */
template<class T> using aligned_vector = vector<T, alligned_allocator<T, 64>>;

/**
 * Speichert relevante Informationen zu einem hochgeladenen Bild
 */
struct FileInformation {
    /** Die Breite des Bildes */
    int width;
    /** Die Höhe des Bildes */
    int height;
    /** Der maximale Farbwert des Bildes */
    int maxColor;
    /** Das Bildformat*/
    string fileFormat;
    /** Die Bilddaten */
    char *imageData;
};

/**
 * Statistische Eigenschaften eines Datensatzes
 */
struct statisticalProperties {
    /** Durchschnitt der x-Werte */
    float meanX;
    /** Durchschnitt der y-Werte */
    float meanY;
    /** Varianz der x-Werte */
    float varianceX;
    /** Varianz der y-Werte */
    float varianceY;
    /** Standardabweichung der x-Werte */
    float stdDeviationX;
    /** Standardabweichung der y-Werte */
    float stdDeviationY;
};

/**
 * Konfigurationen, mit denen das Programm läuft.
 * Hat veränderbare Standardkonfigurationen.
 */
struct Configurations {
    /**B estimmt, wie sehr die statistische Eigenschaften abweichen dürfen */
    double accuracy = 0;
    /** Bestimmt, wie viele Nachkommestellen betrachtet werden sollen */
    int decimals = 2;
    /** Maximale Bewegung, die ein Punkt mit einem Mal machen kann */
    float maxMovement = 1;
    /** Maximale Temperatur */
    double maxTemp = 0.4;
    /** Minimale Temperatur */
    double minTemp = 0.01;
    /** Anzahl an Iterationen */
    int iterations = 200000;
};

/**
 * Art eines hochgeladenen Bildes
 */
enum InputType {
    /** Der zu verändernde Datensatz */
    initialData,
    /** Die Zielform */
    target,
};

/**
 * Bildgrößen bei vorgegebenen Bildern.
 */
enum InputSize {
    small,
    big,
};

#endif //PROJEKT_DATATYPES_H
