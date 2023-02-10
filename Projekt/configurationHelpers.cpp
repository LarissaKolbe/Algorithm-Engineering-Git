
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
         << " [2] Anzahl an Nachkommastellen der statistischen Eigenschaften, die berechnet werden. " << endl
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
         "     Sie wird verwendet, um auch schlechtere Schritte zu akzeptieren und so zu vermeiden, "
         "dass das Programm an einer Stelle stecken bleibt. " << endl;
}

/**
 * Funktion zum interaktiven ändern der Konfigurationen.
 * @param conf aktuelle Konfigurationen
 * @return neue Konfigurationen
 */
Configurations changeConfigurations(Configurations conf) {
    printConfigurationOverview(conf);

    //lässt den user beliebig viele Konfigurationen ändern
    while(true) {
        cout << endl << "Please write the number of the input you want to change "
            "or write any other number to return to the program." << endl <<
            "You can see the overview once more by entering 0 (zero)." << endl <<
            "Number:  ";
        int input;
        cin >> input;
        if(incorrectInput()){
            cout << endl << "!! Input error: please type an integer number !!" << endl;
            continue;
        }
        double newValue;
        if (input >= 1  && input <= 6) {
            cout << endl << "Please write the new value:  ";
            cin >> newValue;
            if (incorrectInput()) {
                cout << endl << "!! Input error: please type a number !!" << endl;
                continue;
            }
        }

        switch (input) {
            case 0: printConfigurationOverview(conf); break;
            case 1: conf.accuracy = newValue; break;
            case 2: conf.decimals = newValue; break;
            case 3: conf.iterations = newValue; break;
            case 4: conf.maxMovement = newValue; break;
            case 5: conf.maxTemp = newValue; break;
            case 6: conf.minTemp = newValue; break;
            default:
                string answer;
                cout << endl << "Bist du dir sicher, dass du fertig bist? [y/n]:  ";
                cin >> answer;
                if (incorrectInput()) {
                    cout << endl << "!! Input error !!" << endl;
                    break;
                } else if(answer == "y" || answer == "yes" || answer == "j" || answer == "ja") {
                    return conf;
                }
        }
    }
}

//TODO: evtl Configurationen anbhängig von Bildgröße und Punktanzahl machen?
Configurations setConfigurations(Configurations conf){
    string input;
    cout << endl
         << "Hinweis: Changing the Configurations can worsen the result!" << endl
         << "Do you want to run the program using the standard configurations? (Recommended) [y/n]  ";
    cin >> input;
    if (input == "n" || input == "no" || input == "nein"){
        return changeConfigurations(conf);
    } else {
        return conf;
    }
}