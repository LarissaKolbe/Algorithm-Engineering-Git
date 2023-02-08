#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"
#include <omp.h>
#include <cmath>
#include <random>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "datatypes.h"
#include "exportHelper.h"
#include "importHelper.h"
#include "helperFunctions.h"
#include "configurationHelpers.h"
//#include "aligned_allocator.h"
//
//template<class T>
//using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

using namespace std;

//TODO: Bei allen Funktionen ect Documentation dazu schreiben und Kommentare zum Code machen!
// auch in den Helper und Header Dateien!!

//TODO: Alle Anweisungen etc umformulieren (auf eine Sprache einigen!)

//TODO: messen in welcher FUnktion wie viel Zeit verbracht wird!

int outputHeight, outputWidth, inputSize, numberPointsTarget;

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

/**
 * Prüft die Übereinstimmung der neuen Punkte mit der Zielform
 * @param modifiedDS  veränderte Punktesammlung
 * @param targetShape Punktesammlung der Zielform
 * @param size        Anzahl an Punkten in modifiedDS
 * @param sizeTarget  Anzahl an Punkten in targetShape
 * @return boolean ob distance kleiner geworden ist oder nicht
 */
bool isBetterFit(vector<Coordinates> modifiedDS, vector<Coordinates> previousDS, vector<Coordinates> targetShape){
    double distanceMod = 0, distancePrev = 0;
//    auto start = omp_get_wtime();
    //TODO: berechnung noch effizienter/performanter machen wenn möglich
#pragma omp parallel for reduction(+: distanceMod, distancePrev)
    for (int i = 0; i < inputSize; i++) {
        double distMinMod = max(outputHeight, outputWidth);
        double distMinPrev = max(outputHeight, outputWidth);
        //sets distMin to the smallest distance between modifiedDS[i]/previousDS[i] and a point from targetShape
#pragma omp parallel for reduction(min: distMinMod, distMinPrev)
        for (int j = 0; j < numberPointsTarget; j++) {
            double distMod = sqrt(
                    pow(modifiedDS[i].x - targetShape[j].x, 2) +
                    pow(modifiedDS[i].y - targetShape[j].y, 2));
            double distPrev = sqrt(
                    pow(previousDS[i].x - targetShape[j].x, 2) +
                    pow(previousDS[i].y - targetShape[j].y, 2));
            distMinMod = min(distMinMod, distMod);
            distMinPrev = min(distMinPrev, distPrev);
        }
        distanceMod += distMinMod;
        distancePrev += distMinPrev;
    }
//    cout << "\n-- Dauer dist: " << omp_get_wtime() - start << " seconds" << endl;
    return distanceMod < distancePrev;
}

/**
 * Berechnet die statistischen Eigenschaften des übergebenen Datensatzes.
 * @param dataset
 * @return
 */
statisticalProperties calculateStatisticalProperties(vector<Coordinates> dataset){
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
bool isErrorOk(vector<Coordinates> newDS, statisticalProperties initialProps) {
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
vector<Coordinates> moveRandomPoint(vector<Coordinates> dataset) {
    std::random_device rd;  // seed for the random number engine
    std::mt19937 gen(rd());  // Mersenne Twister random number engine
    std::uniform_real_distribution<> disForIndex(0, inputSize);
    std::uniform_real_distribution<> disForMove(-conf.maxMovement, conf.maxMovement);  // define the range of the float numbers

    int randomIndex = disForIndex(gen); //bestimmt welcher Punkt verschoben wird

    //bewegt Punkte um Werte zwischen + und - maxMovement
//    int nrPossibleValues = maxMovement * 2 + 1;
    Coordinates newCoordinates;
    //do-while stellt sicher, dass die neue Position noch innerhalb der Outputgröße ist
    // andernfalls wird neu berechnet
    do {
        double moveX = disForMove(gen);
        double moveY = disForMove(gen); // erstellt die random Zahl
        newCoordinates = {
                dataset[randomIndex].x + moveX,
                dataset[randomIndex].y + moveY,
        };
    } while(round(newCoordinates.x) >= outputWidth || round(newCoordinates.y) >= outputHeight
        || newCoordinates.x < 0 || newCoordinates.y < 0);
    dataset[randomIndex] = newCoordinates;
    return dataset;
}

vector<Coordinates> perturb(vector<Coordinates> dsBefore, vector<Coordinates> targetShape, double temp){
    vector<Coordinates> dsNew(dsBefore);
    //will move points until if-condition is fulfilled
    while(true){
        //bewegt random Punkte in dsNew
        dsNew = moveRandomPoint(dsNew);
        //akzeptiert dsNew, wenn es näher an der targetShape ist als dsBefore
        // akzeptiert es auch, wenn die Zufallszahl kleiner als temp ist
//        double fitNew = fit(dsNew, targetShape);
//        double fitBefore = fit(dsBefore, targetShape);
        std::random_device rd;  // seed for the random number engine
        std::mt19937 gen(rd());  // Mersenne Twister random number engine
        std::uniform_real_distribution<> dis(0, 1);
        double randomNr = dis(gen);
        //TODO: manchmal bleibt das programm iwo ewig hängen, weil es die punkte immer weiter in falsche Richtungen bewegt
        // und fit dadurch einfach nicht passt
        if(isBetterFit(dsNew, dsBefore, targetShape)  //fitNew < fitBefore
            || randomNr < temp){
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
vector<Coordinates> generateNewPlot(vector<Coordinates> initialDS, vector<Coordinates> targetShape, statisticalProperties initialProperties) {
    vector<Coordinates> currentDS(initialDS);
//#pragma omp for
    for (int i = 0; i<conf.iterations; i++){
        //TODO: am Ende wieder rausnehmen
        if(i % 10000 == 0) {
            cout << endl << "Iteration: " << i  << endl;
            vector<Coordinates> dataset(currentDS);
            string fileNameTest = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                                    "/output/perIteration/output" + to_string(i) + ".ppm";
//#pragma omp critical
            exportImage(fileNameTest, dataset, 255, outputHeight, outputWidth);
        }

        //"temperature" starts with 0.4 and goes down until 0 with each iteration
        double temp = (conf.maxTemp - conf.minTemp) * ((conf.iterations-i)/conf.iterations) + conf.minTemp;

        vector<Coordinates> testDS = perturb(currentDS, targetShape, temp);
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
    FileInformation targetInfo = readTargetData(targetShape);

    //Liest Outputpfad ein
    string fileNameExport = readExportPath();

    //gives user opportunity to change configurations
    conf = setConfigurations(conf);

    //TODO: zum zeitmessen; kommt ganz am Ende raus
    auto start = omp_get_wtime();

    //Reads data from input and target images
    vector<Coordinates> inputVector = createVectorFromImage(inputInfo);
    vector<Coordinates> targetShape = createVectorFromImage(targetInfo);

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
    vector<Coordinates> result = generateNewPlot(inputVector, targetShape, initialProperties);

    // Create export image file
    exportImage(fileNameExport, result, inputInfo.maxColor, outputHeight, outputWidth);

    //TODO: Am Ende rausnehmen
    float runtime = omp_get_wtime() - start;
    cout << endl << "-- Dauer:  " << runtime << " sek ;  " << runtime / 60 << "min" << endl;
    return 0;
}

#pragma clang diagnostic pop