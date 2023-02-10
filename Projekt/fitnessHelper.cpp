#include <vector>
#include <immintrin.h>
#include "datatypes.h"
#include "fitnessHelper.h"
#include "helperFunctions.h"
#include "aligned_allocator.h"

template<class T>
using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

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
 * Prüft die Übereinstimmung der neuen Punkte mit der Zielform
 * @param modifiedDS  veränderte Punktesammlung
 * @param targetShape Punktesammlung der Zielform
 * @param size        Anzahl an Punkten in modifiedDS
 * @param sizeTarget  Anzahl an Punkten in targetShape
 * @return boolean ob distance kleiner geworden ist oder nicht
 */
bool isBetterFit_VIUnroll4(Coordinates pointNew, Coordinates pointPrev, aligned_vector<Coordinates> targetShape){
    //erstellt Vektor, der die Koordinaten der beiden Punkte enthält
    __m128 dataVec = _mm_setr_ps(pointNew.x, pointNew.y, pointPrev.x, pointPrev.y);

    //berechnet für beide Punkte die Distanz zum ersten Punkt der Zielform
    float minDistNew = getDistance(pointNew, targetShape[0]);
    float minDistPrev = getDistance(pointPrev, targetShape[0]);

    //bestimmt die minimale Distanz der beiden Punkte zu einem Punkt der Zielform
#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
//    for (int j = 1; j < numberPointsTarget; j++) {
    __unroll(4) for (int j = 1; j < targetShape.size(); j+=4) {
        __m128 targetVec = _mm_setr_ps(targetShape[j].x, targetShape[j].y, targetShape[j].x, targetShape[j].y);
        __m128 targetVec1 = _mm_setr_ps(targetShape[j+1].x, targetShape[j+1].y, targetShape[j+1].x, targetShape[j+1].y);
        __m128 targetVec2 = _mm_setr_ps(targetShape[j+2].x, targetShape[j+2].y, targetShape[j+2].x, targetShape[j+2].y);
        __m128 targetVec3 = _mm_setr_ps(targetShape[j+3].x, targetShape[j+3].y, targetShape[j+3].x, targetShape[j+3].y);

        __m128 sub = _mm_sub_ps(dataVec, targetVec);
        __m128 sub1 = _mm_sub_ps(dataVec, targetVec1);
        __m128 sub2 = _mm_sub_ps(dataVec, targetVec2);
        __m128 sub3 = _mm_sub_ps(dataVec, targetVec3);

        __m128 mul = _mm_mul_ps(sub, sub);
        __m128 mul1 = _mm_mul_ps(sub1, sub1);
        __m128 mul2 = _mm_mul_ps(sub2, sub2);
        __m128 mul3 = _mm_mul_ps(sub3, sub3);

        __m128 hsum = _mm_hadd_ps(mul, mul);
        __m128 hsum1 = _mm_hadd_ps(mul1, mul1);
        __m128 hsum2 = _mm_hadd_ps(mul2, mul2);
        __m128 hsum3 = _mm_hadd_ps(mul3, mul3);

        __m128 hsumsMod = _mm_setr_ps(hsum[0],hsum1[0],hsum2[0],hsum3[0]);
        __m128 hsumsPrev = _mm_setr_ps(hsum[1],hsum1[1],hsum2[1],hsum3[1]);
        __m128 distsMod = _mm_sqrt_ps(hsumsMod);
        __m128 distsPrev = _mm_sqrt_ps(hsumsPrev);

        //get the smallest value in distsMod and distsPrev
        __m128 currentMinDistMod = _mm_min_ps(distsMod, _mm_movehl_ps(distsMod, distsMod));
        __m128 currentMinDistPrev = _mm_min_ps(distsPrev, _mm_movehl_ps(distsPrev, distsPrev));
        currentMinDistMod = _mm_min_ps(currentMinDistMod, _mm_shuffle_ps(currentMinDistMod, currentMinDistMod, 1));
        currentMinDistPrev = _mm_min_ps(currentMinDistPrev, _mm_shuffle_ps(currentMinDistPrev, currentMinDistPrev, 1));

        //sets minimal distances to minimum of current minimal value and minimal value of vectors
        minDistNew = min(minDistNew, _mm_cvtss_f32(currentMinDistMod));
        minDistPrev = min(minDistPrev, _mm_cvtss_f32(currentMinDistPrev));

    }
    //prüft, ob der neue Punkt näher an der Zielform ist als zuvor
    return minDistNew < minDistPrev;
}

bool isBetterFit_VIUnroll8(Coordinates pointNew, Coordinates pointPrev, Coordinates *__restrict__ targetShape, int size){
    //erstellt Vektor, der die Koordinaten der beiden Punkte enthält
    __m128 dataVec = _mm_setr_ps(pointNew.x, pointNew.y, pointPrev.x, pointPrev.y);

    //berechnet für beide Punkte die Distanz zum ersten Punkt der Zielform
    float minDistNew = getDistance(pointNew, targetShape[0]);
    float minDistPrev = getDistance(pointPrev, targetShape[0]);
    float minimalDistances[2] = {minDistNew, minDistPrev};

    auto targetVec = (__m128 *) targetShape;

    int unrollNumber = 8;

    //bestimmt die minimale Distanz der beiden Punkte zu einem Punkt der Zielform
//#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
//    for (int j = 1; j < numberPointsTarget; j++) {
    __unroll(8) for (int j = 1; j < size; j+=unrollNumber) {
        __m128 calcTmp[8] = {};
#pragma omp simd aligned(targetVec : 64)
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_setr_ps(targetShape[j+k].x, targetShape[j+k].y, targetShape[j+k].x, targetShape[j+k].y);
        }
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_sub_ps(dataVec, calcTmp[k]);
        }
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_mul_ps(calcTmp[k], calcTmp[k]);
        }
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_hadd_ps(calcTmp[k], calcTmp[k]);
        }
        __m128 distance[4] = {};
        int index = 0;
        //grader index -> neuer Punkt, ungrader Index -> vorheriger Punkt
        for (int k = 0; k < unrollNumber; k+=4) {
            distance[index] = _mm_setr_ps(calcTmp[k][0],calcTmp[k+1][0],calcTmp[k+2][0],calcTmp[k+3][0]);
            distance[index+1] = _mm_setr_ps(calcTmp[k][1],calcTmp[k+1][1],calcTmp[k+2][1],calcTmp[k+3][1]);
            index+=2;
        }
        for (int k = 0; k < 4; k++) {
            distance[k] = _mm_sqrt_ps(distance[k]);
        }
        //get the smallest value in distsMod and distsPrev
        for (int k = 0; k < 4; k++) {
            distance[k] = _mm_min_ps(distance[k], _mm_movehl_ps(distance[k], distance[k]));
        }
        for (int k = 0; k < 4; k++) {
            distance[k] = _mm_min_ps(distance[k], _mm_shuffle_ps(distance[k], distance[k], 1));
        }
        //sets minimal distances to minimum of current minimal value and minimal value of vectors
        for (int k = 0; k < 4; k++) {
            minimalDistances[k%2] = min(minimalDistances[k%2], _mm_cvtss_f32(distance[k]));
        }
    }
    //prüft, ob der neue Punkt näher an der Zielform ist als zuvor
    return minimalDistances[0] < minimalDistances[1];
}