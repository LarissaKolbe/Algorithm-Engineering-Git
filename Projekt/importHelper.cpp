#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "datatypes.h"
#include "helperFunctions.h"
#include "importHelper.h"

using namespace std;

/**
 * Gibt eine Übersicht der angebotenen Inputbildern aus.
 * Dazu wird die Anzahl an Punkten angezeigt.
 * @param size Bildgröße
 */
void getInputShapeOverview(InputSize size){
    switch (size) {
        case small:
            cout << " [6]  linearer Graph - steigend       (100 Elemente)" << endl
                 << " [7]  linearer Graph - fallend        (100 Elemente)" << endl
                 << " [8]  diagonale Streifen - steigend   (250 Elemente)" << endl
                 << " [9]  diagonale Streifen - fallend    (250 Elemente)" << endl
                 << " [10] vertikale Streifen              (250 Elemente)" << endl
                 << " [11] horizontale Streifen            (250 Elemente)" << endl
                 << " [12] Kreis                           (250 Elemente)" << endl
                 << " [13] Stern                           (250 Elemente)" << endl
                 << " [14] Würfel                          (250 Elemente)" << endl
                 << " [15] Kreuz                           (500 Elemente)" << endl
                 << " [16] Würfel mit Kreuz                (750 Elemente)" << endl;
            break;
        case big:
            cout << " [6]  linearer Graph - steigend       ( 500 Elemente)" << endl
                 << " [7]  linearer Graph - fallend        ( 500 Elemente)" << endl
                 << " [8]  diagonale Streifen - steigend   ( 750 Elemente)" << endl
                 << " [9]  diagonale Streifen - fallend    ( 750 Elemente)" << endl
                 << " [10] vertikale Streifen              ( 500 Elemente)" << endl
                 << " [11] horizontale Streifen            ( 500 Elemente)" << endl
                 << " [12] Kreis                           ( 500 Elemente)" << endl
                 << " [13] Stern                           ( 750 Elemente)" << endl
                 << " [14  Würfel                          ( 750 Elemente)" << endl
                 << " [15] Kreuz                           ( 750 Elemente)" << endl
                 << " [16] Würfel mit Kreuz                (1500 Elemente)" << endl;
            break;
    }
    return;
}

/**
 * Gibt eine Übersicht der angebotenen Zielformen aus.
 * Dazu wird die Anzahl an Punkten angezeigt.
 * @param size Bildgröße
 */
void getTargetShapeOverview(InputSize size){
    switch (size) {
        case small:
            cout << " [1]  linearer Graph - steigend       ( 270 Elemente)" << endl
                 << " [2]  linearer Graph - fallend        ( 270 Elemente)" << endl
                 << " [3]  diagonale Streifen - steigend   ( 512 Elemente)" << endl
                 << " [4]  diagonale Streifen - fallend    ( 512 Elemente)" << endl
                 << " [5]  vertikale Streifen              ( 416 Elemente)" << endl
                 << " [6]  horizontale Streifen            ( 530 Elemente)" << endl
                 << " [7]  Kreis                           ( 719 Elemente)" << endl
                 << " [8]  Stern                           ( 654 Elemente)" << endl
                 << " [9]  Würfel                          ( 546 Elemente)" << endl
                 << " [10] Kreuz                           (1181 Elemente)" << endl
                 << " [11] Würfel mit Kreuz                (3002 Elemente)" << endl;
            break;
        case big:
            cout << " [1]  linearer Graph - steigend       ( 681 Elemente)" << endl
                 << " [2]  linearer Graph - fallend        ( 681 Elemente)" << endl
                 << " [3]  diagonale Streifen - steigend   (1152 Elemente)" << endl
                 << " [4]  diagonale Streifen - fallend    (1152 Elemente)" << endl
                 << " [5]  vertikale Streifen              (1060 Elemente)" << endl
                 << " [6]  horizontale Streifen            (1749 Elemente)" << endl
                 << " [7]  Kreis                           (1321 Elemente)" << endl
                 << " [8]  Stern                           (1797 Elemente)" << endl
                 << " [9]  Würfel                          (2792 Elemente)" << endl
                 << " [10] Kreuz                           (1327 Elemente)" << endl
                 << " [11] Würfel mit Kreuz                (3827 Elemente)" << endl;
            break;
    }
    return;
}

/**
 * Lässt den User aus verschiedenen Inputbildern wählen.
 * @return Dateiname
 */
string chooseFromInputShapes(InputSize size){
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true){
        int answer;
        cout << endl
             << "Es gibt folgende Formen:" << endl
             << " [1]  Punktewolke                    ( 100 Punkte) " << endl
             << " [2]  Punktewolke                    ( 250 Punkte) " << endl
             << " [3]  Punktewolke                    ( 500 Punkte) " << endl
             << " [4]  Punktewolke                    ( 750 Punkte) " << endl
             << " [5]  Punktewolke                    (1000 Punkte) " << endl;
        getInputShapeOverview(size);
        cout << "Bitte gib die Nummer der Form an, die du verwenden möchtest:  ";
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Inputfehler: Bitte schreibe eine Ziffer !!"<< endl;
            continue;
        }
        switch(answer){
            case 1: return "dots_100";
            case 2: return "dots_250";
            case 3: return "dots_500";
            case 4: return "dots_750";
            case 5: return "dots_1000";
            case 6: return "linGraphUp";
            case 7: return "linGraphDown";
            case 8: return "diagStripesUp";
            case 9: return "diagStripesDown";
            case 10: return "verticalStripes";
            case 11: return "horizontStripes";
            case 12: return "circle";
            case 13: return "star";
            case 14: return "cube";
            case 15: return "cross";
            case 16: return "xCube";
            default: cout << endl << "!! Inputfehler: Bitte gibt eine der angegebenen Auswahlwöglichkeiten ein !!" << endl; continue;
        }
    }
}

/**
 * Lässt den User aus verschiedenen Zielformen wählen.
 * @return Dateiname
 */
string chooseFromTargetShapes(InputSize size){
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true) {
        int answer;
        cout << endl << "Es gibt folgende Formen:" << endl;
        getTargetShapeOverview(size);
        cout << "Bitte gib die Nummer der Form an, die du verwenden möchtest:  ";
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Inputfehler: Bitte schreibe eine Ziffer !!" << endl;
            continue;
        }
        //gibt den jeweiligen Dateinamen oder eine Fehlermeldung zurück
        switch(answer){
            case 1: return "linGraphUp";
            case 2: return "linGraphDown";
            case 3: return "diagStripesUp";
            case 4: return "diagStripesDown";
            case 5: return "verticalStripes";
            case 6: return "horizontStripes";
            case 7: return "circle";
            case 8: return "star";
            case 9: return "cube";
            case 10: return "cross";
            case 11: return "xCube";
            default: cout << endl << "!! Inputfehler: Bitte gibt eine der angegebenen Auswahlwöglichkeiten ein. !!" << endl; continue;
        }
    }
}

/**
 * Lässt den User die Größe des Bildes wählen.
 * @return gewählte Größe
 */
InputSize getSize(){
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true) {
        int answer;
        cout << endl
             << "Wie groß soll das Bild sein?" << endl
             << " [1]  255 x 283 " << endl
             << " [2]  500 x 500 " << endl;
        cout << "Bitte gib die Nummer der Form an, die du verwenden möchtest:  ";
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Inputfehler: Bitte schreibe eine Ziffer !!" << endl;
            continue;
        }
        switch(answer){
            case 1: return small;
            case 2: return big;
            default: cout << endl << "!! Inputfehler: Bitte gibt eine der angegebenen Auswahlwöglichkeiten ein. !!" << endl; continue;
        }
    }
}

/**
 * Baut den Pfad zur vom user gewählten Datei zusammen
 * @param type Inputbild oder Zielform
 * @return Pfad zur ausgewählten Datei
 */
string chooseImageFromDefault(InputType type){
    string fileName, answer;
    // setzt Pfad zu Ordner
    switch (type) {
        case initialData: fileName = "../data/input/"; break;
        case target: fileName = "../data/shapes/"; break;
    }
    //Wahl der Bildgröße
    InputSize size = getSize();
    switch (size) {
        case small: fileName = fileName.append("small/"); break;
        case big: fileName = fileName.append("big/"); break;
    }
    // Wahl des Bildes
    switch (type) {
        case initialData: fileName = fileName.append(chooseFromInputShapes(size)); break;
        case target: fileName = fileName.append(chooseFromTargetShapes(size)); break;
    }
    // hängt Dateityp an und returnt Pfad
    return fileName.append(".ppm");
}

/**
 * Lässt den User ein eigenes Bild hochladen.
 * @param type Inputbild oder Zielform
 * @return Pfad zur ausgewählten Datei
 */
string inputFilePath(InputType type){
    string fileName;
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true){
        cout << endl << "Hinweis: Um optimale Ergebnisse zu erzielen, sollten die Eingabedatei und die Zielform möglichst greich Groß sein." << endl;
        switch (type) {
            case initialData:
                cout << "Bitte gib den Pfad zu dem ppm-Bild mit den initialen Daten an:  ";
                break;
            case target:
                cout << "Bitte gib den Pfad zu dem ppm-Bild mit der Zielform an:  ";
                break;
        }
        cin >> fileName;
        if (incorrectInput()) {
            cout << endl << "!! Inputfehler: Bitte gibt einen korrekten Pfad an !!" << endl;
            continue;
        } else if (fileName.substr(fileName.length() - 4) != ".ppm"){
            fileName = fileName.append(".ppm");
        }
        return fileName;
    }
}


/**
 * Liest die Datei mit dem übergebenen Filenamen ein und gibt die enthaltenen Daten zurück.
 * Wenn die Datei nicht gelesen werden kann, wird der User aufgefordert eine neue anzugeben.
 *
 * @return Information über Bildmaße, Pixeldaten und Dateiformat
 */
FileInformation importImage(string fileName, InputType type){
    /** Speichert relevante Informationen zum Bild */
    FileInformation info{};

    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true){
        // Öffnet das Bild
        ifstream file(fileName, ios::binary);

        // Liest das Dateiformat
        file >> info.fileFormat;

        // Bei Fehler mit der Inputdatei wird der User aufgefordert einen neuen Namen anzugeben
        if (info.fileFormat.empty()){
            cout << endl << "!! Inputfehler: Datei konnte nicht gefundeen werden. Bitte überprüfe, ob der Dateiname und Pfad korrekt ist oder wähle eine andere Datei. !!" << endl;
            fileName = inputFilePath(type);
            continue;
            // Datei hat falschen Typ
        } else if (info.fileFormat != "P6" && info.fileFormat != "P3"){
            cout << endl << "!! Inputfehler: Die Datei hat das falsche Format. Bitte wähle ein .ppm-Bild. !!" << endl;
            fileName = inputFilePath(type);
            continue;
        }

        // das ignore und das do-while überspringen Zeilenumbrüche und Zeilen mit Kommentaren
        //  ohne das kann es zu Fehlern beim Einlesen der Daten kommen
        string line;
        file.ignore(256, '\n');
        do{
            getline(file, line);
        } while(line[0] == '#');

        // liest Bildmaße und maxColor ein
        stringstream(line) >> info.width >> info.height;
        getline(file, line);
        stringstream(line) >> info.maxColor;

        // Bestimmt die Bildgröße und legt Buffer für die Bilddaten an
        int imageSize = info.width * info.height * 3;
        info.imageData = new char[imageSize];

        // Liest das Bild in den Buffer
        if (info.fileFormat == "P6") {
            file.read(info.imageData, imageSize);
        } else if (info.fileFormat == "P3") {
            // Liest die ASCII-Werte
            for (int i = 0; i < imageSize; i += 3) {
                int red, green, blue;
                file >> red >> green >> blue;
                info.imageData[i] = static_cast<char>(red);
                info.imageData[i+1] = static_cast<char>(green);
                info.imageData[i+2] = static_cast<char>(blue);
            }
        }
        //schließt die Datei
        file.close();
        return info;
    }
}

/**
 * Das Interface über das der User Dateien hochladen oder
 *  aus den vorgegebenen Dateien wählen kann.
 * @param type Inputbild oder Zielform
 * @return Information über Bildmaße, Pixeldaten und Dateiformat
 */
FileInformation readImageData(InputType type){
    string fileName, answer;
    //läuft solange wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben wird returnt
    while(true) {
        switch (type) {
            case initialData:
                cout << endl << "Möchtest du mit einem vorgefertigten Datensatz starten? [y/n]  ";
                break;
            case target:
                cout << endl << "Möchtest du eine vorgefertigte Zielform verwenden? [y/n]  ";
                break;
        }
        cin >> answer;
        if (incorrectInput()) {
            cout << endl << "!! Inputfehler: Bitte entsccheide doch für ja oder nein !!" << endl;
            continue;
        }
        // wähle vorgefertigtes Bild
        if (answer == "y" || answer == "yes" || answer == "j" || answer == "ja") {
            fileName = chooseImageFromDefault(type);

        // User lädt eigenes Bild hoch
        } else if (answer == "n" || answer == "no" || answer == "nein") {
            fileName = inputFilePath(type);
        }

        //lädt Bild hoch und returnt
        return importImage(fileName, type);
    }
}

/**
 * Erstellt Vektor anhand der übergebenen Bildinformationen.
 * Der Vektor enthält die Koordinaten aller Punkte, die im Bild schwarz waren.
 * @param info Bildinformationen
 * @return Vektor mit Koordinaten
 */
aligned_vector<Coordinates> createVectorFromImage(const char *imageData, int width, int height){
    aligned_vector<Coordinates> vec = {};
    int imageSize = width * height * 3;
    //geht alle Bildpunkte durch und liest ihre Farbwerte ein
    for (int i=0;i<imageSize;i=i+3){
        unsigned char red   = imageData[i];
        unsigned char green = imageData[i+1];
        unsigned char blue  = imageData[i+2];
        // packt schwarze (oder fast schwarze) Bildpunkte in den Vektor
        if (red < 50 && green < 50 && blue < 50){
            auto currentY = i / 3 / width;
            auto currentX = i / 3 - currentY * width;
            vec.push_back({(float)currentX, (float)currentY});
        }
    }
    return vec;
}
