
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "datatypes.h"
#include "helperFunctions.h"
#include "exportHelper.h"

using namespace std;

/**
 * Das Interface, über das der User auswählt, wo der Output gespeichert werden soll.
 * Achtung: Es wird nicht geprüft, ob der Pfad tatsächlich valide ist!
 * @return Outputpfad und -name
 */
string readExportPath(){
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
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
        // wählt standard Pfad
        } else if (fileName == "--"){
            fileName = "../data/output/output.ppm";
        // hängt '.ppm' an den Pfad, falls der User das nicht getan hat
        } else if (fileName.substr(fileName.length() - 4) != ".ppm"){
            fileName = fileName.append(".ppm");
        }
        return fileName;
    }
}

/**
 * Rundet alle Werte auf Integer, sortiert den Vektor und löscht doppelte Einträge.
 * @param dataset Koordinatenvektor
 * @return bereinigter Vektor
 */
aligned_vector<Coordinate> cleanDataset(aligned_vector<Coordinate> dataset){
    int size = (int)dataset.size();

    //Rundet alle Werte auf Integer, da Bildkoordinaten nur ganze Zahlen sein können
#pragma omp simd
    for (int i=0; i < size; i++){
        dataset[i].x = round(dataset[i].x);
        dataset[i].y = round(dataset[i].y);
    }

    //sortiert dataset nach y-Koordinaten
    sort(dataset.begin(), dataset.end(), compareByY);

    //löscht doppelte Koordinaten
    // ohne das kommt es zu Fehlern beim Export
    for (int i=0; i < size; i++){
        //vergleicht aktuelles mit nächstem Element und löscht es, wenn sie gleich sind
        if(i+1<size && dataset[i].x == dataset[i+1].x && dataset[i].y == dataset[i+1].y){
            dataset.erase(dataset.begin() + i);
            size -= 1;
            i--;
        }
    }
    return dataset;
}

/**
 * Exportiert ppm-Bild mit schwarzen Pixeln an den in data angegebenen Koordinaten.
 * @param fileName Outputpfad und -name
 * @param data     Koordinaten, an denen ein shcwarzer Punkt sein soll
 * @param maxColor maximaler Farbwert
 * @param height   Höhe des Bildes
 * @param width    Breite des Bildes
 */
void exportImage(const string &fileName, aligned_vector<Coordinate> data, int maxColor, int height, int width){

    //bereinigt die Daten
    data = cleanDataset(data);

    //Bestimmt die Anzahl an Punkten und die Bildgröße
    int dataSize = (int)data.size();
    int imageSize = width * height * 3;

    //initialisiert Outputdatei
    ofstream outputFile(fileName, ios::binary);

    // Erstellt den Dateiheader
    outputFile << "P6" << endl;
    outputFile << width << " " << height << endl;
    outputFile << maxColor << endl;

    int index = 0;
    // Iteriert durch jeden Pixel des Bildes und weißt ihm entweder die Farbe weiß oder schwarz zu
    for (int i=0; i < imageSize; i= i + 3){
        auto currentY = i / 3 / width;
        auto currentX = i / 3 - currentY * width;
        // Färbt Pixel schwarz, wenn er Koordinaten in data entspricht
        // Weil data nicht durchsucht, sondern Schritt für Schritt durchgegangen wird,
        //  müssen die Daten vorher sortiert und bereinigt werden
        if(index < dataSize && (float)currentX == data[index].x && (float)currentY == data[index].y){
            outputFile.put(0);
            outputFile.put(0);
            outputFile.put(0);
            index++;
        // Färbt Pixel weiß, wenn es keine Übereinstimmung gibt
        } else {
            outputFile.put((char)maxColor);
            outputFile.put((char)maxColor);
            outputFile.put((char)maxColor);
        }
    }

    // Schließt die Bilddatei
    outputFile.close();
}
