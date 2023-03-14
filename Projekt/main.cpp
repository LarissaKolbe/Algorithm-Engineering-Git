#include <cmath>
#include <omp.h>
#include <random>
#include <iostream>
#include <algorithm>
#include "datatypes.h"
#include "helperFunctions.h"
#include "exportHelper.h"
#include "importHelper.h"
#include "fitnessHelper.h"
#include "configurationHelpers.h"

using namespace std;


#if defined(_WIN32) // restrict pointers on Windows
    #if defined(_MSC_VER) || defined(__ICL)
        #define __restrict__ __restrict
    #endif
#endif


/**
 * Vergleicht die minimale Distanz der beiden übergebenen Punkte zur Zielform.
 * Ruft eine der Implementierungen auf (Standard: Native Implementierung).
 * @param p1  die neuen Koordinaten des Punktes
 * @param p2 die vorherigen Koordinaten des Punktes
 * @param targetShape Vektor von Koordinaten der Zielform
 * @param size Größe von `targetShape`
 * @return true wenn minimale Distanz von p1 < p2, false sonst
 */
bool isBetterFit(const Coordinate pointNew, const Coordinate pointPrev, const Coordinate*__restrict__ targetShape, const int targetSize) {
    return isBetterFit_Naive(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_Parallel(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI128_Lu(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI256_Lu(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI128_LuPar(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI256_LuPar(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI128(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI256(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI128_Par(pointNew, pointPrev, targetShape, targetSize);
//    return isBetterFit_VI256_Par(pointNew, pointPrev, targetShape, targetSize);
}

/**
 * Berechnet die statistischen Eigenschaften des übergebenen Datensatzes
 * @param dataset Datensatz
 * @param size Größe des Datensatzes
 * @param decimals Bestimmt, wie viele Nachkommestellen betrachtet werden sollen
 * @return statistische Eigenschaften des übergebenen Datensatzes
 */
statisticalProperties calculateStatisticalProperties(const Coordinate *__restrict__ dataset, const int size, const int decimals){
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
    const statisticalProperties currentProps = calculateStatisticalProperties(currentDS, size, decimals);

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
Coordinate movePoint(const Coordinate pointToMove, const int maxX, const int maxY, const float maxMovement, mt19937 gen) {
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
 * @param targetSize  Größe des `targetShape` Vektors
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
        const Coordinate modifiedPoint = movePoint(pointToMove, maxX, maxY, maxMovement, gen);

        //akzeptiert bewegten Punkt, wenn er näher an der Zielform ist als vorher oder
        // wenn die Zufallszahl kleiner als temp ist
        if(isBetterFit(modifiedPoint, pointToMove, targetShape, targetSize) || distTemp(gen) < temp){
            return modifiedPoint;
        }
    }
}

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

        //berechnet aktuelle "temperature"
        // beginnt mit maxTemp und nähert sich mit jeder Iteration minTemp
        const double temp = (maxTemp - minTemp) * ((double) (iterations - i) / (double) iterations) + minTemp;

        //bestimmt zufälligen Punkt, der bewegt werden soll
        const int randomIndex = (int) distIndex(gen);

        //bewegt den übergebenen Punkt in Richtung der Zielform
        const Coordinate movedPoint = perturb(currentDS[randomIndex], targetShape.data(), targetSize, maxX, maxY, temp, maxMovement, gen);

#pragma omp critical
        {
            aligned_vector<Coordinate> testDS(currentDS);
            testDS[randomIndex] = movedPoint;

            //prüft, ob statistische Eigenschaften noch mit den anfänglichen übereinstimmen
            if (isErrorOk(testDS.data(), initialProperties, dataSize, accuracy, decimals)) {
                currentDS[randomIndex] = movedPoint;
            }
        }
    }
    return currentDS;
}

int main() {
    //Userinterface zum Wählen der Input- und Zieldatei.
    // User hat die Wahl selbst Bilder hochzuladen oder eins der bereitgestellten Bilder zu verwenden
    const FileInformation inputInfo = readImageData(initialData);
    const FileInformation targetInfo = readImageData(target);

    //Userinterface zum Wählen des Outputpfades
    const string fileNameExport = readExportPath();

    //Userinterface zum Einzusehen und Verändern der Konfigurationen
    const Configurations conf = setConfigurations();

    //Misst Laufzeit des Algorithmus
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

    //printet die statistischen Eigenschaften
    cout << endl
        << "Die graphischen Eigenschaften des Datensatzes sind: " << endl
        << "  Durchschnitt der x-Werte:       " << initialProperties.meanX << endl
        << "  Durchschnitt der y-Werte:       " << initialProperties.meanY << endl
        << "  Standardabweichung der x-Werte: " << initialProperties.stdDeviationX << endl
        << "  Standardabweichung der x-Werte: " << initialProperties.stdDeviationY << endl;

    //Bewegt Inputdaten in Richtung der Zielform, ohne dabei die statistischen Eigenschaften zu verändern
    aligned_vector<Coordinate> result = generateNewPlot(initialDS, targetShape, initialProperties, conf, outputWidth, outputHeight);

    //Exportiert das Ergebnisbild
    exportImage(fileNameExport, result, inputInfo.maxColor, outputHeight, outputWidth);

    double runtime = omp_get_wtime() - startProgram;
    cout << endl << "Laufzeit:  " << runtime << " sek" << endl;

    cout << endl << "Du findest das Ergebnis hier:  " << fileNameExport << endl;

    return 0;
}
