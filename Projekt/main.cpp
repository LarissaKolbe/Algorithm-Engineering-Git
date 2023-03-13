
#include <cmath>
#include <omp.h>
#include <random>
#include <string>
#include <iostream>
#include <algorithm>
#include "datatypes.h"
#include "helperFunctions.h"
#include "exportHelper.h"
#include "importHelper.h"
#include "fitnessHelper.h"
#include "configurationHelpers.h"
#include "PerfEvent.hpp"

using namespace std;


#if defined(_WIN32) // restrict pointers on Windows
    #if defined(_MSC_VER) || defined(__ICL)
        #define __restrict__ __restrict
    #endif
#endif


double timeCalc = 0.0, timeExport = 0.0, timeMRP = 0.0, timeIteration = 0.0;
int callCalc = 0, callExp = 0, callMRP = 0, callPertub = 0;

double timeNat=0.0, timePar = 0.0, timeVIUnroll8 = 0.0, timeVIUnroll256 = 0.0, timeVIUnroll256For=0.0, timeVIUnroll8For=0.0;
int callFit = 0, parDiff=0, vi8Diff=0, vi256Diff=0;


bool isBetterFit(const Coordinate pointNew, const Coordinate pointPrev, const Coordinate*__restrict__ targetShape, const int targetSize) {
    bool fit;
    auto startFit = omp_get_wtime();
//    BenchmarkParameters paramsFit;
//    {
//        paramsFit.setParam("_name", "Nat"); // set parameter
//        PerfEventBlock e((int) targetShape.size(), paramsFit, true);
    fit = isBetterFit_Naive(pointNew, pointPrev, targetShape, targetSize);
//    }
//    {
//        paramsFit.setParam("_name", "Par"); // set parameter
//        PerfEventBlock e((int) targetShape.size(), paramsFit, true);
//    fit = isBetterFit_Parallel(pointNew, pointPrev, targetShape, targetSize);
//    }
//    {
//        paramsFit.setParam("_name", "VI8"); // set parameter
//        int size = (int) targetShape.size();
//        size = (size - 8) / 8;
//        size = size + (size % 8);
//        PerfEventBlock e((int) targetShape.size(), paramsFit, true);
//    fit = isBetterFit_VI128(pointNew, pointPrev, targetShape, targetSize);
//    }
//    {
//        paramsFit.setParam("_name", "256"); // set parameter
//        int size = (int) targetShape.size();
//        size = (size - 16) / 16;
//        size = size + (size % 16);
//        PerfEventBlock e((int) targetShape.size(), paramsFit, true);
//    fit = isBetterFit_VI256(pointNew, pointPrev, targetShape, targetSize);
//    }
//    fit = isBetterFit_VI128_Par(pointNew, pointPrev, targetShape, targetSize);

//    fit = isBetterFit_VI256_Par(pointNew, pointPrev, targetShape, targetSize);

//    fit = isBetterFit_VI128_NoUnroll(pointNew, pointPrev, targetShape, targetSize);

//    fit = isBetterFit_VI256_NoUnroll(pointNew, pointPrev, targetShape, targetSize);

//    fit = isBetterFit_VI128_NoUnroll_Par(pointNew, pointPrev, targetShape, targetSize);

//    fit = isBetterFit_VI256_NoUnroll_Par(pointNew, pointPrev, targetShape, targetSize);

    double time = omp_get_wtime() - startFit;
#pragma omp critical
    {
        timeNat += time;
        callFit++;
    }
    return fit;
}

/**
 * Berechnet die statistischen Eigenschaften des übergebenen Datensatzes
 * @param dataset Datensatz
 * @param size Größe des Datensatzes
 * @param decimals Bestimmt, wie viele Nachkommestellen betrachtet werden sollen
 * @return statistische Eigenschaften des übergebenen Datensatzes
 */
statisticalProperties calculateStatisticalProperties(const Coordinate *__restrict__ dataset, const int size, const int decimals){
    auto startCalc = omp_get_wtime();
    float meanX = 0.0, meanY = 0.0, stdDeviationX = 0.0, stdDeviationY = 0.0;

    //Berechnet die Durchschnittswerte der einzelnen Koordinaten
#pragma omp simd aligned(dataset : 64) reduction(+: meanX, meanY)
    for(int i=0; i<size; i++){
        meanX += dataset[i].x;
        meanY += dataset[i].y;
    }

    //Rundet die Durchschnittswerte auf decimals viele Nachkommastellen
    meanX = roundToNDecimals(meanX / (float) size, decimals);
    meanY = roundToNDecimals(meanY / (float) size, decimals);

    //Berechnet die Standardabweichung der einzelnen Koordinatenwerte
#pragma omp simd aligned(dataset : 64) reduction(+: stdDeviationX, stdDeviationY)
    for(int i=0; i<size; i++){
        stdDeviationX += (float)pow(dataset[i].x - meanX, 2);
        stdDeviationY += (float)pow(dataset[i].y - meanY, 2);
    }
    stdDeviationX = sqrt(stdDeviationX / (float)(size-1));
    stdDeviationY = sqrt(stdDeviationY / (float)(size-1));

    //Rundet die Standardabweichung auf decimals viele Nachkommastellen
    stdDeviationX = roundToNDecimals(stdDeviationX, decimals);
    stdDeviationY = roundToNDecimals(stdDeviationY, decimals);

    double time = omp_get_wtime() - startCalc;
#pragma omp critical
    {
        timeCalc += time;
        callCalc++;
    }
    return {meanX, meanY, stdDeviationX, stdDeviationY};
}

/**
 * Prüft, ob die statistischen Eigenschaften des übergebenen Datensatzes mit den übergebenen Eigenschaften übereinstimmen.
 * @param currentDS veränderter Datensatze
 * @param initialProps statistischen Eigenschaften des ursprünglichen Datensatzes
 * @param size     Größe des Datensatzes
 * @param accuracy Bestimmt, wie sehr die Werte von einander abweichen dürfen
 * @param decimals Bestimmt, wie viele Nachkommestellen betrachtet werden sollen
 * @return True, wenn die statistischen Eigenschaften übereinstimmen, false sonst
 */
bool isErrorOk(const Coordinate *__restrict__ currentDS, const statisticalProperties& initialProps, const int size, const double accuracy, const int decimals) {
    //berechnet statistischen Eigenschaften des veränderten Datensatzes
//    statisticalProperties currentProps;
//    BenchmarkParameters paramsStat;
//    {
//        paramsStat.setParam("_name", "calcStats");
//        PerfEventBlock e(size, paramsStat, true);
    const statisticalProperties currentProps = calculateStatisticalProperties(currentDS, size, decimals);
//    }

    //vergleicht die Eigenschaften des ursprünglichen und des veränderten Datensatzes
    return abs(currentProps.meanX - initialProps.meanX) <= accuracy
        && abs(currentProps.meanY - initialProps.meanY) <= accuracy
        && abs(currentProps.stdDeviationX - initialProps.stdDeviationX) <= accuracy
        && abs(currentProps.stdDeviationY - initialProps.stdDeviationY) <= accuracy;
}


/**
 * Bewegt den übergebenen Punkt um einen Wert von maximal maxMovement.
 * @param pointToMove Punkt, der bewegt werden soll
 * @param maxX        Maximal erlaubte x-Koordinate
 * @param maxY        Maximal erlaubte y-Koordinate
 * @param maxMovement Maximale Entfernung, um die sich ein Punkt bewegen kann
 * @param gen         Zufallszahlenseed
 * @return Koordinaten des bewegten Punktes
 */
Coordinate moveRandomPoint(const Coordinate pointToMove, const int maxX, const int maxY, const float maxMovement, mt19937 gen) {
    //initialisiert Zufallszahlengenerator für Bewegung
    uniform_real_distribution<float> distMove(-maxMovement,maxMovement);

    Coordinate newCoordinate;
    // Bewegt den Punkt um zufällige Werte
    // akzeptiert Bewegung nur, wenn sie nach runden noch innerhalb der Bildgrenzen ist
    //  andernfalls wird er erneut verschoben
    do {
        newCoordinate = {
                pointToMove.x + distMove(gen),
                pointToMove.y + distMove(gen),
        };
    } while(round(newCoordinate.x) >= maxX || round(newCoordinate.y) >= maxY
            || newCoordinate.x < 0 || newCoordinate.y < 0);
    return newCoordinate;
}

/**
 * Bewegt den übergebenen Punkt in eine zufällige Richtung und prüft, ob er der Zielform dadurch näher kam.
 * Wenn ja, werden die neuen koordinaten zurückgegeben, wenn nicht wird er nochmal bewegt.
 * @param pointToMove Punkt, der bewegt werden soll
 * @param targetShape Koordinaten der Form, die erreicht werden soll
 * @param maxX Maximal erlaubte x-Koordinate
 * @param maxY Maximal erlaubte y-Koordinate
 * @param temp aktuelle temperature
 * @param maxMovement Maximale Entfernung, um die sich ein Punkt bewegen kann
 * @param gen Zufallszahlenseed
 * @return neue Koordinaten des bewegten Punktes
 */
Coordinate perturb(const Coordinate pointToMove, const Coordinate*__restrict__ targetShape, const int targetSize, const int maxX, const int maxY, const double temp, const float maxMovement, mt19937 gen){
    //initialisiert Zufallszahlengenerator zwischen 0 und 1
    uniform_real_distribution<double> distTemp(0, 1);

    //bewegt den übergebenen Punkt bis if-Bedingung erfüllt wird
    while(true){
        //bewegt Punkt
        auto startMRP = omp_get_wtime();
        const Coordinate modifiedPoint = moveRandomPoint(pointToMove, maxX, maxY, maxMovement, gen);

        double time = omp_get_wtime() - startMRP;
#pragma omp critical
        {
            timeMRP += time;
            callMRP++;
        }

        //akzeptiert bewegten Punkt, wenn er näher an der Zielform ist als vorher oder
        // wenn die Zufallszahl kleiner als temp ist
        if(isBetterFit(modifiedPoint, pointToMove, targetShape, targetSize) || distTemp(gen) < temp){
            return modifiedPoint;
        }
    }
}

int errorOk = 0;

/**
 * Bewegt die Punkte des Inputdatensatz in Richtung der Zielform.
 * Der Ausgabedatensatz erfüllt die gleichen statistischen Eigenschaften.
 * @param currentDS Koordinaten des Inputdatensatz
 * @param targetShape Koordinaten der Zielform
 * @param initialProperties statistischen Eigenschaften des Inputdatensatz
 * @param conf Die Konfigurationen, mit deneen das Programm laufen soll
 * @param maxX Maximal erlaubte x-Koordinate
 * @param maxY Maximal erlaubte y-Koordinate
 * @return Datensatz, der der Zielform nahe ist, aber die gleichen statistischen
 *   Eigenschaften wie der Inputdatensatz hat
 */
aligned_vector<Coordinate> generateNewPlot(aligned_vector<Coordinate> currentDS, const aligned_vector<Coordinate> targetShape, const statisticalProperties initialProperties, const Configurations conf, const int maxX, const int maxY) {
    //speichert Konfigurationen in Konstanten, um nicht immer das ganze Objekt laden zu müssen
    const double maxTemp = conf.maxTemp, minTemp = conf.minTemp, accuracy = conf.accuracy;
    const int iterations = conf.iterations, decimals = conf.decimals, dataSize = (int)currentDS.size(), targetSize = (int)targetShape.size();
    const float maxMovement = conf.maxMovement;

    //initialisiert Zufallszahlengenerator
    random_device rd;
    mt19937 gen(rd());
    //initialisiert Zufallszahlengenerator um zufälligen Punkt im Inputdatensatz auszuwählen
    uniform_real_distribution<> distIndex(0, dataSize);

#pragma omp parallel for schedule(dynamic, 8)
    for (int i = 0; i < iterations; i++) {
        auto startIteration = omp_get_wtime();

        //TODO: am Ende wieder rausnehmen
//        if (i % 25000 == 0) {
////            auto start = omp_get_wtime();
//            cout << endl << "Iteration: " << i << endl;
//            string fileNameTest = "../data/output/output" + to_string(i) + ".ppm";
//            exportImage(fileNameTest, currentDS, 255, maxY, maxX);
////            timeExport += omp_get_wtime() - start;
////            callExp++;
//        }

        //berechnet aktuelle "temperature"
        // beginnt mit maxTemp und nähert sich mit jeder Iteration minTemp
        const double temp = (maxTemp - minTemp) * ((double) (iterations - i) / (double) iterations) + minTemp;

        const int randomIndex = (int) distIndex(gen);
//        BenchmarkParameters paramsPerturb;
//        {
//            paramsPerturb.setParam("_name", "Perturb");
//            PerfEventBlock e(1, paramsPerturb, true);
        //bewegt den übergebenen Punkt in Richtung der Zielform
        const Coordinate movedPoint = perturb(currentDS[randomIndex], targetShape.data(), targetSize, maxX, maxY, temp, maxMovement, gen);
//        }

//#pragma omp critical
//        {
        //TODO: hier kommt es öfter mal zu nem lost update
        // man könnte es vermeiden, wenn man die ciritical section ab hier schon machen würde,
        //  aber dann würde es auch deeeutlich langsamer werden
        aligned_vector<Coordinate> testDS(currentDS);
        testDS[randomIndex] = movedPoint;
        //prüft, ob statistische Eigenschaften noch mit den anfänglichen übereinstimmen
        if (isErrorOk(testDS.data(), initialProperties, dataSize, accuracy, decimals)) {
#pragma omp critical
            {
                currentDS = testDS;
                //TODO: wenn ich hier stattdessen das hier macchen würde:
                //   currentDS[randomIndex] = testDS[randomIndex];
                // dann würde nach wenigen Malen selbst isErrorOk(currentDS) false zurückliefern,
                //  weil das testDS für die if-Bedinigung hier drüber bei jedem Thread unterschiedlich aktuell ist
                errorOk++;
            }
        }
        double time = omp_get_wtime() - startIteration;
#pragma omp critical
        {
            timeIteration += time;
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

    //Userinterface zum Wählen der Input- und Zieldatei.
    // User hat die Wahl selbst Bilder hochzuladen oder eins der bereitgestellten Bilder zu verwenden
    const FileInformation inputInfo = readImageData(initialData);
    const FileInformation targetInfo = readImageData(target);

    //Userinterface zum Wählen des Outputpfades
    const string fileNameExport = readExportPath();

    //Userinterface zum Einzusehen und Verändern der Konfigurationen
    const Configurations conf = setConfigurations();

    //TODO: zum zeitmessen; kommt ganz am Ende raus
    auto startProgram = omp_get_wtime();

    //Erstellt Koordinatenvektoren anhand der Bilddaten
    // Die Vektoren enthalten die Koordinaten aller schwarzen Punkte im Bild
    const aligned_vector<Coordinate> initialDS   = createVectorFromImage(inputInfo.imageData, inputInfo.width, inputInfo.height);
    const aligned_vector<Coordinate> targetShape = createVectorFromImage(targetInfo.imageData, targetInfo.width, targetInfo.height);

    //Gibt den nicht mehr benötigten Speicher vom Heap frei
    delete[] inputInfo.imageData, delete[] targetInfo.imageData;

    //Bestimmt die Größe des Outputbildes
    // Dazu werden Eingabe- und Zielbild verglichen und je die größeren Maße genommen
    const int outputWidth = getGreaterValue(inputInfo.width, targetInfo.width);
    const int outputHeight = getGreaterValue(inputInfo.height, targetInfo.height);

    //Berechnet die statistischen Eigenschaften der Eingabedaten
    const statisticalProperties initialProperties = calculateStatisticalProperties(initialDS.data(), (int)initialDS.size(), conf.decimals);

    cout << endl
        << "Die graphischen Eigenschaften des Datensatzes sind: " << endl
        << "  Durchschnitt der x-Werte:       " << initialProperties.meanX << endl
        << "  Durchschnitt der y-Werte:       " << initialProperties.meanY << endl
        << "  Standardabweichung der x-Werte: " << initialProperties.stdDeviationX << endl
        << "  Standardabweichung der x-Werte: " << initialProperties.stdDeviationY << endl;

    //Bewegt Inputdaten in Richtung der Zielform, ohne dabei die statistischen Eigenschaften zu verändern
    aligned_vector<Coordinate> result; // = generateNewPlot(initialDS, targetShape, initialProperties, conf, outputWidth, outputHeight);

//    BenchmarkParameters params;
//    {
//        params.setParam("_name", "GenNewPlot"); // set parameter
//        int size = conf.iterations;
//        PerfEventBlock e(size, params, true); // start counter
        result = generateNewPlot(initialDS, targetShape, initialProperties, conf, outputWidth, outputHeight);
//    }

    //Exportiert das Ergebnisbild
//    auto startExp = omp_get_wtime();
    exportImage(fileNameExport, result, inputInfo.maxColor, outputHeight, outputWidth);
//    timeExport += omp_get_wtime() - startExp;
//    callExp++;

    //TODO: Am Ende rausnehmen
    double runtime = omp_get_wtime() - startProgram;

    cout << endl << "Dauern:" << endl
         << "-- Ges:           " << runtime << " sek ;  " << runtime / 60 << "min" << endl;
//    cout << "-- Iteration Ges:   " << timeIteration << " sek ;  " << timeIteration / 60 << "min" << endl;
    cout << "--   Iteration/run: " << timeIteration / conf.iterations << " sek ;  " << (timeIteration / conf.iterations) / 60 << "min" << endl;
//    cout << "-- Calc Ges:        " << timeCalc << " sek ;  " << timeCalc / 60 << "min" << endl;
    cout << "--   Calc/run:      " << timeCalc / callCalc<< " sek ;  " << (timeCalc / callCalc) / 60 << "min" << endl;
//    cout << "-- MRP Ges:         " << timeMRP << " sek ;  " << timeMRP / 60 << "min" << endl;
    cout << "--   MRP/run:       " << timeMRP / callMRP<< " sek ;  " << (timeMRP / callMRP) / 60 << "min" << endl;
//    cout << "-- Fit Ges:         " << timeNat << " sek ;  " << timeNat / 60 << "min" << endl;
    cout << "--   Fit/run:      " << timeNat / callFit<< " sek ;  " << (timeNat / callFit) / 60 << "min" << endl;
//    cout << "-- Export Ges:    " << timeExport << " sek ;  " << timeExport / 60 << "min" << endl;
//    cout << "--   Export/run:  " << timeExport / callExp<< " sek ;  " << (timeExport / callExp) / 60 << "min" << endl;
//    cout << endl;
//    cout << "-- Native Ges:     " << timeNat << " sek ;  " << timeNat / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeNat / callFit<< " sek ;  " << (timeNat / callFit) / 60 << "min" << endl;
//    cout << "-- Parallel Ges:   " << timePar << " sek ;  " << timePar / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timePar / callFit<< " sek ;  " << (timePar / callFit) / 60 << "min" << endl;
//    cout << "-- VI_Unroll8 Ges: " << timeVIUnroll8 << " sek ;  " << timeVIUnroll8 / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeVIUnroll8 / callFit<< " sek ;  " << (timeVIUnroll8 / callFit) / 60 << "min" << endl;
//    cout << "-- VI_Unr256 Ges:  " << timeVIUnroll256 << " sek ;  " << timeVIUnroll256 / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeVIUnroll256 / callFit<< " sek ;  " << (timeVIUnroll256 / callFit) / 60 << "min" << endl;
//    cout << "-- VI8For Ges:     " << timeVIUnroll8For << " sek ;  " << timeVIUnroll8For / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeVIUnroll8For / callFit<< " sek ;  " << (timeVIUnroll8For / callFit) / 60 << "min" << endl;
//    cout << "-- VI256For Ges:   " << timeVIUnroll256For << " sek ;  " << timeVIUnroll256For / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeVIUnroll256For / callFit<< " sek ;  " << (timeVIUnroll256For / callFit) / 60 << "min" << endl;
    cout << "-- Runs:   " << callFit << endl << endl;
    cout << "-- errorOk:   " << errorOk << endl;
//
//    cout << "Errors: " << endl
//        << "  Par: " << parDiff << "; VI8: " << vi8Diff << "; 256: " << vi256Diff << endl;
    return 0;
}
