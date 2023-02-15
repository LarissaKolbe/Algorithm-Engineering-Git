
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "datatypes.h"
#include "helperFunctions.h"
#include "exportHelper.h"

using namespace std;

string readExportPath(){
    while(true){
        string fileName;
        cout << endl
             << "Wo und unter welchem Namen soll das Ergebnis gespeichert werden? (Pfad)" << endl
             << "Mit '--' kannst du den Standardpfad wählen. (../data/output/output.ppm) " << endl
             << "Speicherort und -name:  ";
        cin >> fileName;
        if (incorrectInput()) {
            cout << endl << "!! Input error: please type a correct path !!" << endl;
            continue;
        } else if (fileName == "--"){
            fileName = "../data/output/output.ppm";
        //adds '.ppm' if the user didn't write it
        } else if (fileName.substr(fileName.length() - 4) != ".ppm"){
            fileName = fileName.append(".ppm");
        }
        return fileName;
    }
}

/**
 * Rundet alle Werte auf Integer, sortiert den Vektor und löscht doppelte Einträge.
 * @param dataset Inputvektor
 * @return ensprechend veränderter Inputvektor
 */
aligned_vector<Coordinates> cleanDataset(aligned_vector<Coordinates> dataset){
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

/**
 * Exportiert ppm-File mit schwarzen Pixeln an den in data angegebenen Koordinaten.
 * @param fileName
 * @param data
 * @param maxColor Maximal color value
 * @param outputHeight
 * @param outputWidth
 */
void exportImage(const string &fileName, aligned_vector<Coordinates> data, int maxColor, int outputHeight, int outputWidth){

    cout << endl << "Ergebnis: " << endl;

    //clean Data
    data = cleanDataset(data);

    //initialisiert output file
    ofstream outputFile(fileName, ios::binary);

    //TODO: print rauslöschen
    cout << " Alle Punkte (ges):";
    int size = (int)data.size();
    for (int i=0; i < size; i++){
        cout << "(" << data[i].x << ", " << data[i].y << "), ";
    }
    cout << endl << " Alle Punkte (gef):";

    // Write the PPM image header
    outputFile << "P6" << endl;
    outputFile << outputWidth << " " << outputHeight << endl;
    outputFile << maxColor << endl;

    // Iterate through each pixel of the image
    int index = 0;
    for (int i=0; i<outputWidth * outputHeight * 3; i=i+3){
        auto currentY = i / 3 / outputWidth;
        auto currentX = i / 3 - currentY * outputWidth;
        // Write the red, green, and blue values for the pixel
        // black, if coordinates match currentDS, else white
        if(index < size && (float)currentX == data[index].x && (float)currentY == data[index].y){
            //TODO: print rauslöschen
            cout << "(" << data[index].x << ", " << data[index].y << "), ";
            outputFile.put(0);
            outputFile.put(0);
            outputFile.put(0);
            index++;
        } else {
            outputFile.put((char)maxColor);
            outputFile.put((char)maxColor);
            outputFile.put((char)maxColor);
        }
    }
    cout << endl;

    // Close the image file
    outputFile.close();
}
