
#include <string>
#include <iostream>
#include "datatypes.h"
#include "helperFunctions.h"
#include "configurationHelpers.h"

using namespace std;

/**
 * Zeigt die aktuellen Konfigurationen inklusive kurzen Erklärungen an.
 * @param conf aktuelle Konfigurationen
 */
void printConfigurationOverview(Configurations conf){
    cout << endl
         << "Konfigurationen: " << endl
         << " [1] Erlaubte Abweichung der statistischen Eigenschaften. " << endl
         << "   - Standard: " << conf.accuracy << endl
         << " [2] Bestimmt, wie viele Nachkommestellen betrachtet werden sollen. " << endl
         << "   - Standard: " << conf.decimals << endl
         << " [3] Anzahl an Iterationen, die das Programm durchläuft. " << endl
         << "   - Standard: " << conf.iterations << endl
         << " [4] Maximale Bewegung, die ein Punkt in einer Iteration machen kann. " << endl
         << "   - Standard: " << conf.maxMovement << endl
         << " [5] Maximale Temperatur (*)." << endl
         << "   - Standard: " << conf.maxTemp << endl
         << " [6] Minimale Temperatur (*). " << endl
         << "   - Standard: " << conf.minTemp << endl
         //TODO: Erklärung zu temp verbessern
         << " (*) Temperatur startet beim maximalen Wert und wird mit Fortschreiten des Programms kleiner, "
            "bis der minimale Wert erreicht wird. " << endl <<
         "     Sie wird verwendet, um auch Punktbewegungen zu akzeptieren, die nicht Zielführend sind und so zu vermeiden, "
         "dass das Programm an einer Stelle stecken bleibt. " << endl;
}

/**
 * Userinterface zum Einzusehen und Verändern der Konfigurationen.
 * @param conf aktuelle Konfigurationen
 * @return neue Konfigurationen
 */
Configurations changeConfigurations(Configurations conf) {
    //Zeigt zu Beginn aktuelle Konfigurationen an
    printConfigurationOverview(conf);

    //lässt den User beliebig viele Konfigurationen ändern
    while(true) {
        cout << endl << "Bitte gib die Nummer der Konfiguration an, die du ändern möchtest, "
            "oder schreibe eine andere Zahl, um zum Programm zurückzukehren." << endl <<
            "Mit 0 (Null) kannst du dir die Überesicht erneut anzeigen lassen." << endl <<
            "Nummer:  ";
        int input;
        cin >> input;
        if(incorrectInput()){
            cout << endl << "!! Inputfehler: Bitte schreibe eine Ziffer !!" << endl;
            continue;
        }
        //Eingabe des neuen Wertes
        double newValue;
        if (input >= 1  && input <= 6) {
            cout << endl << "Bitte gib den neuen Wert an:  ";
            cin >> newValue;
            if (incorrectInput()) {
                cout << endl << "!! Inputfehler: Bitte schreibe eine Ziffer !!" << endl;
                continue;
            }
        }
        switch (input) {
            //Zeigt aktuelle Konfigurationen an
            case 0: printConfigurationOverview(conf); break;
            //verändert entsprechenden Wert
            case 1: conf.accuracy = newValue; break;
            case 2: conf.decimals = (int)newValue; break;
            case 3: conf.iterations = (int)newValue; break;
            case 4: conf.maxMovement = (float)newValue; break;
            case 5: conf.maxTemp = newValue; break;
            case 6: conf.minTemp = newValue; break;
            //returnt neue Konfigurationen und kehrt zurück zum Hauptprogramm
            default:
                string answer;
                cout << endl << "Bist du dir sicher, dass du fertig bist? [y/n]:  ";
                cin >> answer;
                if (incorrectInput()) {
                    cout << endl << "!! Inputfehler !!" << endl;
                    break;
                } else if(answer == "y" || answer == "yes" || answer == "j" || answer == "ja") {
                    return conf;
                }
        }
    }
}

/**
 * Userinterface zum Einzusehen und Verändern der Konfigurationen.
 * @return Standard- oder vom User veränderte Konfigurationen
 */
Configurations setConfigurations(){
    Configurations conf{};
    string input;
    //läuft solange, wie falsche Eingaben gemacht werden
    // bei korrekten Eingaben geht es weiter
    while(true) {
        cout << endl
             << "!! Hinweis: Die optimalen Konfigurationen hängen von der Größe des Bildes und der Daten ab und können bei Bedarf hier angepasst werden !!"
             << endl
             << "Möchtest du das Programm mit den Standardkonfigurationen ausführen? [y/n]  ";
        cin >> input;
        if(incorrectInput()){
            cout << endl << "!! Inputfehler: Bitte schreibe eine Ziffer !!" << endl;
            continue;
        // User ändert Konfigurationen
        } else if (input == "n" || input == "no" || input == "nein") {
            return changeConfigurations(conf);
        // User behält Standardkonfigurationen bei
        } else if (input == "y" || input == "yes" || input == "j" || input == "ja") {
            return conf;
        }
    }
}