#include <iomanip>
#include <iostream>
#include <fstream>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

using namespace std;

struct Coordinates {
    double x;
    double y;
};

struct FileInformation {
    int width;
    int height;
    int maxColor;
    char *imageData;
};

struct statisticalProperties {
    //TODO: füg evtl mehr Eigenschaften hinzu? Aber was?
    double meanX;
    double meanY;
    double varianceX;
    double varianceY;
    double stdDeviationX;
    double stdDeviationY;
};

//TODO: kann man die private oder so machen?
//accuracy ... wie genau müssen die Werte übereinstimmen bzw wie sehr dürfeen sie abweichen
//decimals ... wie viele Nachkommastellen werden berücksichtigt
//shiftValues ... shiftet die Werte im Datensatz um den angegebenen Betrag
int outputWidth, outputHeight, inputSize, numberPointsTarget, accuracy = 1, decimals = 0, shiftValues = 10;
double maxX, maxY;

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

bool compareByY(const Coordinates &a, const Coordinates &b) {
    if(a.y == b.y){
        return a.x < b.x;
    } else {
        return a.y < b.y;
    }
}

/**
 * Rundet den übergebenen Wert auf die angegebene Anzahl an Nachkommastellen.
 * Standard: keine Nachkommastellen
 * @param value zu rundender Wert
 * @param decimals Anzahl an Nachkommastellen
 * @return gerundeten Wert
 */
float roundValue(float value, int decimals){
    //TODO: schöner machen
    float roundingValue = pow(10, decimals);
    value = round(value * roundingValue) / roundingValue;
    return value;
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
    //TODO: wirkt so als wär das nicht parallel...?
#pragma omp parallel for reduction(+: distanceMod, distancePrev)
    //TODO: dafür sorgen, dass die Punkte sich wirklich der Form annähern...
    // nochmal im Papercode kucken, wie die das da geschrieben haben?
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
            if (distMod < distMinMod) {
                distMinMod = distMod;
            }
            if (distPrev < distMinPrev){
                distMinPrev = distPrev;
            }
        }
        distanceMod += distMinMod;
        distancePrev += distMinPrev;
    }
    return distanceMod < distancePrev;
}

/**
 * Berechnet die statistischen Eigenschaften des übergebenen Datensatzes.
 * @param dataset
 * @return
 */
statisticalProperties calculateStatisticalProperties(vector<Coordinates> dataset){
    statisticalProperties properties = {0.0, 0.0, 0.0, 0.0};
#pragma omp parallel for reduction(+: properties.meanX, properties.meanY)
    for(int i=0; i<inputSize; i++){
        properties.meanX += dataset[i].x;
        properties.meanY += dataset[i].y;
    }
    properties.meanX = roundValue(properties.meanX / inputSize, decimals);
    properties.meanY = roundValue(properties.meanY / inputSize, decimals);

#pragma omp parallel for reduction(+: properties.varianceX, properties.varianceY)
    for(int i=0; i<inputSize; i++){
        properties.varianceX += pow(dataset[i].x - properties.meanX, 2);
        properties.varianceY += pow(dataset[i].y - properties.meanY, 2);
    }
    properties.varianceX = properties.varianceX / (inputSize-1);
    properties.varianceY = properties.varianceY / (inputSize-1);
    properties.stdDeviationX = roundValue(sqrt(properties.varianceX), decimals);
    properties.stdDeviationY = roundValue(sqrt(properties.varianceY), decimals);
    properties.varianceX = roundValue(properties.varianceX, decimals);
    properties.varianceY = roundValue(properties.varianceY, decimals);
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
bool isErrorOk(vector<Coordinates> newDS, statisticalProperties initialProps, double allowedDifference) {
    statisticalProperties newProps = calculateStatisticalProperties(newDS);
    return abs(newProps.meanX - initialProps.meanX) <= allowedDifference
        && abs(newProps.meanY - initialProps.meanY) <= allowedDifference
//        && abs(newProps.varianceX - initialProps.varianceX) <= allowedDifference
//        && abs(newProps.varianceY - initialProps.varianceY) <= allowedDifference;
        && abs(newProps.stdDeviationX - initialProps.stdDeviationX) <= allowedDifference
        && abs(newProps.stdDeviationY - initialProps.stdDeviationY) <= allowedDifference;
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
vector<Coordinates> moveRandomPoint(vector<Coordinates> dataset, double maxMovement) {
    //TODO: vlt besser random zahlen wie folgt bestimmen?
    //  #include <random>
    std::random_device rd;  // seed for the random number engine
    std::mt19937 gen(rd());  // Mersenne Twister random number engine
    std::uniform_real_distribution<> dis1(0, inputSize-1);
    std::uniform_real_distribution<> dis(-maxMovement, maxMovement);  // define the range of the float numbers

    int randomIndex = dis1(gen); //rand() % inputSize; //bestimmt welcher Punkt verschoben wird

    //bewegt Punkte um Werte zwischen + und - maxMovement
//    int nrPossibleValues = maxMovement * 2 + 1;
    Coordinates newCoordinates;
    //do-while stellt sicher, dass die neue Position noch innerhalb der Outputgröße ist
    // andernfalls wird neu berechnet
    do {
//        int moveX = (rand() % nrPossibleValues) - maxMovement;
//        int moveY = (rand() % nrPossibleValues) - maxMovement;
        double moveX = dis(gen);
        double moveY = dis(gen); // erstellt die random Zahl
//        double moveX = (rand() / (RAND_MAX / 2.0)) - maxMovement;
//        double moveY = (rand() / (RAND_MAX / 2.0)) - maxMovement;
        newCoordinates = {
                dataset[randomIndex].x + moveX,
                dataset[randomIndex].y + moveY,
        };
    } while(round(newCoordinates.x) >= maxX || round(newCoordinates.y) >= maxY
        || newCoordinates.x < 0 || newCoordinates.y < 0);
    dataset[randomIndex] = newCoordinates;
    return dataset;
}

//TODO: später wieder rauslöscchen, sind nur für mich als info
int errorWasOkay = 0, fitWasOkay = 0;

vector<Coordinates> perturb(vector<Coordinates> dsBefore, vector<Coordinates> targetShape, double temp, double maxMovement){
    vector<Coordinates> dsNew(dsBefore);
    //will move points until if-condition is fulfilled
    while(true){
        //bewegt random Punkte in dsNew
        dsNew = moveRandomPoint(dsNew, maxMovement);
        //akzeptiert dsNew, wenn es näher an der targetShape ist als dsBefore
        // akzeptiert es auch, wenn die Zufallszahl kleiner als temp ist
//        double fitNew = fit(dsNew, targetShape);
//        double fitBefore = fit(dsBefore, targetShape);
        double randomNr = (double) rand() / (double) RAND_MAX;
        //TODO: manchmal bleibt das programm iwo ewig hängen, weil es die punkte immer weiter in falsche Richtungen bewegt
        // und fit dadurch einfach nicht passt
        if(isBetterFit(dsNew, dsBefore, targetShape)  //fitNew < fitBefore
            || randomNr < temp){
            fitWasOkay++;
            return dsNew;
        }
    }
}

//TODO: nur zum debuggen
void exportImageTmp(int number, vector<Coordinates> points, int maxColor){
    vector<Coordinates> dataset(points);
    int size = dataset.size();
    //round all entries to integer and reset the shift
    //TODO: hier könnte man loop unrolling machen, aber denk nicht das sich das wirklich lohnt...
#pragma omp parallel for
    for (int i=0; i < size; i++){
        dataset[i].x = round(dataset[i].x * shiftValues);
        dataset[i].y = round(dataset[i].y * shiftValues);
    }
    //sortiert currentDS nach y-Koordinaten
    std::sort(dataset.begin(), dataset.end(), compareByY);

    //deletes duplicate entries
    //TODO: hier könnte man auch parallel machen, aber dann muss man noch drauf achten, das "size" aktiell zu halten?
    // und elementreihenfolge etc können sicch ändern also vmtl eher riskant?
//#pragma omp parallel for
    for (int i=0; i < size; i++){
        if(i+1<size && dataset[i].x == dataset[i+1].x && dataset[i].y == dataset[i+1].y){
            cout << "Erased double Point: (" << dataset[i+1].x << ", " << dataset[i+1].y << ") \n";
            dataset.erase(dataset.begin() + i);
            size -= 1;
        }
    }
    string fileNameExport = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                            "/output/perIteration/output" + to_string(number) + ".ppm";
    std::ofstream outputFile(fileNameExport, std::ios::binary);

    // Write the PPM image header
    outputFile << "P6" << std::endl;
    outputFile << outputWidth << " " << outputHeight << std::endl;
    outputFile << maxColor << std::endl;

    // Iterate through each pixel of the image
    int index = 0;
    //TODO:: könnte man vlt paralellisieren, aber dann kann man probleme mit 'index' kriegen...
//#pragma omp parallel for
    for (int i=0;i<outputWidth * outputHeight * 3;i=i+3){
        int currentY = (i / 3 / outputWidth);
        int currentX = (i / 3 - currentY * outputWidth);
        // Write the red, green, and blue values for the pixel
        // black, if coordinates match currentDS, else white
//        if((int)points[index].x != round(points[index].x))
//            cout << "(int)points[index].x = " << (int)points[index].x << "; round(points[index].x) = " << round(points[index].x) << "\n";
        if(currentX == dataset[index].x && currentY == dataset[index].y){
            cout << "(" << dataset[index].x << ", " << dataset[index].y << "), ";
            outputFile.put(0);
            outputFile.put(0);
            outputFile.put(0);
            index++;
        } else {
            outputFile.put(maxColor);
            outputFile.put(maxColor);
            outputFile.put(maxColor);
        }
    }
    // Close the image file
    outputFile.close();
}

/**
 * Wandelt die Punkte aus dem Eingabearray um.
 * Der Ausgabearray erfüllt die gleichen statistischen Eigenschaften.
 * @param initialDS Eingabearray (Punktewolke)
 * @param currentDS Kopie des Eingabearrays. Wird innerhalb der Funktion geändert.
 * @param size Größe des Eingabearray
 * @return Ausgabearray der gleichen statistischen Eigenschaften erfüllt
 */
vector<Coordinates> generateNewPlot(vector<Coordinates> initialDS, vector<Coordinates> targetShape, statisticalProperties initialProperties, int iterations) {
    vector<Coordinates> currentDS(initialDS);
    for (int i = 0; i<iterations; i++){
        //seed randomizer
        //TODO: wenn parallelisiert setze id auf omp_get_thread_num()
        int id = omp_get_thread_num();
        srand((unsigned) time(NULL)+id);

        if(i % 1000 == 0) {
            cout << "Iteration: " << i << "\n";
            exportImageTmp(i, currentDS, 255);
        }

        //"temperature" starts with 0.4 and goes down until 0 with each iteration
        double maxTemp = 0.4, minTemp = 0.1;
        double temp = (maxTemp - minTemp) * ((iterations-i)/iterations) + minTemp;

        //maximal movement a point makes at once.
        // Starts with movementStart and goes down until movementEnd with each iteration
        //TODO: maxMovement lieber doch nicht so hoch setzen, sonst bewegt sich alles zu schnell zu weit weg
//        double movementStart = 5, movementEnd = 1;
//        double process = ((double)iterations-i)/iterations;
//        double maxMovement = (movementStart - movementEnd) * process + movementEnd;
        double maxMovement = 0.1;

        vector<Coordinates> testDS = perturb(currentDS, targetShape, temp, maxMovement);
        if (isErrorOk(testDS, initialProperties, accuracy)){
            currentDS = testDS;
            errorWasOkay++;
        }
    }
    return currentDS;
}

vector<Coordinates> createArrayFromImageVector(FileInformation info){
    vector<Coordinates> vector = {};
    //zählt/sucht alle nahezu schwarzen Punkte
    //TODO: beim input einlesen iwie dafür sorgen, dass jeder Punkt nur wirklich einmal registriert wird!
    // (hat halt meistens noch gräuliche Randpunkte, die jetzt auch eingelesen werden...)
#pragma omp parallel for
    for (int i=1;i<info.width * info.height * 3;i=i+3){
        unsigned char red = info.imageData[i];
        unsigned char green = info.imageData[i+1];
        unsigned char blue = info.imageData[i+2];
        if (red < 150 && green < 150 && blue < 150){
            double currentY = (i / 3 / info.width);
            double currentX = (i / 3 - currentY * info.width);
            currentY = currentY / shiftValues;
            currentX = currentX / shiftValues;
            Coordinates newCoord = {currentX, currentY};
            vector.push_back(newCoord);
        }
    }
    return vector;
}

vector<Coordinates> cleanDataset(vector<Coordinates> dataset){
    int size = dataset.size();
    //round all entries to integer and reset the shift
    //TODO: hier könnte man loop unrolling machen, aber denk nicht das sich das wirklich lohnt...
#pragma omp parallel for
    for (int i=0; i < size; i++){
        dataset[i].x = round(dataset[i].x * shiftValues);
        dataset[i].y = round(dataset[i].y * shiftValues);
    }
    //sortiert currentDS nach y-Koordinaten
    std::sort(dataset.begin(), dataset.end(), compareByY);

    //deletes duplicate entries
    //TODO: hier könnte man auch parallel machen, aber dann muss man noch drauf achten, das "size" aktiell zu halten?
    // und elementreihenfolge etc können sicch ändern also vmtl eher riskant?
//#pragma omp parallel for
    for (int i=0; i < size; i++){
        if(i+1<size && dataset[i].x == dataset[i+1].x && dataset[i].y == dataset[i+1].y){
            cout << "Erased double Point: (" << dataset[i+1].x << ", " << dataset[i+1].y << ") \n";
            dataset.erase(dataset.begin() + i);
            size -= 1;
        }
    }
    return dataset;
}

/**
 * Exportiert ppm-File mit schwarzen Pixeln an den in points angegebenen Koordinaten.
 * Code von ChatGPT
 * @param fileName
 * @param points
 * @param maxColor Maximal color value
 */
void exportImage(string fileName, vector<Coordinates> points, int maxColor){
    std::ofstream outputFile(fileName, std::ios::binary);

    // Write the PPM image header
    outputFile << "P6" << std::endl;
    outputFile << outputWidth << " " << outputHeight << std::endl;
    outputFile << maxColor << std::endl;

    // Iterate through each pixel of the image
    int index = 0;
    //TODO:: könnte man vlt paralellisieren, aber dann kann man probleme mit 'index' kriegen...
//#pragma omp parallel for
    for (int i=0;i<outputWidth * outputHeight * 3;i=i+3){
        int currentY = (i / 3 / outputWidth);
        int currentX = (i / 3 - currentY * outputWidth);
        // Write the red, green, and blue values for the pixel
        // black, if coordinates match currentDS, else white
//        if((int)points[index].x != round(points[index].x))
//            cout << "(int)points[index].x = " << (int)points[index].x << "; round(points[index].x) = " << round(points[index].x) << "\n";
        //TODO: passiert immer noch manchmal, dass er iwann einfach abbricht und nicht alle Punkte am Ende rauskommen
        if(currentX == points[index].x && currentY == points[index].y){
            cout << "(" << points[index].x << ", " << points[index].y << "), ";
            outputFile.put(0);
            outputFile.put(0);
            outputFile.put(0);
            index++;
        } else {
            outputFile.put(maxColor);
            outputFile.put(maxColor);
            outputFile.put(maxColor);
        }
    }
    // Close the image file
    outputFile.close();
}

/**
 * Asks the user for a filename, reads the file and returns relevant data.
 * The file has to be .ppm format.
 * If no file is found the user will have to give a new filename.
 *
 * @return Information about width, height and data from the file
 */
 //TODO: fileName wird später nicht übergeben, ist jetzt nur zum testen temporär!!!
FileInformation readFile(string fileName){
    int width, height, maxColor;
    char *imageData;
    bool incorrectInput = false;
    do {
        //TODO: lies Pfad zu Bild ein
        // string filename = //input()

        //Open the image file
        std::ifstream fileInit(fileName, std::ios::binary);
        // Read the image header
        std::string fileFormat;
        fileInit >> fileFormat;
        fileInit >> width >> height >> maxColor;

        //TODO: Prints anpassen (englisch oder deutsch?)
        if (fileFormat == ""){
            cout << "File could not be found! \n Please check if the filename is correct or choose a different file. ";
            cout << "Bitte gib einen anderen Dateinamen an: ";
            incorrectInput = true;
            continue;
        } else if (fileFormat != "P6"){
            cout << "File has wrong format! \n Please choose a .ppm-image.";
            cout << "Bitte gib einen anderen Dateinamen an: ";
            incorrectInput = true;
            continue;
        }
        int image_size = width * height * 3;// Determine the size of the image data
        imageData = new char[image_size];// Create a buffer to hold the image data
        fileInit.read(imageData, image_size);// Read the image data into the buffer
        fileInit.close();// Close the image file
    } while(incorrectInput);
    FileInformation info = {width, height, maxColor, imageData};
    return info;
}


//TODO: nur erstmal zum testen, weils noch keine Eingabe gibt. Später rauslöschen!
string fileNameInit = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                      "/input/dots_small_168.ppm";          //read input
//                     "/input/circle_125.ppm";
string fileNameTarget = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
//                        "/shapes/diagonalStripes_small.ppm";   //read shape
//                       "/shapes/circle_small.ppm";   //read shape
                        "/shapes/star_small.ppm";   //read shape
//                        "/empty_5x4_5.ppm"                   //read empty

int main() {
    //nur um zu testen, dass OpenMP funktioniert
    //piCalcTest();

    //Liest Eingabe- bzw. Zieldatei ein
    //TODO: Anweisungen evtl umformulieren
    cout << "Bitte gib den Pfad zur den initialen Daten an: \n";
    FileInformation inputInfo = readFile(fileNameInit);
    cout << "Bitte gib den Pfad zur Zielform an: \n";
    FileInformation targetInfo = readFile(fileNameTarget);

    //TODO: 1x alle wichtigen Variabeln (temp, maxMovement, iterations, etc...) anzeigen und
    // user die Möglicchkeit geben die bei Bedarf zu ändern

    vector<Coordinates> inputVector = createArrayFromImageVector(inputInfo);
    vector<Coordinates> targetShape = createArrayFromImageVector(targetInfo);

    inputSize = inputVector.size();
    numberPointsTarget = targetShape.size();

    setOutputWidth(inputInfo.width, targetInfo.width);
    setOutputHeight(inputInfo.height, targetInfo.height);

    maxX = (double)outputWidth / (double)shiftValues;
    maxY = (double)outputHeight / (double)shiftValues;

    // Release memory from heap after using it
    delete[] inputInfo.imageData, delete[] targetInfo.imageData;

    statisticalProperties initialProperties = calculateStatisticalProperties(inputVector);
    //TODO: iterations hochschalten!
    int iterations = 100000;
    vector<Coordinates> result = generateNewPlot(inputVector, targetShape, initialProperties, iterations);

    //printet Ergebnis
    for (int i=0; i < inputSize; i++){
        cout << "(" << result[i].x << ", " << result[i].y << "), ";
    }

    // Create the image file
    string fileNameExport = "/Users/larissa/Desktop/Uni/Master/1.Sem_WS22:23/Algorithm Engineering/Algorithm-Engineering-Git/Projekt/data"
                      "/output/output.ppm";

    result = cleanDataset(result);

    cout << " \n \nDifferences: ";
    int nr = 0;
    for (int i=0; i < inputSize; i++){
        if(result[i].x != inputVector[i].x*shiftValues && result[i].y != inputVector[i].y*shiftValues) {
            cout << "  index: " << i << "; Input: (" << inputVector[i].x*shiftValues << ", " << inputVector[i].y*shiftValues
                 << "), Result: (" << result[i].x << ", " << result[i].y << ") \n";
            nr++;
        }
    }
    cout << "Number of Differences: " << nr << "\n";

    cout << "Alle Punkte ohne duplicate: \n";
    int size = result.size();
    for (int i=0; i < size; i++){
        cout << "(" << result[i].x << ", " << result[i].y << "), ";
    }

    cout << "\nAlle gefundenen: \n";
    exportImage(fileNameExport, result, inputInfo.maxColor);

    statisticalProperties resultProps = calculateStatisticalProperties(result);
    cout << "\nResult Properties:  " << resultProps.meanY << ", " << resultProps.meanX << ", " << resultProps.stdDeviationY << ", " << resultProps.stdDeviationX << "\n";
    cout << "Initial Properties: " << initialProperties.meanY*shiftValues << ", " << initialProperties.meanX*shiftValues << ", " << initialProperties.stdDeviationY*shiftValues << ", " << initialProperties.stdDeviationX*shiftValues << "\n";

    cout << "errorWasOkay = " << errorWasOkay << ", fitWasOkay = " << fitWasOkay;

    return 0;
}
