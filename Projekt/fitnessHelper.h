
#include "datatypes.h"


#if defined(_WIN32) // restrict pointers on Windows
#if defined(_MSC_VER) || defined(__ICL)
        #define __restrict__ __restrict
    #endif
#endif

#ifndef PROJEKT_FITNESSHELPER_H
#define PROJEKT_FITNESSHELPER_H

/**
 * Prüft die Übereinstimmung der neuen Punkte mit der Zielform
 * @param modifiedDS  veränderte Punktesammlung
 * @param targetShape Punktesammlung der Zielform
 * @param size        Anzahl an Punkten in modifiedDS
 * @param sizeTarget  Anzahl an Punkten in targetShape
 * @return boolean ob distance kleiner geworden ist oder nicht
 */
bool isBetterFit_VIUnroll4(Coordinates pointNew, Coordinates pointPrev, aligned_vector<Coordinates> targetShape);

bool isBetterFit_VIUnroll8(Coordinates pointNew, Coordinates pointPrev, Coordinates *__restrict__ targetShape, int size);

bool isBetterFit_VIUnroll256(Coordinates pointNew, Coordinates pointPrev, Coordinates *__restrict__ targetShape, int size);

#endif //PROJEKT_FITNESSHELPER_H
