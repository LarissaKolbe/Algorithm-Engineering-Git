
#include <omp.h>
#include <cmath>
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

double timeCalc = 0.0, timeExport = 0.0, timeMRP = 0.0, timePertub = 0.0;
int callCalc = 0, callExp = 0, callMRP = 0, callPertub = 0;


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

bool isBetterFitControll (Coordinates pointNew, Coordinates pointPrev, aligned_vector<Coordinates> targetShape){
    float minDistNew = getDistance(pointNew, targetShape[0]);
    float minDistPrev = getDistance(pointPrev, targetShape[0]);

#pragma omp parallel for reduction(min: minDistNew, minDistPrev)
    for (int j = 1; j < numberPointsTarget; j++) {
        minDistNew = min(minDistNew, getDistance(pointNew, targetShape[j]));
        minDistPrev = min(minDistPrev, getDistance(pointPrev, targetShape[j]));
    }
//    cout << "Contr: " << minDistNew << " | " << minDistPrev << endl;
    return minDistNew < minDistPrev;
}


double timeVIUnroll4 = 0.0, timeVIUnroll8 = 0.0, timeVIUnroll256 = 0.0; // timeVI8=0.0, timeVIOmp8 = 0.0;
int callFit = 0;
int vi4Diff=0, vi8Diff=0, vi256Diff=0;

bool isBetterFit(Coordinates pointNew, Coordinates pointPrev, aligned_vector<Coordinates> targetShape) {

//    bool controll = isBetterFitControll(pointNew, pointPrev, targetShape);

//    bool viUnroll4 = controll;
//    auto start = omp_get_wtime();
//    bool viUnroll4 = isBetterFit_VIUnroll4(pointNew, pointPrev, targetShape);
//    timeVIUnroll4 += omp_get_wtime() - start;

    auto start = omp_get_wtime();
    bool viUnroll8 = isBetterFit_VIUnroll8(pointNew, pointPrev, targetShape.data(), (int)targetShape.size());
    timeVIUnroll8 += omp_get_wtime() - start;

    start = omp_get_wtime();
    bool viUnroll256 = isBetterFit_VIUnroll256(pointNew, pointPrev, targetShape.data(), (int)targetShape.size());
    timeVIUnroll256 += omp_get_wtime() - start;

    //TODO: die einzelnen Funktionen haben manchmal versch Ergebnisse!
    // Liegt am unroll, dass funktioniert nicht wie es soll.
    // Dadurch kommen teils NaN Werte in die Arrays, woduch andere Werte bei Operationen mit denne rausgelöscht werden
//    if(controll != viUnroll4 || controll != viUnroll256 || controll != viUnroll8){
//        cout << "!!!!! ERROR - FITNESS UNTERSCHIEDLICH !!!!!" << endl;
//    }
//    //VI4 hat am meisten Errors: ca 6936/409612 = 17/1000
//    //VI8 & 256 haben gleich viele: 2453/409612 =  6/1000
//    if (controll != viUnroll4){
//        vi4Diff++;
//    } if (controll != viUnroll8){
//        vi8Diff++;
//    } if (controll != viUnroll256){
//        vi256Diff++;
//    }
    callFit++;
    return viUnroll256;
}

/**
 * Berechnet die statistischen Eigenschaften des übergebenen Datensatzes.
 * @param dataset
 * @return
 */
statisticalProperties calculateStatisticalProperties(Coordinates *__restrict__ dataset, int decimals){
    auto start = omp_get_wtime();
    //TODO: berechnung noch effizienter/performanter machen wenn möglich
    float meanX = 0.0, meanY = 0.0, varianceX = 0.0, varianceY = 0.0, stdDeviationX, stdDeviationY;
#pragma omp simd aligned(dataset : 64) reduction(+: meanX, meanY)
    for(int i=0; i<inputSize; i++){
        meanX += dataset[i].x;
        meanY += dataset[i].y;
    }
    meanX = roundValue(meanX / (float)inputSize, decimals);
    meanY = roundValue(meanY / (float)inputSize, decimals);

#pragma omp simd aligned(dataset : 64) reduction(+: varianceX, varianceY)
    for(int i=0; i<inputSize; i++){
        varianceX += (float)pow(dataset[i].x - meanX, 2);
        varianceY += (float)pow(dataset[i].y - meanY, 2);
    }
    varianceX = varianceX / (float)(inputSize-1);
    varianceY = varianceY / (float)(inputSize-1);
    stdDeviationX = roundValue(sqrt(varianceX), decimals);
    stdDeviationY = roundValue(sqrt(varianceY), decimals);
    varianceX = roundValue(varianceX, decimals);
    varianceY = roundValue(varianceY, decimals);

    timeCalc += omp_get_wtime() - start;
    callCalc++;
    return {meanX, meanY, varianceX, varianceY, stdDeviationX, stdDeviationY};
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
    statisticalProperties newProps = calculateStatisticalProperties(newDS.data(), conf.decimals);
    bool meanX = abs(newProps.meanX - initialProps.meanX) <= conf.accuracy;
    bool meanY = abs(newProps.meanY - initialProps.meanY) <= conf.accuracy;
    bool stdDeviationX = abs(newProps.stdDeviationX - initialProps.stdDeviationX) <= conf.accuracy;
    bool stdDeviationY = abs(newProps.stdDeviationY - initialProps.stdDeviationY) <= conf.accuracy;

    return meanX && meanY && stdDeviationX && stdDeviationY;

//     return abs(newProps.meanX - initialProps.meanX) <= conf.accuracy
//        && abs(newProps.meanY - initialProps.meanY) <= conf.accuracy
////        && abs(newProps.varianceX - initialProps.varianceX) <= conf.accuracy
////        && abs(newProps.varianceY - initialProps.varianceY) <= conf.accuracy;
//        && abs(newProps.stdDeviationX - initialProps.stdDeviationX) <= conf.accuracy
//        && abs(newProps.stdDeviationY - initialProps.stdDeviationY) <= conf.accuracy;
}

/**
 * Bewegt zufälligen Punkt im übergebenen Datensatz um maximal maxMovement viele Stellen.
 * Will change the passed vector.
 *
 * @param pointToMove     Dataset in which points should be moved
 * @param size        Size of the pointToMove
 * @param maxMovement Maximale Bewegung die ein Punkt machen kann
 * @return
 */
Coordinates moveRandomPoint(Coordinates pointToMove, mt19937 gen) {
    uniform_real_distribution<> dis(-conf.maxMovement, conf.maxMovement);  // define the range of the float numbers
    Coordinates newCoordinates;
    do {
        newCoordinates = {
                pointToMove.x + (float)dis(gen),
                pointToMove.y + (float)dis(gen),
        };
    // akzeptiert Bewegung nur, wenn sie noch innerhalb der Bildgrenzen ist, andernfalls wird neu berechnet
    } while(round(newCoordinates.x) >= outputWidth || round(newCoordinates.y) >= outputHeight
        || newCoordinates.x < 0 || newCoordinates.y < 0);
    return newCoordinates;
}

aligned_vector<Coordinates> perturb(aligned_vector<Coordinates> dsBefore, const aligned_vector<Coordinates>& targetShape, double temp){
    //kopiert Datensatz
    aligned_vector<Coordinates> dsNew(dsBefore);

    //initialisiert Zufallszahlengenerator
    random_device rd;
    mt19937 gen(rd());

    //bewegt zufällige Punkte bis if-Bedingung erfüllt wird
    while(true){
        //bestimmt, welcher Punkt verschoben wird
        uniform_real_distribution<> disForIndex(0, inputSize);
        int indexToMove = (int)disForIndex(gen);

        //bewegt zufälligen Punkt in dsNew
        auto startMRP = omp_get_wtime();
        dsNew[indexToMove] = moveRandomPoint(dsNew[indexToMove], gen);

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
aligned_vector<Coordinates> generateNewPlot(aligned_vector<Coordinates> currentDS, const aligned_vector<Coordinates>& targetShape, statisticalProperties initialProperties) {
//#pragma omp for
    for (int i = 0; i<conf.iterations; i++){
        //TODO: am Ende wieder rausnehmen
        if(i % 50000 == 0) {
            auto start = omp_get_wtime();
            cout << endl << "Iteration: " << i  << endl;
            string fileNameTest = "../data/output/output" + to_string(i) + ".ppm";
//#pragma omp critical
            exportImage(fileNameTest, currentDS, 255, outputHeight, outputWidth);
            timeExport += omp_get_wtime() - start;
            callExp++;
        }

        //"temperature" starts with 0.4 and goes down until 0 with each iteration
        double temp = (conf.maxTemp - conf.minTemp) * ((float)(conf.iterations-i)/(float)conf.iterations) + conf.minTemp;

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

aligned_vector<Coordinates> cleanDatasetTmp(aligned_vector<Coordinates> dataset){
    int size = (int)dataset.size();
    //rounds all entries to integer
    // because in the ppm-file every pixel is represented by integer coordinates
#pragma omp parallel for
    for (int i=0; i < size; i++){
        dataset[i].x = round(dataset[i].x);
        dataset[i].y = round(dataset[i].y);
    }

    //sortiert dataset nach y-Koordinaten
    sort(dataset.begin(), dataset.end(), compareByY);

    //deletes duplicate entries
    // if we don't do this, there will be problems while creating the ppm-image
    for (int i=0; i < size; i++){
        //compares current element with next element
        if(i+1<size && dataset[i].x == dataset[i+1].x && dataset[i].y == dataset[i+1].y){
            cout << "Erased double Point: (" << dataset[i+1].x << ", " << dataset[i+1].y << ")" << endl;
            //erases element if it is same as next
            dataset.erase(dataset.begin() + i);
            size -= 1;
            i--;
        }
    }
    return dataset;
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
    FileInformation inputInfo = readImageData(initialData);
    FileInformation targetInfo = readImageData(target);

    //Liest Outputpfad ein
    string fileNameExport = readExportPath();

    //gives user opportunity to change configurations
    conf = setConfigurations(conf);

    //TODO: zum zeitmessen; kommt ganz am Ende raus
    auto startProgram = omp_get_wtime();

    //Reads data from input and target images
    aligned_vector<Coordinates> inputVector = createVectorFromImage(inputInfo.imageData, inputInfo.width, inputInfo.height);
    aligned_vector<Coordinates> targetShape = createVectorFromImage(targetInfo.imageData, targetInfo.width, targetInfo.height);

    //sets global Parameters
    inputSize = (int)inputVector.size();
    numberPointsTarget = (int)targetShape.size();
    setOutputWidth(inputInfo.width, targetInfo.width);
    setOutputHeight(inputInfo.height, targetInfo.height);
    // Release memory from heap after using it
    delete[] inputInfo.imageData, delete[] targetInfo.imageData;

    //calculates statistical properties of input data
    statisticalProperties initialProperties = calculateStatisticalProperties(inputVector.data(), conf.decimals);

    //generates new plot with same statistical properties but different point alignment
    aligned_vector<Coordinates> result = generateNewPlot(inputVector, targetShape, initialProperties);

    // Create export image file
    auto startExp = omp_get_wtime();
    exportImage(fileNameExport, result, inputInfo.maxColor, outputHeight, outputWidth);
    timeExport += omp_get_wtime() - startExp;
    callExp++;

    //TODO: Am Ende rausnehmen
    double runtime = omp_get_wtime() - startProgram;
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

    cout << "-- VI_Unroll4 Ges: " << timeVIUnroll4 << " sek ;  " << timeVIUnroll4 / 60 << "min" << endl;
    cout << "--   Dist/run:     " << timeVIUnroll4 / callFit<< " sek ;  " << (timeVIUnroll4 / callFit) / 60 << "min" << endl;
    cout << "-- VI_Unroll8 Ges: " << timeVIUnroll8 << " sek ;  " << timeVIUnroll8 / 60 << "min" << endl;
    cout << "--   Dist/run:     " << timeVIUnroll8 / callFit<< " sek ;  " << (timeVIUnroll8 / callFit) / 60 << "min" << endl;
    cout << "-- VI_Unr256 Ges:  " << timeVIUnroll256 << " sek ;  " << timeVIUnroll256 / 60 << "min" << endl;
    cout << "--   Dist/run:     " << timeVIUnroll256 / callFit<< " sek ;  " << (timeVIUnroll256 / callFit) / 60 << "min" << endl;
//    cout << "-- VI_8 Ges:       " << timeVI8 << " sek ;  " << timeVI8 / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeVI8 / callFit<< " sek ;  " << (timeVI8 / callFit) / 60 << "min" << endl;
//    cout << "-- VI_Omp8 Ges:    " << timeVIOmp8 << " sek ;  " << timeVIOmp8 / 60 << "min" << endl;
//    cout << "--   Dist/run:     " << timeVIOmp8 / callFit<< " sek ;  " << (timeVIOmp8 / callFit) / 60 << "min" << endl;
    cout << "-- Runs:   " << callFit << endl;

    cout << "Errors: " << endl
        << "VI4: " << vi4Diff << "; VI8: " << vi8Diff << "; 256: " << vi256Diff << endl;
    return 0;
} //  ../data/shapes/exportedShapes/shape4_size3002.ppm
