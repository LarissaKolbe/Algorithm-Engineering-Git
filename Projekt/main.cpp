#include <iomanip>
#include <iostream>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include "Coordinates.h"

using namespace std;

void piCalcTest() {
    int num_steps = 100000000; // amount of rectangles
    double width = 1.0 / double(num_steps); // width of a rectangle
    double sum = 0.0; // for summing up all heights of rectangles
    double start_time = omp_get_wtime(); // wall clock time in seconds
#pragma omp parallel num_threads(10)
    {
        int threadNum = omp_get_num_threads();
        int threadId  = omp_get_thread_num();
        double sumLocal = 0.0;
        int stepsPerThread = num_steps / threadNum;
        for (int i = threadId * stepsPerThread; i < (threadId+1) * stepsPerThread; i++) {
            double x = (i + 0.5) * width; // midpoint
            sumLocal = sumLocal + (1.0 / (1.0 + x * x)); // add new height of a rectangle
        }
#pragma omp atomic
        sum += sumLocal;
    }
    double pi = sum * 4 * width; // compute pi
    double run_time = omp_get_wtime() - start_time;

    cout << "pi with " << num_steps << " steps is " << setprecision(17)
         << pi << " in " << setprecision(6) << run_time << " seconds\n";
}


/**
 * Rundet den übergebenen Wert auf die angegebene Anzahl an Nachkommastellen.
 * Standard: 2 Nachkommastellen
 * @param value zu rundender Wert
 * @param decimals Anzahl an Nachkommastellen
 * @return gerundeten Wert
 */
float roundValue(float value, int decimals=2){
    //TODO: schöner machen
    int roundingValue = pow(10, decimals);
    value = (int)(value * roundingValue + .5);
    return value;
}

/**
 * Prüft die Übereinstimmung der neuen Punkte mit der Zielform
 * @param modifiedDS veränderte Punktesammlung
 * @param targetShape Punktesammlung der Zielform
 * @param size Anzahl an Punkten in modifiedDS
 * @return Distanz zwischen modifiedDS und targetShape
 */
float fit(Coordinates modifiedDS[], Coordinates *targetShape, int size){
    //TODO: modifiedDS[i] und targetShape[i] müssen nicht übereinstimmen!
    float distance = 0;
    for(int i=0; i<size; i++){
        float newDist = sqrt(
                pow(modifiedDS[i].x - targetShape[i].x, 2) +
                pow(modifiedDS[i].y - targetShape[i].y, 2));
        distance = distance + newDist;
    }
    return distance;
}

/**
 * Prüft, ob die statischtiscchen Eigenschaften der beiden übergebenen Punktemengen gleich sind.
 * @param newDS veränderte Punktemengen
 * @param initialDS ursprüngliche Punktemengen
 * @param size Größe der Arrays
 * @return boolean
 */
bool isErrorOk(Coordinates *newDS, Coordinates *initialDS, int size) {
    //TODO: berechne und vergleiche statistische Eigenschaften von beiden Datensätzen
    //TODO: schreib ne eigne Funktion für jede Eigenschaft (sonst wirds hier evtl zu lang, wenns mehr werden? andererseits brauchen die alle ne for-Schleife...)
    float meanXNew = 0.0;
    float meanXInit = 0.0;
    for(int i=0; i<size; i++){
        meanXNew = meanXNew + newDS[i].x;
        meanXInit = meanXInit + initialDS[i].x;
    }
    meanXNew = roundValue(meanXNew / size);
    meanXInit = roundValue(meanXInit / size);
    return meanXNew == meanXInit;
}

Coordinates * moveRandomPoints(Coordinates *dataset, int size) {
    //TODO: wenn paralelisiert:
    //srand(omp_get_thread_num())
    srand((unsigned) time(NULL));
    /** Maximale Bewegung die ein Punkt machen kann*/
    //TODO: Idee: fange mit hohem Wert an und gehe runter (vgl Neuronale Netze)
    float maxMovement = 0.1;

    //generiert random Zahlen zwischen 0 und dsSize-1
    //TODO: kann man auch gleich nur Werte in bestimmter range generieren?
    int randomIndex = rand() % size;

    //TODO: dafür sorgen, dass evtl auch mehrere Positionen verändert werden können?
    //TODO: move kann momentan nur positiv sein, muss aber auch negativ gehen!
    float moveX = ((float) rand()) / (float) RAND_MAX  * maxMovement;
    float moveY = ((float) rand()) / (float) RAND_MAX  * maxMovement;
    Coordinates newCoordinates = {
            dataset[randomIndex].x + moveX,
            dataset[randomIndex].y + moveY,
    };
    dataset[randomIndex] = newCoordinates;
    return dataset;
}

Coordinates * perturb(Coordinates dsBefore[], Coordinates *dsNew, Coordinates *targetShape, int size){
    while(true){
        //bewegt random Punkte in dsNeew
        moveRandomPoints(dsNew, size);
        //checkt ob dsNew besser ist als dsBefore
        //TODO: bau das mit Temperatur ein
        if(fit(dsNew, targetShape, size) < fit(dsBefore, targetShape, size)){
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
Coordinates * generateNewPlot(Coordinates *initialDS, Coordinates *currentDS, Coordinates *targetShape, int size) {
    //TODO: iterations hochschalten!
    int iterations = 50;
    //Coordinates currentDS[size];
    //copy(initialDS, initialDS+size, currentDS);
    for (int i = 0; i<iterations; i++){
        Coordinates testDS[size];
        //setzt testDS = currentDS
        std::copy(currentDS, currentDS+size, testDS);
        //verändert TestDS
        perturb(currentDS, testDS, targetShape, size);
        if (isErrorOk(testDS, initialDS, size)){
            //setzt currentDS = testDS
            std::copy(testDS, testDS+size, currentDS);
        }
    }
}

/**
 * Code from: https://www.delftstack.com/howto/cpp/read-ppm-file-cpp/
 *
 * TODO: evtl in mehrere Funktionen aufteilen, nen Teil in die main packen oder ne eigne Klasse/File wo das alles gemacht wird
 */
void readFile(char *fileName){
    //TODO: der Part kann denk ich in die Main
    FILE *read = fopen(fileName, "rb");
    if (read == NULL){
        //TODO: User soll solange Eingabe machen können, bis alles klappt
        cout << "File could not be found!";
        return;
    }

    // read header
    //TODO: das evtl auch in die Main (wegen dem Fehlerabfangen da...)
    unsigned char header[15];
    int test = 0;
    fread( header , 15,  1, read);
    if (header[0]!='P' || header[1]!='6'){
        //TODO: User soll solange Eingabe machen können, bis alles klappt
        cout << "File has wrong format!";
        return;
    }

    //TODO: das könnte in ne eigne Funktion?
    //extract width and height
    int width = 0, height = 0, pos = 3;
    for (;header[pos]!='\n' && header[pos]!=' ';pos++)
        width = width * 10 + (header[pos] - '0');
    pos++;
    for (;header[pos]!='\n' && header[pos]!=' ';pos++)
        height = height * 10 + (header[pos] - '0');

    //declare a dynamic array of size width x height x 3
    // reason: there are width x height pixels in the image, and each image has 3 bytes (RGB)
    unsigned char *image = new unsigned char [width * height * 3];
    fread( image , width * height * 3,  1, read);

    //zählt/sucht alle nahezu schwarzen Punkte
    //TODO: beim input einlesen iwie dafür sorgen, dass jeder Punkt nur wirklich einmal registriert wird! (hat halt meistens noch gräuliche Randpunkte...)
    int index = 0;
    /**Stores those points that make up the shape*/
    Coordinates targetShapeTmp[width * height];
    for (int i=0;i<width * height * 3;i=i+3){
        unsigned char red = image[i];
        unsigned char green = image[i+1];
        unsigned char blue = image[i+2];
        if (red < 200 && green < 200 && blue < 200){
            float currentY = (int)(i / 3 / width);
            float currentX = (int)(i / 3 - currentY * width);
            Coordinates newCoord = {currentX, currentY};
            targetShapeTmp[index] = newCoord;
            index++;
        }
    }
    //creates array of needed length und copies the points in it
    Coordinates targetShape[index];
    copy(targetShapeTmp, targetShapeTmp+index, targetShape);
    //TODO: muss deen array in die Main bekommen (dazu vmtl einiges von hier in die Main verschieben)
}

int main() {
    //nur um zu testen, dass OpenMP funktioniert
    piCalcTest();
    //TODO: fileName soll später als Ussereingabe eingelesen werden
    char *fileName = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                    "/shapes/diagonalStripes_small.ppm";   //read shape
                    //"/input/dots_small_12.ppm";          //read input
                    //"/empty_5x4_5.ppm"                   //read empty

    readFile(fileName);

    //TODO: Bildeingabe und einlesen der Punkte daraus
    Coordinates inputDSTmp[] = {{0, 1}, {1, 2}, {2, 3}};
    Coordinates targetShapeTmp[] = {{1, 2}, {2, 3}, {3, 4}, {4, 5}};
    int size = sizeof(inputDSTmp) / sizeof(Coordinates);

    Coordinates currentDS[size];
    copy(inputDSTmp, inputDSTmp + size, currentDS);

    generateNewPlot(inputDSTmp, currentDS, targetShapeTmp, size);

    //printet Ergebnis
    for (int i=0; i < size; i++){
        cout << "(" << currentDS[i].x << ", " << currentDS[i].y << "), ";
    }

    return 0;
}
