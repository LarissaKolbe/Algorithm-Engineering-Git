
#include <math.h>
#include <iostream>
#include <immintrin.h>
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
 * @param p1 Koordinaten zu Punkt 1
 * @param p2 Koordinaten zu Punkt 1
 * @return Distanz
 */
float getDistance(Coordinates p1, Coordinates p2){
    auto distX = (float)pow(p1.x - p2.x, 2);
    auto distY = (float)pow(p1.y - p2.y, 2);
    return sqrt(distX + distY);
}

float getMinimalValues128(__m128 v) {
    float result[4];
    _mm_storeu_ps(result, v);
    float min_a = result[0];
    for (int i = 1; i < 4; i++) {
        min_a = min(min_a, result[i]);
    }
    return min_a;
}

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

    int size = (int)targetShape.size();

    //bestimmt die minimale Distanz der beiden Punkte zu einem Punkt der Zielform
#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
    for (int j = 1; j < size; j++) {
//    __unroll(4) for (int j = 1; j < targetShape.size(); j+=4) {
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

//        if (minValues[0] != _mm_cvtss_f32(currentMinDistMod) || minValues[1] != _mm_cvtss_f32(currentMinDistPrev)){
//            cout << "  !!!!!! VI4 -- WRONG MIN VALUE !!!!!!" << endl;
//        }
//        if (isnan(_mm_cvtss_f32(currentMinDistMod)) || isnan(currentMinDistMod[0])){
//            cout << "  !!!!!! VI4 -- HAS NAN VALUE !!!!!!" << endl;
//        }
        //sets minimal distances to minimum of current minimal value and minimal value of vectors
        minDistNew = min(minDistNew, _mm_cvtss_f32(currentMinDistMod));
        minDistPrev = min(minDistPrev, _mm_cvtss_f32(currentMinDistPrev));

    }
//    cout << "VI4:   " << minDistNew << " | " << minDistPrev << endl;
    //prüft, ob der neue Punkt näher an der Zielform ist als zuvor
    return minDistNew < minDistPrev;
}


float getMinimalValues(__m256 v) {
    float result[8];
    _mm256_storeu_ps(result, v);
    float min_a = result[0];
    for (int i = 1; i < 8; i++) {
        min_a = min(min_a, result[i]);
    }
    return min_a;
}


bool isBetterFit_VIUnroll8(Coordinates pointNew, Coordinates pointPrev, Coordinates *__restrict__ targetShape, int size){
    //erstellt Vektor, der die Koordinaten der beiden Punkte enthält
    __m128 dataVec = _mm_setr_ps(pointNew.x, pointNew.y, pointPrev.x, pointPrev.y);

    //berechnet für beide Punkte die Distanz zum ersten Punkt der Zielform
    float minDistNew = getDistance(pointNew, targetShape[0]);
    float minDistPrev = getDistance(pointPrev, targetShape[0]);
    vector<float> minimalDistances = {minDistNew, minDistPrev};

    int unrollNumber = 8;

    //bestimmt die minimale Distanz der beiden Punkte zu einem Punkt der Zielform
//#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
//    for (int j = 1; j < numberPointsTarget; j++) {
//    __unroll(8) for (int j = 1; j < size; j+=unrollNumber) {
//        minimalDistances = getMinDistance(targetShape, dataVec, minimalDistances, unrollNumber, size);
//    }

//#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
    __unroll(8) for (int j = 1; j < size; j+=unrollNumber) {
        __m128 calcTmp[8] = {};
#pragma omp simd aligned(targetShape : 64)
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_setr_ps(targetShape[j + k].x, targetShape[j + k].y, targetShape[j + k].x,
                                     targetShape[j + k].y);
        }
#pragma omp simd
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_sub_ps(dataVec, calcTmp[k]);
        }
#pragma omp simd
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_mul_ps(calcTmp[k], calcTmp[k]);
        }
#pragma omp simd
        for (int k = 0; k < unrollNumber; k++) {
            calcTmp[k] = _mm_hadd_ps(calcTmp[k], calcTmp[k]);
        }
        __m128 distances[4] = {};
        int index = 0;
        //grader index -> neuer Punkt, ungrader Index -> vorheriger Punkt
        for (int k = 0; k < unrollNumber; k += 4) {
            distances[index] = _mm_setr_ps(calcTmp[k][0], calcTmp[k + 1][0], calcTmp[k + 2][0], calcTmp[k + 3][0]);
            distances[index + 1] = _mm_setr_ps(calcTmp[k][1], calcTmp[k + 1][1], calcTmp[k + 2][1], calcTmp[k + 3][1]);
            index += 2;
        }
#pragma omp simd
        for (int k = 0; k < 4; k++) {
            distances[k] = _mm_sqrt_ps(distances[k]);
//        for (auto & dist : distances) {
//            dist = _mm_sqrt_ps(dist);
        }
        //get the smallest value in distsMod and distsPrev
//        float minValues[4] = {};
#pragma omp simd
        for (int k = 0; k < 4; k++) {
            distances[k] = _mm_min_ps(distances[k], _mm_movehl_ps(distances[k], distances[k]));
//        for (auto & dist : distances) {
//            minValues[dist] = getMinimalValues128(distances[dist]);
//            dist = _mm_min_ps(dist, _mm_movehl_ps(dist, dist));
        }
#pragma omp simd
        for (int k = 0; k < 4; k++) {
            distances[k] = _mm_min_ps(distances[k], _mm_shuffle_ps(distances[k], distances[k], 1));
//        for (auto & dist : distances) {
//            dist = _mm_min_ps(dist, _mm_shuffle_ps(dist, dist, 1));
        }
        //sets minimal distances to minimum of current minimal value and minimal value of vectors
        for (int k = 0; k < 4; k++) {
//            if(k%2 == 2){
//                minDistNew = min(minDistNew, _mm_cvtss_f32(distances[k]));
//            } else {
//                minDistPrev = min(minDistPrev, _mm_cvtss_f32(distances[k]));
//            }
            //        if (minValues[k] != _mm_cvtss_f32(distances[k])){
            //            cout << "  !!!!!! VI8 -- WRONG MIN VALUE !!!!!!" << endl;
            //        }
            //        if (isnan(_mm_cvtss_f32(distances[k])) || isnan(distances[k][0])){
            //            cout << "  !!!!!! VI8 -- HAS NAN VALUE !!!!!!" << endl;
            //        }
            minimalDistances[k % 2] = min(minimalDistances[k % 2], _mm_cvtss_f32(distances[k]));
        }
//        minDistNew = minimalDistances[0];
//        minDistPrev = minimalDistances[1];
    }
//    cout << "VI8:   " << minimalDistances[0] << " | " << minimalDistances[1] << endl;
    //prüft, ob der neue Punkt näher an der Zielform ist als zuvor
//    return minDistNew < minDistPrev;
    return minimalDistances[0] < minimalDistances[1];
}

bool isBetterFit_VIUnroll256(Coordinates pointNew, Coordinates pointPrev, Coordinates *__restrict__ targetShape, int size){
    //erstellt Vektor, der die Koordinaten der beiden Punkte enthält
    __m256 dataVec = _mm256_setr_ps(pointNew.x, pointNew.y, pointNew.x, pointNew.y, pointPrev.x, pointPrev.y, pointPrev.x, pointPrev.y);

    //berechnet für beide Punkte die Distanz zum ersten Punkt der Zielform
    float minDistNew = getDistance(pointNew, targetShape[0]);
    float minDistPrev = getDistance(pointPrev, targetShape[0]);
    vector<float> minimalDistances = {minDistNew, minDistPrev};

    //bestimmt die minimale Distanz der beiden Punkte zu einem Punkt der Zielform
//#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
//    for (int j = 1; j < numberPointsTarget; j++) {
//    __unroll(8) for (int j = 1; j < size; j+=unrollNumber*2) {
//        minimalDistances = getMinDistance256(targetShape, dataVec, minimalDistances, size);
//    }


//#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
    __unroll(8) for (int j = 1; j < size; j+=8*2) {
        __m256 calcTmp[8] = {};
#pragma omp simd aligned(targetShape : 64)
        for (int k = 0; k < 8 * 2; k += 2) {
            calcTmp[k / 2] = _mm256_setr_ps(targetShape[j + k].x, targetShape[j + k].y, targetShape[j + k + 1].x,
                                            targetShape[j + k + 1].y,
                                            targetShape[j + k].x, targetShape[j + k].y, targetShape[j + k + 1].x,
                                            targetShape[j + k + 1].y);
        }
#pragma omp simd
        for (int k = 0; k < 8; k ++) {
            calcTmp[k] = _mm256_sub_ps(dataVec, calcTmp[k]);
//        for (auto & tmp : calcTmp) {
//            tmp = _mm256_sub_ps(dataVec, tmp);
        }
#pragma omp simd
        for (int k = 0; k < 8; k ++) {
            calcTmp[k] = _mm256_mul_ps(calcTmp[k], calcTmp[k]);
//        for (auto & tmp : calcTmp) {
//            tmp = _mm256_mul_ps(tmp, tmp);
        }
#pragma omp simd
        for (int k = 0; k < 8; k ++) {
            calcTmp[k] = _mm256_hadd_ps(calcTmp[k], calcTmp[k]);
//        for (auto & tmp : calcTmp) {
//            tmp = _mm256_hadd_ps(tmp, tmp);
        }
        __m256 distances[4] = {};
        int index = 0;
        //grader index -> neuer Punkt, ungrader Index -> vorheriger Punkt
        for (int k = 0; k < 8; k += 4) {
            distances[index] = _mm256_setr_ps(calcTmp[k][0], calcTmp[k + 1][0], calcTmp[k + 2][0], calcTmp[k + 3][0],
                                              calcTmp[k][1], calcTmp[k + 1][1], calcTmp[k + 2][1], calcTmp[k + 3][1]);
            distances[index + 1] = _mm256_setr_ps(calcTmp[k][4], calcTmp[k + 1][4], calcTmp[k + 2][4], calcTmp[k + 3][4],
                                                  calcTmp[k][5], calcTmp[k + 1][5], calcTmp[k + 2][5],
                                                  calcTmp[k + 3][5]);
            index += 2;
        }
#pragma omp simd
        for (int k = 0; k < 4; k++) {
            distances[k] = _mm256_sqrt_ps(distances[k]);
//        for (auto & dist : distances) {
//            dist = _mm256_sqrt_ps(dist);
        }

        float minValues[4] = {};
        //get the smallest value in distsMod and distsPrev
#pragma omp simd
        for (int k = 0; k < 4; k++) {
            minValues[k] = getMinimalValues(distances[k]);
            //        if (isnan(_mm256_cvtss_f32(distances[k])) || isnan(distances[k][0])){
            //            cout << "  !!!!!! 256 -- HAS NAN VALUE !!!!!!" << endl;
            //        }
        }

        //sets minimal distances to minimum of current minimal value and minimal value of vectors
        for (int k = 0; k < 4; k++) {
//            if(k%2 == 2){
//                minDistNew = min(minDistNew, minValues[k]);
//            } else {
//                minDistPrev = min(minDistPrev, minValues[k]);
//            }
            minimalDistances[k % 2] = min(minimalDistances[k % 2], minValues[k]);
        }
//        minDistNew = minimalDistances[0];
//        minDistPrev = minimalDistances[1];
    }
//    cout << "256:   " << minimalDistances[0] << " | " << minimalDistances[1] << endl;
    //prüft, ob der neue Punkt näher an der Zielform ist als zuvor
//    return minDistNew < minDistPrev;
    return minimalDistances[0] < minimalDistances[1];
}
