#include <iomanip>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <random>
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
 * @param modifiedDS  veränderte Punktesammlung
 * @param targetShape Punktesammlung der Zielform
 * @param size        Anzahl an Punkten in modifiedDS
 * @param sizeTarget  Anzahl an Punkten in targetShape
 * @return Distanz zwischen modifiedDS und targetShape
 */
float fit(Coordinates modifiedDS[], Coordinates *targetShape, int sizeDS, int sizeTarget){
    float distance = 0;
    for(int i=0; i<sizeDS; i++){
        float distMin = 999999999999;
        //sets distMin to the smallest distance between modifiedDS[i] and a point from targetShape
        for(int j=0; j<sizeTarget; j++){
            float distThis = sqrt(
                    pow(modifiedDS[i].x - targetShape[j].x, 2) +
                    pow(modifiedDS[i].y - targetShape[j].y, 2));
            if (distThis < distMin){
                distMin = distThis;
            }
        }
        distance = distance + distMin;
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

/**
 *
 * @param dataset     Dataset in which points should be moved
 * @param size        Size of the dataset
 * @param maxMovement Maximale Bewegung die ein Punkt machen kann
 * @return
 */
Coordinates * moveRandomPoints(Coordinates *dataset, int size, float maxMovement) {
    //TODO: wenn paralelisiert setze id auf omp_get_thread_num()
    int id = 1; //omp_get_thread_num();
    srand((unsigned) time(NULL)+id);
    int randomIndex = rand() % size; //bestimmt welcher Punkt verschoben wird

    //TODO: evtl Koordinaten nur um int's verschieben? (weil ppm-Bild eh nur int's darstellen kann)
    //bewegt Punkte um Werte zwischen + und - maxMovement
    float moveX = (((float(rand()) / float(RAND_MAX)) * (maxMovement + maxMovement)) - maxMovement);
    float moveY = (((float(rand()) / float(RAND_MAX)) * (maxMovement + maxMovement)) - maxMovement);
    dataset[randomIndex] = {
            dataset[randomIndex].x + moveX,
            dataset[randomIndex].y + moveY,
    };
    return dataset;
}

Coordinates * perturb(Coordinates dsBefore[], Coordinates *dsNew, Coordinates *targetShape, int size, int sizeTarget, float temp, float maxMovement){
    while(true){
        //bewegt random Punkte in dsNew
        moveRandomPoints(dsNew, size, maxMovement);
        //TODO: wenn paralelisiert setze id auf omp_get_thread_num()
        //int id = 1; //omp_get_thread_num();
        //srand((unsigned) time(NULL)+id);

        //akzeptiert dsNew, wenn es näher an der targetShape ist als dsBefore
        // akzeptiert es auch, wenn die Zufallszahl kleiner als temp ist
        float fitNew = fit(dsNew, targetShape, size, sizeTarget);
        float fitBefore = fit(dsBefore, targetShape, size, sizeTarget);
        float randomNr = (float) rand() / (float) RAND_MAX;
        if(fitNew < fitBefore
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
Coordinates * generateNewPlot(Coordinates *initialDS, Coordinates *currentDS, Coordinates *targetShape, int size, int sizeTarget) {
    //TODO: iterations hochschalten!
    int iterations = 1000;
    //Coordinates currentDS[size];
    //copy(initialDS, initialDS+size, currentDS);
    for (int i = 0; i<iterations; i++){
        Coordinates testDS[size];
        //setzt testDS = currentDS
        std::copy(currentDS, currentDS+size, testDS);
        //"temperature" starts with 0.4 and goes down until 0 with each iteration
        float maxTemp = 0.4, minTemp = 0.1;
        float temp = (maxTemp - minTemp) * ((iterations-i)/iterations) + minTemp;
        //maximal movement a point makes at once. Starts with 5 and goes down until 0.1 with each iteration
        float movementStart = 5, movementEnd = 0.1;
        float maxMovement = (movementStart - movementEnd) * ((iterations-i)/iterations) + movementEnd;
        //verändert TestDS
        perturb(currentDS, testDS, targetShape, size, sizeTarget, temp, maxMovement);
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
int createArrayFromImage(FILE *file, unsigned char *image, int width, int height, Coordinates *array){
    fread( image , width * height * 3,  1, file);
    //zählt/sucht alle nahezu schwarzen Punkte
    //TODO: beim input einlesen iwie dafür sorgen, dass jeder Punkt nur wirklich einmal registriert wird! (hat halt meistens noch gräuliche Randpunkte...)
    int index = 0;
    for (int i=0;i<width * height * 3;i=i+3){
        unsigned char red = image[i];
        unsigned char green = image[i+1];
        unsigned char blue = image[i+2];
        if (red < 200 && green < 200 && blue < 200){
            float currentY = (i / 3 / width);
            float currentX = (i / 3 - currentY * width);
            Coordinates newCoord = {currentX, currentY};
            array[index] = newCoord;
            index++;
        }
    }
    return index;
}

/**
 * Exportiert ppm-File mit schwarzen Pixeln an den in points angegebenen Koordinaten.
 * CCode von ChatGPT
 * @param fileName
 * @param points
 * @param heightTarget
 * @param widthTarget
 */
void exportImage(string fileName, Coordinates *points, int heightTarget, int widthTarget){
    std::ofstream outputFile(fileName, std::ios::binary);

    // Write the PPM image header
    outputFile << "P6" << std::endl;
    outputFile << widthTarget << " " << heightTarget << std::endl;
    outputFile << 255 << std::endl;

    // Iterate through each pixel of the image
    int index = 0;
    for (int i=0;i<widthTarget * heightTarget * 3;i=i+3){
        int currentY = (i / 3 / widthTarget);
        int currentX = (i / 3 - currentY * widthTarget);
        // Write the red, green, and blue values for the pixel
        // black, if coordinates match currentDS, else white
        if(currentX == (int)points[index].x && currentY == (int)points[index].y){
            outputFile.put(0);
            outputFile.put(0);
            outputFile.put(0);
            index++;
        } else {
            outputFile.put(255);
            outputFile.put(255);
            outputFile.put(255);
        }
    }
    // Close the image file
    outputFile.close();
}

//To extract width & height from header
int getDimension(unsigned char *header, int &pos){
    int dim=0;
    for ( ;header[pos]!='\n' && header[pos]!=' ';pos++)
        dim = dim * 10 + (header[pos] - '0');
    return dim;
}

int main() {
    //nur um zu testen, dass OpenMP funktioniert
    //piCalcTest();

    //TODO: fileName soll später als Ussereingabe eingelesen werden
    char *fileNameInit = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                         "/input/dots_small_12.ppm";          //read input
    char *fileNameTarget = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                        "/shapes/diagonalStripes_small.ppm";   //read shape
                        //"/empty_5x4_5.ppm"                   //read empty

    FILE *initialFile, *targetFile;

    //TODO: ändere die ganzen Texte
    bool incorrectInput = false;
    unsigned char headerInit[15];
    //Wenn etwas mit dem angegebenen File nicht passt, muss der User so lange einen neuen angeben, bis es passt
    do {
        printf("Gib die Startdaten an: ");
        initialFile = fopen(fileNameInit, "rb");
        if (initialFile == NULL){
            cout << "File could not be found!";
            incorrectInput = true;
            continue;
        }
        // read header
        fread( headerInit , 15,  1, initialFile);
        if (headerInit[0]!='P' || headerInit[1]!='6'){
            cout << "File has wrong format!";
            incorrectInput = true;
            continue;
        }
    } while(incorrectInput);
    int pos = 3;
    int widthInit = getDimension(headerInit, pos);
    pos++;
    int heightInit = getDimension(headerInit, pos);

    //Zielform:
    incorrectInput = false;
    unsigned char headerTarget[15];
    //Wenn etwas mit dem angegebenen File nicht passt, muss der User so lange einen neuen angeben, bis es passt
    do {
        printf("Gib die Zielform an: ");
        targetFile = fopen(fileNameTarget, "rb");
        if (targetFile == NULL){
            cout << "File could not be found!";
            incorrectInput = true;
            continue;
        }
        // read header
        fread( headerTarget , 15,  1, targetFile);
        if (headerTarget[0]!='P' || headerTarget[1]!='6'){
            cout << "File has wrong format!";
            incorrectInput = true;
            continue;
        }
    } while(incorrectInput);
    pos = 3;
    int widthTarget = getDimension(headerTarget, pos);
    pos++;
    int heightTarget = getDimension(headerTarget, pos);


    //declare a dynamic array of size width x height x 3
    // reason: there are width x height pixels in the image, and each image has 3 bytes (RGB)
    unsigned char *imageInit = new unsigned char [widthInit * heightInit * 3];
    unsigned char *imageTarget = new unsigned char [widthTarget * heightTarget * 3];
    Coordinates inputDSTmp[widthInit * heightInit];
    Coordinates targetShapeTmp[widthTarget * heightTarget];
    int sizeInit = createArrayFromImage(initialFile, imageInit, widthInit, heightInit, inputDSTmp);
    int sizeTarget = createArrayFromImage(targetFile, imageTarget, widthTarget, heightTarget, targetShapeTmp);
    //creates array of needed length und copies the points in it
    Coordinates inputDS[sizeInit];
    copy(inputDSTmp, inputDSTmp+sizeInit, inputDS);
    Coordinates targetShape[sizeTarget];
    copy(targetShapeTmp, targetShapeTmp+sizeTarget, targetShape);

    //Punkte zum testen, die nicht aus files kommen:
    //Coordinates inputDSTmp[] = {{0, 1}, {1, 2}, {2, 3}};
    //Coordinates targetShapeTmp[] = {{1, 2}, {2, 3}, {3, 4}, {4, 5}, {0.5, 2.5}, {0.3, 1.5}};
    //int size = sizeof(inputDSTmp) / sizeof(Coordinates);
    //int sizeTarget = sizeof(targetShapeTmp) / sizeof(Coordinates);

    Coordinates currentDS[sizeInit];
    copy(inputDS, inputDS + sizeInit, currentDS);

    //generateNewPlot(inputDS, currentDS, targetShape, sizeInit, sizeTarget);

    //printet Ergebnis
    for (int i=0; i < sizeInit; i++){
        cout << "(" << currentDS[i].x << ", " << currentDS[i].y << "), ";
    }

    // Create the image file
    string fileNameExport = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                      "/output/output.ppm";
    exportImage(fileNameExport, currentDS, heightTarget, widthTarget);

    return 0;
}
