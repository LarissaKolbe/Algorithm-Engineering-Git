
#include "datatypes.h"


#if defined(_WIN32) // restrict pointers on Windows
#if defined(_MSC_VER) || defined(__ICL)
        #define __restrict__ __restrict
    #endif
#endif

#ifndef PROJEKT_FITNESSHELPER_H
#define PROJEKT_FITNESSHELPER_H

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VIUnroll8(Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VIUnroll256(Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size);

bool isBetterFit_Native (Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size);

bool isBetterFit_Parallel (Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size);

bool isBetterFit_VIUnroll8For(Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size);

bool isBetterFit_VIUnroll256For(Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size);

#endif //PROJEKT_FITNESSHELPER_H
