#include <iomanip>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <random>

using namespace std;

struct Coordinates {
    int x;
    int y;
};

struct FileInformation {
    int width;
    int height;
    char *imageData;
};

//TODO: kann man die private oder so machen?
int outputWidth, outputHeight, inputSize, numberPointsTarget;

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
 * Rundet den übergebenen Wert auf die angegebene Anzahl an Nachkommastellen.
 * Standard: keine Nachkommastellen
 * @param value zu rundender Wert
 * @param decimals Anzahl an Nachkommastellen
 * @return gerundeten Wert
 */
float roundValue(float value, int decimals=0){
    //TODO: schöner machen
    int roundingValue = pow(10, decimals);
    value = int(value * roundingValue + .5);
    return value;
}

/**
 * Prüft die Übereinstimmung der neuen Punkte mit der Zielform
 * @param modifiedDS  veränderte Punktesammlung
 * @param targetShape Punktesammlung der Zielform
 * @param size        Anzahl an Punkten in modifiedDS
 * @param sizeTarget  Anzahl an Punkten in targetShape
 * @return Distanz zwischen modifiedDS und targetShape
 *
 * TODO: besserer Name vlt distance? Oder die Rückgabe auf was zwischen 0 und 1 setzen, aber das ist auch komisch und unnötiger Aufwand
 */
float fit(Coordinates modifiedDS[], Coordinates *targetShape){
    float distance = 0;
    for(int i=0; i<inputSize; i++){
        float distMin = 999999999999;
        //sets distMin to the smallest distance between modifiedDS[i] and a point from targetShape
        for(int j=0; j<numberPointsTarget; j++){
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
bool isErrorOk(Coordinates *newDS, Coordinates *initialDS) {
    //TODO: berechne und vergleiche statistische Eigenschaften von beiden Datensätzen
    //TODO: schreib ne eigne Funktion für jede Eigenschaft (sonst wirds hier evtl zu lang, wenns mehr werden? andererseits brauchen die alle ne for-Schleife...)
    float meanXNew = 0.0;
    float meanXInit = 0.0;
    for(int i=0; i<inputSize; i++){
        meanXNew = meanXNew + newDS[i].x;
        meanXInit = meanXInit + initialDS[i].x;
    }
    meanXNew = roundValue(meanXNew / inputSize);
    meanXInit = roundValue(meanXInit / inputSize);
    return meanXNew == meanXInit;
}

/**
 * Bewegt zufälligen Punkt im übergebenen Datensatz um maximal maxMovement viele Stellen.
 * @param dataset     Dataset in which points should be moved
 * @param size        Size of the dataset
 * @param maxMovement Maximale Bewegung die ein Punkt machen kann
 * @return
 */
Coordinates * moveRandomPoint(Coordinates *dataset, int maxMovement) {
    int randomIndex = rand() % inputSize; //bestimmt welcher Punkt verschoben wird

    //bewegt Punkte um Werte zwischen + und - maxMovement
    int nrPossibleValues = maxMovement * 2 + 1;
    int moveX = (rand() % nrPossibleValues) - maxMovement;
    int moveY = (rand() % nrPossibleValues) - maxMovement;
    Coordinates newCoordinates;
    //stellt sicher, dass die neue Position noch innerhalb der Outputgröße ist
    // andernfalls wird neu berechnet
    do {
        newCoordinates = {
                dataset[randomIndex].x + moveX,
                dataset[randomIndex].y + moveY,
        };
    } while(newCoordinates.x >= outputWidth || newCoordinates.y >= outputHeight
        || newCoordinates.x < 0 || newCoordinates.y < 0);
    dataset[randomIndex] = newCoordinates;
    return dataset;
}

Coordinates * perturb(Coordinates dsBefore[], Coordinates *dsNew, Coordinates *targetShape, float temp, int maxMovement){
    while(true){
        //bewegt random Punkte in dsNew
        moveRandomPoint(dsNew, maxMovement);
        //akzeptiert dsNew, wenn es näher an der targetShape ist als dsBefore
        // akzeptiert es auch, wenn die Zufallszahl kleiner als temp ist
        float fitNew = fit(dsNew, targetShape);
        float fitBefore = fit(dsBefore, targetShape);
        float randomNr = (float) rand() / (float) RAND_MAX;
        //TODO: manchmal bleibt das programm iwo ewig hängen, weil es die punkte immer weiter in falsche Richtungen bewegt
        // und fit dadurch einfach nicht passt
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
Coordinates * generateNewPlot(Coordinates *initialDS, Coordinates *currentDS, Coordinates *targetShape) {
    //TODO: iterations hochschalten!
    int iterations = 10000;
    //Coordinates currentDS[size];
    //copy(initialDS, initialDS+size, currentDS);
    for (int i = 0; i<iterations; i++){
        Coordinates testDS[inputSize];
        //seed randomizer
        //TODO: wenn parallelisiert setze id auf omp_get_thread_num()
        int id = 1; //omp_get_thread_num();
        srand((unsigned) time(NULL)+id);
        //setzt testDS = currentDS
        std::copy(currentDS, currentDS+inputSize, testDS);
        //"temperature" starts with 0.4 and goes down until 0 with each iteration
        float maxTemp = 0.4, minTemp = 0.1;
        float temp = (maxTemp - minTemp) * ((iterations-i)/iterations) + minTemp;
        //maximal movement a point makes at once.
        // Starts with movementStart and goes down until movementEnd with each iteration
        //TODO: maxMovement lieber doch nicht so hoch setzen, sonst bewegt sich alles zu schnell zu weit weg
//        float movementStart = 5, movementEnd = 1;
//        float process = ((float)iterations-i)/iterations;
//        float maxMovement = (movementStart - movementEnd) * process + movementEnd;
        int maxMovement = 1;
        //verändert TestDS
        perturb(currentDS, testDS, targetShape, temp, round(maxMovement));
        if (isErrorOk(testDS, initialDS)){
            //setzt currentDS = testDS
            std::copy(testDS, testDS+inputSize, currentDS);
        }
    }
}

int createArrayFromImage(char *image, int width, int height, Coordinates *array){
    //fread( image , width * height * 3,  1, file);
    //zählt/sucht alle nahezu schwarzen Punkte
    //TODO: beim input einlesen iwie dafür sorgen, dass jeder Punkt nur wirklich einmal registriert wird!
    // (hat halt meistens noch gräuliche Randpunkte, die jetzt auch eingelesen werden...)
    int index = 0;
    for (int i=1;i<width * height * 3;i=i+3){
        unsigned char red = image[i];
        unsigned char green = image[i+1];
        unsigned char blue = image[i+2];
        if (red < 200 && green < 200 && blue < 200){
            int currentY = (i / 3 / width);
            int currentX = (i / 3 - currentY * width);
            Coordinates newCoord = {currentX, currentY};
            array[index] = newCoord;
            index++;
        }
    }
    return index;
}

int createArrayFromImageVector(char *image, int width, int height, vector<Coordinates> *vector){
    //fread( image , width * height * 3,  1, file);
    //zählt/sucht alle nahezu schwarzen Punkte
    //TODO: beim input einlesen iwie dafür sorgen, dass jeder Punkt nur wirklich einmal registriert wird!
    // (hat halt meistens noch gräuliche Randpunkte, die jetzt auch eingelesen werden...)
    int index = 0;
    for (int i=1;i<width * height * 3;i=i+3){
        unsigned char red = image[i];
        unsigned char green = image[i+1];
        unsigned char blue = image[i+2];
        if (red < 200 && green < 200 && blue < 200){
            int currentY = (i / 3 / width);
            int currentX = (i / 3 - currentY * width);
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
void exportImage(string fileName, Coordinates *points){
    std::ofstream outputFile(fileName, std::ios::binary);

    // Write the PPM image header
    outputFile << "P6" << std::endl;
    outputFile << outputWidth << " " << outputHeight << std::endl;
    outputFile << 255 << std::endl;

    // Iterate through each pixel of the image
    int index = 0;
    for (int i=0;i<outputWidth * outputHeight * 3;i=i+3){
        int currentY = (i / 3 / outputWidth);
        int currentX = (i / 3 - currentY * outputWidth);
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

//TODO: nur erstmal zum teste, später rauslöschen!
string fileNameInit = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                     "/input/dots_small_12.ppm";          //read input
string fileNameTarget = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                       "/shapes/diagonalStripes_small.ppm";   //read shape
                        //"/empty_5x4_5.ppm"                   //read empty


FileInformation readFile(bool isTargetShape){
    int width, height;
    char *imageData;
    bool incorrectInput = false;
    do {
        //TODO: Anweisungen evtl umformulieren
        string fileName; //später hier rauslöschen und nur unten haben
        if(isTargetShape) {
            cout << "Bitte gib den Pfad zur Zielform an: \n";
            fileName = fileNameTarget;
        } else {
            cout << "Bitte gib den Pfad zur den initialen Daten an: \n";
            fileName = fileNameInit;
        }
        //TODO: lies Pfad zu Bild ein
        //string filename = //eingabe

        //Open the image file
        std::ifstream fileInit(fileName, std::ios::binary);
        // Read the image header
        std::string fileFormat;
        fileInit >> fileFormat;
        fileInit >> width >> height; // >> max_color;

        if (fileFormat == ""){
            cout << "File could not be found! \n";
            incorrectInput = true;
            continue;
        } else if (fileFormat != "P6"){
            cout << "File has wrong format! \n";
            incorrectInput = true;
            continue;
        }
        int image_size = width * height * 3;// Determine the size of the image data
        imageData = new char[image_size];// Create a buffer to hold the image data
        fileInit.read(imageData, image_size);// Read the image data into the buffer
        fileInit.close();// Close the image file
    } while(incorrectInput);
    FileInformation info = {width, height, imageData};
    return info;
}

bool compareByY(const Coordinates &a, const Coordinates &b) {
    return a.y < b.y;
}

#include <vector>

int main() {
    //nur um zu testen, dass OpenMP funktioniert
    //piCalcTest();

    //Liest Eingabe- bzw. Zieldatei ein
    FileInformation inputInfo = readFile(false);
    FileInformation targetInfo = readFile(true);

    vector<Coordinates> inputVector(inputSize);
    vector<Coordinates> targetShape(numberPointsTarget);
    inputSize = createArrayFromImageVector(inputInfo.imageData, inputInfo.width, inputInfo.height, inputVector);
    numberPointsTarget = createArrayFromImageVector(targetInfo.imageData, targetInfo.width, targetInfo.height,
                                                    targetShape);


    Coordinates *initialPointsTmp = new Coordinates[inputInfo.width * inputInfo.height];
    Coordinates *targetShapeTmp = new Coordinates[targetInfo.width * targetInfo.height];
    //TODO: will für inputSize/Target evtl auch lieber Setter haben...?
    //creates arrays containing all those coordinates, where the input images have a dot
    // and counts the number of these dots
    inputSize = createArrayFromImage(inputInfo.imageData, inputInfo.width, inputInfo.height, initialPointsTmp);
    numberPointsTarget = createArrayFromImage(targetInfo.imageData, targetInfo.width, targetInfo.height, targetShapeTmp);

    setOutputWidth(inputInfo.width, targetInfo.width);
    setOutputHeight(inputInfo.height, targetInfo.height);

    //creates array of needed length und copies the points in it
    Coordinates initialPoints[inputSize];
    Coordinates targetShape[numberPointsTarget];
    copy(initialPointsTmp, initialPointsTmp + inputSize, initialPoints);
    copy(targetShapeTmp, targetShapeTmp + numberPointsTarget, targetShape);

    // Release memory from heap after using it
    delete[] inputInfo.imageData, delete[] targetInfo.imageData;
    delete[] initialPointsTmp, delete[] targetShapeTmp;

    Coordinates currentDS[inputSize];
    copy(initialPoints, initialPoints + inputSize, currentDS);

    generateNewPlot(initialPoints, currentDS, targetShape);

    //printet Ergebnis
    for (int i=0; i < inputSize; i++){
        cout << "(" << currentDS[i].x << ", " << currentDS[i].y << "), ";
    }

    // Create the image file
    string fileNameExport = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                      "/output/output.ppm";
    //sortieert currentDS nacch y-Koordinaten
    std::sort(currentDS, currentDS + inputSize, compareByY);
    exportImage(fileNameExport, currentDS);

    return 0;
}
