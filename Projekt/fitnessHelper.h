
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
 * (Naive Implementierung)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_Naive (const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Parallele Version der naiven Implementierung)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_Parallel (const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 128-Bit Vector Intrinsics)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI128(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 256-Bit Vector Intrinsics)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI256(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 128-Bit Vector Intrinsics und Loop Unrolling)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI128_Lu(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 256-Bit Vector Intrinsics und Loop Unrolling)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI256_Lu(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 128-Bit Vector Intrinsics und Parallelisierung)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI128_Par(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 156-Bit Vector Intrinsics und Parallelisierung)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI256_Par(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 128-Bit Vector Intrinsics, Loop Unrolling und Parallelisierung)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI128_LuPar(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);
/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * (Implementierung mit 256-Bit Vector Intrinsics, Loop Unrolling und Parallelisierung)
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VI256_LuPar(const Coordinate p1, const Coordinate p2, const Coordinate *__restrict__ targetShape, const int size);

#endif //PROJEKT_FITNESSHELPER_H
