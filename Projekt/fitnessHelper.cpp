
#include <cmath>
#include <iostream>
#include <immintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include "datatypes.h"
#include "helperFunctions.h"
#include "fitnessHelper.h"


using namespace std;


#if defined(_WIN32) // restrict pointers on Windows
#if defined(_MSC_VER) || defined(__ICL)
        #define __restrict__ __restrict
    #endif
#endif

//definiert loop-unroll-Pragma
#define PRAGMA(X) _Pragma(#X)
#if defined(__INTEL_COMPILER)
#define __unroll(X) PRAGMA(unroll(X))
#elif defined(__clang__)
#define __unroll(X) PRAGMA(clang loop unroll_count(X))
#elif defined(__GNUC__) || defined(__GNUG__)
#define __unroll(X) PRAGMA(GCC unroll(X))
#else
    // do nothing
    #define __unroll(X)
#endif

/**
 * Berechnet die euklidische Distanz der übergebenen Punkte
 * @param p1 Koordinaten von Punkt 1
 * @param p2 Koordinaten von Punkt 2
 * @return Distanz zwischen p1 und p2
 */
float getDistance(Coordinate p1, Coordinate p2){
    auto distX = (float)pow(p1.x - p2.x, 2);
    auto distY = (float)pow(p1.y - p2.y, 2);
    return sqrt(distX + distY);
}


/**
 * Ermittelt die minimalen Werte jedes Vektors im übergebenen Array und schreibt sie an die Stelle 0.
 * Nutzt dazu Vektor Intrinsics.
 * @param distances Array aus Vektoren
 * @return Array mit ermittelten minimalen Werte
 */
__m128 * getMinValues(__m128 *distances) {
#pragma omp simd
    for (int k = 0; k < 4; k++) {
        distances[k] = _mm_min_ps(distances[k], _mm_movehl_ps(distances[k], distances[k]));
    }
#pragma omp simd
    for (int k = 0; k < 4; k++) {
        distances[k] = _mm_min_ps(distances[k], _mm_shuffle_ps(distances[k], distances[k], 1));
    }
    return distances;
}

/**
 * Ermittelt den minimalen Wert des übergebenen Vektors
 * @param v Vektor
 * @return minimaler Werte
 */
float getMinimalValues(__m256 v) {
    float result[8];
    _mm256_storeu_ps(result, v);
    float min_a = result[0];
    for (int i = 1; i < 8; i++) {
        min_a = min(min_a, result[i]);
    }
    return min_a;
}


/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VIUnroll8(Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size){
    /**Vektor, der die x- und y-Koordinaten von p1 und p2 je speichert.
     * Erst x und y von kommt p1, dann von p2. */
    __m128 dataVec = _mm_setr_ps(p1.x, p1.y, p2.x, p2.y);

    //berechnet für beide Punkte die Distanz zum ersten Punkt der Zielform als Startwert
    float minDistP1 = getDistance(p1, targetShape[0]);
    float minDistP2 = getDistance(p2, targetShape[0]);
    /** Die berechneten minimalen Distanzen. Index 0 ist p1, Index 1 p2 */
    vector<float> minimalDistances = {minDistP1, minDistP2};

    //Geht alle Punkte der Zielfom durch und berechnet die Distanzen zu p1 und p2
    //Um sich instruction-level Parallelismus zunutze zu machen, wird loop Unrolling mit k=8 gemacht
    int i=1;
    for (; i < size - 8 + 1; i+=8) {
        /** Für Distanzberechnungen genutzter Array aus Vektoren.
         *  Speichert erst die Zielkoordinaten und danach alle Zwischenergebnisse. */
        __m128 calcTmp[8] = {};

#pragma omp simd aligned(targetShape : 64)
        //speichert Koordinaten in "calcTmp"
        for (int k = 0; k < 8; k++) {
            calcTmp[k] = _mm_setr_ps(
                    targetShape[i + k].x, targetShape[i + k].y,
                    targetShape[i + k].x, targetShape[i + k].y
                    );
        }
#pragma omp simd
        //Subtrahiert Werte aus "calcTmp" von p1 und p2 Koordinaten
        for (int k = 0; k < 8; k++) {
            calcTmp[k] = _mm_sub_ps(dataVec, calcTmp[k]);
        }
#pragma omp simd
        //quadriert die Werte
        for (int k = 0; k < 8; k++) {
            calcTmp[k] = _mm_mul_ps(calcTmp[k], calcTmp[k]);
        }
#pragma omp simd
        //Addiert je die beiden Werte die zum selben Punkt gehörten zusammen
        for (int k = 0; k < 8; k++) {
            calcTmp[k] = _mm_hadd_ps(calcTmp[k], calcTmp[k]);
        }
        /**Für die nächsten Schritte verwendeter Array zum speichern der Zwischenergebnisse.*/
        __m128 distances[4] = {};

        //Fügt berechnete Summen in neuen Vektor-Array "distances" ein
        //Da es jetzt nur noch einen Wert pro Koordinate gibt, werden zwei Vektoren aus "calcTmp" in einen Vektor in "distances" gespeichert
        // grader index -> p1, ungrader Index -> p2
        int index = 0;
        for (int k = 0; k < 8; k += 4) {
            distances[index] = _mm_setr_ps(calcTmp[k][0], calcTmp[k + 1][0], calcTmp[k + 2][0], calcTmp[k + 3][0]);
            distances[index + 1] = _mm_setr_ps(calcTmp[k][1], calcTmp[k + 1][1], calcTmp[k + 2][1], calcTmp[k + 3][1]);
            index += 2;
        }
#pragma omp simd
        //Berechnet Wurzel aller Werte
        for (int k = 0; k < 4; k++) {
            distances[k] = _mm_sqrt_ps(distances[k]);
        }

        //Ermittelt die minimalen Werte pro Vektor und schreibt sie in Index 0
        getMinValues(distances);

        //vergleicht minimale Werte pro Vektor mit minimalen Werten in "minimalDistances"
        // grader index -> p1, ungrader Index -> p2
        for (int k = 0; k < 4; k++) {
            minimalDistances[k % 2] = min(minimalDistances[k % 2], _mm_cvtss_f32(distances[k]));
        }
    }
    //remainder loop
    __m128 dataVec128 = _mm_setr_ps(p1.x, p1.y, p2.x, p2.y);
    for (; i < size; i++) {
        /** Für Distanzberechnungen genutzter Array aus Vektoren.
         *  Speichert erst die Zielkoordinaten und danach alle Zwischenergebnisse. */
        __m128 calcTmp = _mm_setr_ps(
                targetShape[i].x, targetShape[i].y,
                targetShape[i].x, targetShape[i].y
        );
        calcTmp = _mm_sub_ps(dataVec128, calcTmp);
        calcTmp = _mm_mul_ps(calcTmp, calcTmp);
        calcTmp = _mm_hadd_ps(calcTmp, calcTmp);
        //Abwechselnd Werte für p1 und p2
        for (int k = 0; k < 2; k++) {
            minimalDistances[k] = min(minimalDistances[k],  sqrt(calcTmp[k]));
        }
    }

    //vergleicht minimale Distanz von p1 mit der von  p2
    return minimalDistances[0] < minimalDistances[1];
}

/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit_VIUnroll256(Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size){
    /**Vektor, der die x- und y-Koordinaten von p1 und p2 je 2x speichert.
     * Index 0 bis 4 sind p1, 5 bis 8 p2. */
    __m256 dataVec = _mm256_setr_ps(p1.x, p1.y, p1.x, p1.y, p2.x, p2.y, p2.x, p2.y);

    //berechnet für beide Punkte die Distanz zum ersten Punkt der Zielform als Startwert
    float minDistP1 = getDistance(p1, targetShape[0]);
    float minDistP2 = getDistance(p2, targetShape[0]);
    /** Die berechneten minimalen Distanzen. Index 0 ist p1, Index 1 p2 */
    vector<float> minimalDistances = {minDistP1, minDistP2};

    //Geht alle Punkte der Zielfom durch und berechnet die Distanzen zu p1 und p2
    //Um sich instruction-level Parallelismus zunutze zu machen, wird loop Unrolling mit k=8 gemacht
    //Jeder Vektor kann die Koordinaten zu 4 Punkten speichern.
    //Da wir gleichzeitig die Distanzen zu zwei verschiedenen Punkten berechnen wollen,
    // werden pro Vektor zwei mal zwei Punkte aus targetShape gespeichert (daher Inkrementierung mit 8*2 = 16)
    //TODO: das unroll muss ich evtl doch manuell machen...
    int i=1;
    for (; i < size-16+1; i+=16) {
        /** Für Distanzberechnungen genutzter Array aus Vektoren.
         *  Speichert erst die Zielkoordinaten und danach alle Zwischenergebnisse. */
        __m256 calcTmp[8] = {};
        
#pragma omp simd aligned(targetShape : 64)
        //speichert Koordinaten in "calcTmp"
        for (int k = 0; k < 8 * 2; k += 2) {
            calcTmp[k / 2] = _mm256_setr_ps(
                    targetShape[i + k].x, targetShape[i + k].y,
                    targetShape[i + k + 1].x,targetShape[i + k + 1].y,
                    targetShape[i + k].x, targetShape[i + k].y,
                    targetShape[i + k + 1].x,targetShape[i + k + 1].y
                    );
        }
#pragma omp simd
        //Subtrahiert Werte aus "calcTmp" von p1 und p2 Koordinaten
        for (int k = 0; k < 8; k ++) {
            calcTmp[k] = _mm256_sub_ps(dataVec, calcTmp[k]);
        }
#pragma omp simd
        //quadriert die Werte
        for (int k = 0; k < 8; k ++) {
            calcTmp[k] = _mm256_mul_ps(calcTmp[k], calcTmp[k]);
        }
#pragma omp simd
        //Addiert je die beiden Werte die zum selben Punkt gehörten zusammen
        for (int k = 0; k < 8; k ++) {
            calcTmp[k] = _mm256_hadd_ps(calcTmp[k], calcTmp[k]);
        }
        /**Für die nächsten Schritte verwendeter Array zum speichern der Zwischenergebnisse.*/
        __m256 distances[4] = {};

        //Fügt berechnete Summen in neuen Vektor-Array "distances" ein
        //Da es jetzt nur noch einen Wert pro Koordinate gibt, werden zwei Vektoren aus "calcTmp" in einen Vektor in "distances" gespeichert
        // grader index -> p1, ungrader Index -> p2
        int index = 0;
        for (int k = 0; k < 8; k += 4) {
            distances[index] = _mm256_setr_ps(
                    calcTmp[k][0], calcTmp[k+1][0], calcTmp[k+2][0], calcTmp[k+3][0],
                    calcTmp[k][1], calcTmp[k+1][1], calcTmp[k+2][1], calcTmp[k+3][1]
                    );
            distances[index+1] = _mm256_setr_ps(
                    calcTmp[k][4], calcTmp[k+1][4],calcTmp[k+2][4], calcTmp[k+3][4],
                    calcTmp[k][5], calcTmp[k+1][5], calcTmp[k+2][5], calcTmp[k+3][5]
                    );
            index += 2;
        }
#pragma omp simd
        //Berechnet Wurzel aller Werte
        for (int k = 0; k < 4; k++) {
            distances[k] = _mm256_sqrt_ps(distances[k]);
        }

        /** Speichert die minimalen Werte pro Vektor in "distances"*/
        float minValues[4] = {};
        //ermittelt minimale Werte pro Vektor
#pragma omp simd
        for (int k = 0; k < 4; k++) {
            minValues[k] = getMinimalValues(distances[k]);
        }

        //vergleicht "minValues" mit aktuellen minimalen Werten
        // grader index -> p1, ungrader Index -> p2
        for (int k = 0; k < 4; k++) {
            minimalDistances[k % 2] = min(minimalDistances[k % 2], minValues[k]);
        }
    }

    //remainder loop
    __m128 dataVec128 = _mm_setr_ps(p1.x, p1.y, p2.x, p2.y);
    for (; i < size; i++) {
        /** Für Distanzberechnungen genutzter Array aus Vektoren.
         *  Speichert erst die Zielkoordinaten und danach alle Zwischenergebnisse. */
        __m128 calcTmp = _mm_setr_ps(
                targetShape[i].x, targetShape[i].y,
                targetShape[i].x, targetShape[i].y
                );
        calcTmp = _mm_sub_ps(dataVec128, calcTmp);
        calcTmp = _mm_mul_ps(calcTmp, calcTmp);
        calcTmp = _mm_hadd_ps(calcTmp, calcTmp);
        //Abwechselnd Werte für p1 und p2
        for (int k = 0; k < 2; k++) {
            minimalDistances[k] = min(minimalDistances[k],  sqrt(calcTmp[k]));
        }
    }

    //vergleicht minimale Distanz von p1 mit der von  p2
    return minimalDistances[0] < minimalDistances[1];
}


bool isBetterFitControllNative (Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size){
    float minDistNew = getDistance(p1, targetShape[0]);
    float minDistPrev = getDistance(p2, targetShape[0]);

    for (int j = 1; j < size; j++) {
        minDistNew = min(minDistNew, getDistance(p1, targetShape[j]));
        minDistPrev = min(minDistPrev, getDistance(p2, targetShape[j]));
    }
    return minDistNew < minDistPrev;
}


bool isBetterFitControllParallel (Coordinate p1, Coordinate p2, Coordinate *__restrict__ targetShape, int size){
    float minDistNew = getDistance(p1, targetShape[0]);
    float minDistPrev = getDistance(p2, targetShape[0]);

#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
    for (int j = 1; j < size; j++) {
        minDistNew = min(minDistNew, getDistance(p1, targetShape[j]));
        minDistPrev = min(minDistPrev, getDistance(p2, targetShape[j]));
    }
    return minDistNew < minDistPrev;
}

