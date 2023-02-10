#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
#include <omp.h>
#include <cmath>
#include <random>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <immintrin.h>
#include "datatypes.h"
#include "exportHelper.h"
#include "importHelper.h"
#include "fitnessHelper.h"
#include "helperFunctions.h"
#include "configurationHelpers.h"
#include "aligned_allocator.h"

template<class T>
using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

using namespace std;

//TODO: kucken ob ich das hier überhaupt benutze
#if defined(_WIN32) // restrict pointers on Windows
    #if defined(_MSC_VER) || defined(__ICL)
        #define __restrict__ __restrict
    #endif
#endif


//TODO: Bei allen Funktionen ect Documentation dazu schreiben und Kommentare zum Code machen!
// auch in den Helper und Header Dateien!!

//TODO: Alle Anweisungen etc umformulieren (auf eine Sprache einigen!)

//TODO: überall statt "vector<Coordinates> vec" als Parameter zu nehmen "Coordinates *__restrict__ vec" schreiben
// und statt "vec" als ganzes zu übergeben nur "vec-data()" übergeben
// gibt dann deutlich weniger overhead und mit dem "__restrict__" kann compiler evtl mehr Optimierungen machen!


int outputHeight, outputWidth, inputSize, numberPointsTarget;

float timeDistance = 0.0, timeCalc = 0.0, timeExport = 0.0, timeMRP = 0.0, timePertub = 0.0;
int callDist = 0, callCalc = 0, callExp = 0, callMRP = 0, callPertub = 0;


/**
 * Configurations used for run.
 * Have standard values but can be changed by user.
 */
Configurations conf;


/**
 * Sets width of output to the greater one of the passed values.
 * @param widthInit    Width of the input
 * @param widthTarget  Width of the Target
 */
void setOutputWidth(int widthInit, int widthTarget){
    if (widthInit > widthTarget){
        outputWidth = widthInit;
    } else {
        outputWidth = widthTarget;
    }
}

/**
 * Sets height of output to the greater one of the passed values.
 * @param heightInit   Height of the input
 * @param heightTarget Height of the Target
 */
void setOutputHeight(int heightInit, int heightTarget){
    if (heightInit > heightTarget){
        outputHeight = heightInit;
    } else {
        outputHeight = heightTarget;
    }
}


double timeVIUnroll4 = 0.0, timeVIUnroll8 = 0.0;
int callFit = 0;

bool isBetterFit(Coordinates pointNew, Coordinates pointPrev, aligned_vector<Coordinates> targetShape) {
    auto start = omp_get_wtime();
    bool viUnroll4 = isBetterFit_VIUnroll4(pointNew, pointPrev, targetShape);
    timeVIUnroll4 += omp_get_wtime() - start;

    start = omp_get_wtime();
    bool viUnroll8 = isBetterFit_VIUnroll8(pointNew, pointPrev, targetShape.data(), targetShape.size());
    timeVIUnroll8 += omp_get_wtime() - start;

    callFit++;
    return viUnroll4;
}
//  ../data/shapes/exportedShapes/shape4_size3002.ppm

/**
 * Berechnet die statistischen Eigenschaften des übergebenen Datensatzes.
 * @param dataset
 * @return
 */
statisticalProperties calculateStatisticalProperties(aligned_vector<Coordinates> dataset){
    auto start = omp_get_wtime();
    //TODO: berechnung noch effizienter/performanter machen wenn möglich
    //macht es nen Unterschied, ob ich die schleifen auf bspw properties.meanX oder auf meanX mache?
    statisticalProperties properties = {0.0, 0.0, 0.0, 0.0};
    double meanX = 0.0, meanY = 0.0, varianceX = 0.0, varianceY = 0.0;
    for(int i=0; i<inputSize; i++){
        properties.meanX += dataset[i].x;
        properties.meanY += dataset[i].y;
    }
    properties.meanX = roundValue(properties.meanX / inputSize, conf.decimals);
    properties.meanY = roundValue(properties.meanY / inputSize, conf.decimals);

    for(int i=0; i<inputSize; i++){
        properties.varianceX += pow(dataset[i].x - properties.meanX, 2);
        properties.varianceY += pow(dataset[i].y - properties.meanY, 2);
    }
    properties.varianceX = properties.varianceX / (inputSize-1);
    properties.varianceY = properties.varianceY / (inputSize-1);
    properties.stdDeviationX = roundValue(sqrt(properties.varianceX), conf.decimals);
    properties.stdDeviationY = roundValue(sqrt(properties.varianceY), conf.decimals);
    properties.varianceX = roundValue(properties.varianceX, conf.decimals);
    properties.varianceY = roundValue(properties.varianceY, conf.decimals);

    timeCalc += omp_get_wtime() - start;
    callCalc++;
    return properties;
}

/**
 * Prüft, ob die statistischen Eigenschaften der übergebenen Punktemengen mit den übergebenen EIgenschaften übereinstimmen.
 * @param newDS veränderte Punktemengen
 * @param initialProps statistischen Eigenschaften der ursprüngliche Punktemengen
 * @param allowedDifference The distance the statistical properties are allowed to have
 * @return boolean
 */
 //TODO: allowed difference in Abhängikeit von Accurancy machen?
bool isErrorOk(aligned_vector<Coordinates> newDS, statisticalProperties initialProps) {
    statisticalProperties newProps = calculateStatisticalProperties(newDS);
    bool meanX = abs(newProps.meanX - initialProps.meanX) <= conf.accuracy;
    bool meanY = abs(newProps.meanY - initialProps.meanY) <= conf.accuracy;
    bool stdDeviationX = abs(newProps.stdDeviationX - initialProps.stdDeviationX) <= conf.accuracy;
    bool stdDeviationY = abs(newProps.stdDeviationY - initialProps.stdDeviationY) <= conf.accuracy;

    return meanX && meanY && stdDeviationX && stdDeviationY;

     return abs(newProps.meanX - initialProps.meanX) <= conf.accuracy
        && abs(newProps.meanY - initialProps.meanY) <= conf.accuracy
//        && abs(newProps.varianceX - initialProps.varianceX) <= conf.accuracy
//        && abs(newProps.varianceY - initialProps.varianceY) <= conf.accuracy;
        && abs(newProps.stdDeviationX - initialProps.stdDeviationX) <= conf.accuracy
        && abs(newProps.stdDeviationY - initialProps.stdDeviationY) <= conf.accuracy;
}

/**
 * Bewegt zufälligen Punkt im übergebenen Datensatz um maximal maxMovement viele Stellen.
 * Will change the passed vector.
 *
 * @param dataset     Dataset in which points should be moved
 * @param size        Size of the dataset
 * @param maxMovement Maximale Bewegung die ein Punkt machen kann
 * @return
 */
Coordinates moveRandomPoint(aligned_vector<Coordinates> dataset, int indexToMove, mt19937 gen) {
    uniform_real_distribution<> disForMove(-conf.maxMovement, conf.maxMovement);  // define the range of the float numbers
    Coordinates newCoordinates;

    do {
        float moveX = disForMove(gen);
        float moveY = disForMove(gen);
        newCoordinates = {
                dataset[indexToMove].x + moveX,
                dataset[indexToMove].y + moveY,
        };
    // akzeptiert Bewegung nur, wenn sie noch innerhalb der Bildgrenzen ist, andernfalls wird neu berechnet
    } while(round(newCoordinates.x) >= outputWidth || round(newCoordinates.y) >= outputHeight
        || newCoordinates.x < 0 || newCoordinates.y < 0);
    return newCoordinates;
}

aligned_vector<Coordinates> perturb(aligned_vector<Coordinates> dsBefore, aligned_vector<Coordinates> targetShape, double temp){
    //kopiert Datensatz
    aligned_vector<Coordinates> dsNew(dsBefore);

    //initialisiert Zufallszahlengenerator
    random_device rd;
    mt19937 gen(rd());

    //bewegt zufällige Punkte bis if-Bedingung erfüllt wird
    while(true){
        //bestimmt, welcher Punkt verschoben wird
        uniform_real_distribution<> disForIndex(0, inputSize);
        int indexToMove = disForIndex(gen);

        //bewegt zufälligen Punkt in dsNew
        auto startMRP = omp_get_wtime();
        dsNew[indexToMove] = moveRandomPoint(dsNew, indexToMove, gen);

        timeMRP += omp_get_wtime() - startMRP;
        callMRP++;

        //erstellt Zufallszahl zwischen 0 und 1
        uniform_real_distribution<> dis(0, 1);
        double randomNr = dis(gen);

        //akzeptiert dsNew, wenn es näher an der Zielform ist als vorher oder
        // wenn die Zufallszahl kleiner als temp ist
        if(isBetterFit(dsNew[indexToMove], dsBefore[indexToMove], targetShape) || randomNr < temp){
            return dsNew;
        }
    }
}

/**
 * Wandelt die Punkte aus dem Eingabearray um.
 * Der Ausgabearray erfüllt die gleichen statistischen Eigenschaften.
 * @param initialDS Eingabearray (Punktewolke)
 * @param currentDS Kopie des Eingabearrays. Wird innerhalb der Funktion geändert.
 * @param size Größe des Eingabearray
 * @return Ausgabearray der gleichen statistischen Eigenschaften erfüllt
 */
aligned_vector<Coordinates> generateNewPlot(aligned_vector<Coordinates> initialDS, aligned_vector<Coordinates> targetShape, statisticalProperties initialProperties) {
    aligned_vector<Coordinates> currentDS(initialDS);
//#pragma omp for
    for (int i = 0; i<conf.iterations; i++){
        //TODO: am Ende wieder rausnehmen
        if(i % 10000 == 0) {
            auto start = omp_get_wtime();
            cout << endl << "Iteration: " << i  << endl;
            aligned_vector<Coordinates> dataset(currentDS);
            string fileNameTest = "../data/output/output" + to_string(i) + ".ppm";
//#pragma omp critical
            exportImage(fileNameTest, dataset, 255, outputHeight, outputWidth);
            timeExport += omp_get_wtime() - start;
            callExp++;
        }

        //"temperature" starts with 0.4 and goes down until 0 with each iteration
        double temp = (conf.maxTemp - conf.minTemp) * ((conf.iterations-i)/conf.iterations) + conf.minTemp;

        auto startPerturb = omp_get_wtime();

        aligned_vector<Coordinates> testDS = perturb(currentDS, targetShape, temp);

        timePertub += omp_get_wtime() - startPerturb;
        callPertub++;

        if (isErrorOk(testDS, initialProperties)){
//#pragma omp critical
            currentDS = testDS;
        }
    }
    return currentDS;
}



int main() {
    //TODO: nur zum testen ob openMP ordentlich läuft...
    cout << "The following should print 4 times: " << endl;
#pragma omp parallel num_threads(4)// compiler directive
    {
        // omp_get_thread_num() is an OpenMP library routine
        auto thread_id = omp_get_thread_num();
        cout << "Hello from thread " << thread_id << endl;
    }

    //Liest Eingabe- und Zieldatei ein
    // user hat die Wahl selbst etwas hochzuladen oder eine der bereitgestellten Dateien zu verwenden
    FileInformation inputInfo = readTargetData(initialData);
    FileInformation targetInfo = readTargetData(target);

    //Liest Outputpfad ein
    string fileNameExport = readExportPath();

    //gives user opportunity to change configurations
    conf = setConfigurations(conf);

    //TODO: zum zeitmessen; kommt ganz am Ende raus
    auto startProgram = omp_get_wtime();

    //Reads data from input and target images
    aligned_vector<Coordinates> inputVector = createVectorFromImage(inputInfo);
    aligned_vector<Coordinates> targetShape = createVectorFromImage(targetInfo);

    //sets global Parameters
    inputSize = inputVector.size();
    numberPointsTarget = targetShape.size();
    setOutputWidth(inputInfo.width, targetInfo.width);
    setOutputHeight(inputInfo.height, targetInfo.height);
    // Release memory from heap after using it
    delete[] inputInfo.imageData, delete[] targetInfo.imageData;

    //calculates statistical properties of input data
    statisticalProperties initialProperties = calculateStatisticalProperties(inputVector);

    //generates new plot with same statistical properties but different point alignment
    aligned_vector<Coordinates> result = generateNewPlot(inputVector, targetShape, initialProperties);

    // Create export image file
    auto startExp = omp_get_wtime();
    exportImage(fileNameExport, result, inputInfo.maxColor, outputHeight, outputWidth);
    timeExport += omp_get_wtime() - startExp;
    callExp++;

    //TODO: Am Ende rausnehmen
    float runtime = omp_get_wtime() - startProgram;
    cout << endl << "Dauern:" << endl
         << "-- Ges:           " << runtime << " sek ;  " << runtime / 60 << "min" << endl;
    cout << "-- MRP Ges:       " << timeMRP << " sek ;  " << timeMRP / 60 << "min" << endl;
    cout << "--   MRP/run:     " << timeMRP / callMRP<< " sek ;  " << (timeMRP / callMRP) / 60 << "min" << endl;
    cout << "-- Export Ges:    " << timeExport << " sek ;  " << timeExport / 60 << "min" << endl;
    cout << "--   Export/run:  " << timeExport / callExp<< " sek ;  " << (timeExport / callExp) / 60 << "min" << endl;
    cout << "-- Calc Ges:      " << timeCalc << " sek ;  " << timeCalc / 60 << "min" << endl;
    cout << "--   Calc/run:    " << timeCalc / callCalc<< " sek ;  " << (timeCalc / callCalc) / 60 << "min" << endl;
    cout << "-- Perturb Ges:   " << timePertub << " sek ;  " << timePertub / 60 << "min" << endl;
    cout << "--   Perturb/run: " << timePertub / callPertub<< " sek ;  " << (timePertub / callPertub) / 60 << "min" << endl;
    cout << "-- Dist Ges:      " << timeDistance << " sek ;  " << timeDistance / 60 << "min" << endl;
    cout << "--   Dist/run:    " << timeDistance / callDist<< " sek ;  " << (timeDistance / callDist) / 60 << "min" << endl;
    cout << "--   Dist Runs:   " << callDist << endl;

    cout << "-- VI_Unroll4 Ges: " << timeVIUnroll4 << " sek ;  " << timeVIUnroll4 / 60 << "min" << endl;
    cout << "--   Dist/run:    " << timeVIUnroll4 / callFit<< " sek ;  " << (timeVIUnroll4 / callFit) / 60 << "min" << endl;
    cout << "-- VI_Unroll8 Ges: " << timeVIUnroll8 << " sek ;  " << timeVIUnroll8 / 60 << "min" << endl;
    cout << "--   Dist/run:    " << timeVIUnroll8 / callFit<< " sek ;  " << (timeVIUnroll8 / callFit) / 60 << "min" << endl;
    cout << "-- Runs:   " << callFit << endl;

    return 0;
}

#pragma clang diagnostic pop