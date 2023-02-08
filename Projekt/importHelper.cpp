#include <vector>
#include <fstream>
#include <iostream>
#include "datatypes.h"
#include "importHelper.h"
#include "helperFunctions.h"
//#include "aligned_allocator.h"
//
//template<class T>
//using aligned_vector = std::vector<T, alligned_allocator<T, 64>>;

using namespace std;

string chooseFromInputShapes(){
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true){
        int answer;
        cout << endl
             << "Es gibt folgende Formen:" << endl
             << " [1]  Punktewolke (12 Punkte) " << endl
             << " [2]  Punktewolke (168 Punkte) " << endl
             << " [3]  Punktewolke (379 Punkte) " << endl
             << " [4]  Kreis (125 Punkte) " << endl
             << " [5]  Stern (194 Punkte) " << endl
             << " [6]  diagonale Streifen (fallend) (112 Punkte) " << endl
             << "Bitte gib die Nummer der Form an, die du verwenden möchtest:  ";
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Input error: please type a number !!"<< endl;
            continue;
        }
        switch(answer){
            case 1: return "dots_12";
            case 2: return "dots_168";
            case 3: return "dots_379";
            case 4: return "circle_125";
            case 5: return "star_194";
            case 6: return "stripes_112";
            default: cout << endl << "!! Bitte gibt eine der angegebenen Auswahlwöglichkeiten ein. !!" << endl; continue;
        }
    }
}

string chooseFromTargetShapes(){
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true) {
        int answer;
        cout << endl
             << "Es gibt folgende Formen:" << endl
             << " [1]  Kreis " << endl
             << " [2]  Stern " << endl
             << " [3]  Würfel " << endl
             << " [4]  Würfel mit Kreuz " << endl
             << " [5]  Kreuz " << endl
             << " [6]  linearer Graph (steigend) " << endl
             << " [7]  linearer Graph (fallend)" << endl
             << " [8]  diagonale Streifen (steigend) " << endl
             << " [9]  diagonale Streifen (fallend) " << endl
             << " [10] vertikale Streifen " << endl
             << " [11] horizontale Streifen " << endl
             << "Bitte gib die Nummer der Form an, die du verwenden möchtest:  ";
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Input error: please type a number !!" << endl;
            continue;
        }
        switch(answer){
            case 1: return "circle";
            case 2: return "star";
            case 3: return "cube";
            case 4: return "xCube";
            case 5: return "cross";
            case 6: return "linGraphUp";
            case 7: return "linGraphDown";
            case 8: return "diagonalStripesUp";
            case 9: return "diagonalStripesDown";
            case 10: return "vertStripes";
            case 11: return "horiStripes";
            default: cout << endl << "!! Bitte gibt eine der angegebenen Auswahlwöglichkeiten ein. !!" << endl; continue;
        }
    }
}

/**
 * Lässt den User die Größe des Bildes wählen
 * @return string zur Größenangabe im Dateipfad
 */
string chooseSize(){
    while(true) {
        int answer;
        cout << endl
             << "Wie groß soll die Datei sein?" << endl
             << "Hinweis: Die Zielform sollte eine ähnliche Größe haben, wie die Eingabedaten!" << endl
             << " [1]  klein (283 x 255) " << endl
             << " [2]  mittel (nicht verfügbar)" << endl
             << " [3]  groß  (nicht verfügbar)" << endl
             << "Bitte gib die Nummer der Größe an, die du verwenden möchtest:  ";
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Input error: please type a number !!" << endl;
            continue;
        }
        switch (answer) {
            case 1: return "_small";
            case 2:
                cout << endl << "!! Diese Größe ist aktuell nicht verfügbar. Bitte wähle eine andere. !!" << endl;
                continue;
//                return "_middle";
            case 3:
                cout << endl << "!! Diese Größe ist aktuell nicht verfügbar. Bitte wähle eine andere. !!" << endl;
                continue;
//                return "_big";
            default:
                cout << endl << "!! Bitte gibt eine der angegebenen Auswahlwöglichkeiten ein. !!" << endl;
                continue;
        }
    }
}

/**
 * Baut den Pfad zur vom user gewählten Datei zusammen
 * @param type
 * @return Pfad zur ausgewählten Datei
 */
string chooseImageFromDefault(InputType type){
    string fileName, answer;
    // setzt Pfad zu Ordner
    switch (type) {
        case initialData: fileName = "../data/input/"; break;
        case targetShape: fileName = "../data/shapes/"; break;
    }
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true) {
        // Wahl des Bildes
        switch (type) {
            case initialData: fileName = fileName.append(chooseFromInputShapes()); break;
            case targetShape: fileName = fileName.append(chooseFromTargetShapes()); break;
        }
        // Wahl der Bildgröße
        fileName = fileName.append(chooseSize());
        return fileName.append(".ppm");
    }
}

string inputFilePath(InputType type){
    string fileName;
    bool incorrectFileName;
    do {
        incorrectFileName = false;
        cout << endl << "Hinweis: Um optimale Ergebnisse zu erzielen, sollten die Eingabedatei und die Zielform möglichst greich Groß sein." << endl;
        switch (type) {
            case initialData:
                cout << "Bitte gib den Pfad zu dem ppm-Bild mit den initialen Daten an:  ";
                break;
            case targetShape:
                cout << "Bitte gib den Pfad zu dem ppm-Bild mit der Zielform an:  ";
                break;
        }
        cin >> fileName;
        if (incorrectInput()) {
            cout << endl << "!! Input error: please type a correct path !!" << endl;
            incorrectFileName = true;
        }
    } while(incorrectFileName);
    return fileName;
}


/**
 * Asks the user for a filename, reads the file and returns relevant data.
 * The file has to be .ppm format.
 * If no file is found the user will have to give a new filename.
 *
 * @return Information about width, height and data from the file
 */
FileInformation importImage(string fileName, InputType type){
    /** Will store relevant information about the image */
    FileInformation info;

    bool incorrectInput;
    do {
        incorrectInput = false;

        //Open the image file
        ifstream fileInit(fileName, ios::binary);

        // Read the image header
        string fileFormat;
        fileInit >> fileFormat;
        fileInit >> info.width >> info.height >> info.maxColor;

        //Bei Fehler mit der Inputdatei wird der User aufgefordert einen neuen namen anzugeben
        if (fileFormat == ""){
            cout << endl << "!! File could not be found! Please check if the filename is correct or choose a different file. !!" << endl;
            fileName = inputFilePath(type);
            incorrectInput = true;
            continue;
        } else if (fileFormat != "P6"){
            //TODO: vlt noch darauf hinweise, dass auch nur bestimmte Art von ppm-Bild akzeptiert wird
            cout << endl << "!! File has wrong format! Please choose a .ppm-image. !!" << endl;
            fileName = inputFilePath(type);
            incorrectInput = true;
            continue;
        }

        int image_size = info.width * info.height * 3;// Determine the size of the image data
        info.imageData = new char[image_size];// Create a buffer to hold the image data
        fileInit.read(info.imageData, image_size);// Read the image data into the buffer
        fileInit.close();// Close the image file
    } while(incorrectInput);

    return info;
}


FileInformation readTargetData(InputType type){
    string fileName, answer;
    switch (type) {
        case initialData:
            cout << endl << "Möchtest du mit einem vorgefertigten Datensatz starten? [y/n]  ";
            break;
        case targetShape:
            cout << endl << "Möchtest du eine vorgefertigte Zielform nehmen? [y/n]  ";
            break;
    }
    cin >> answer;
    // wähle vorgefertigtes Bild
    if(answer == "y" || answer == "yes" || answer == "j" || answer == "ja"){
        fileName = chooseImageFromDefault(type);

    // User lädt eigenes Bild hoch
    } else {
        fileName = inputFilePath(type);
    }
    return importImage(fileName, type);
}
