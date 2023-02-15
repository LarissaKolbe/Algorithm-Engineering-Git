
#include <sstream>
#include <iostream>
#include "catch.h"
#include "configurationHelpers.h"
#include "datatypes.h"

using namespace std;

TEST_CASE("change configurations"){
    //Simuliert Userinput
    stringstream input;
    input << "n "       //verändere Konfigurationen
        << "3 550000 "  //setze iterations auf 250000
        << "2 1.2 "     //setze decimals auf 1.2
        << "5 1.5 "     //setze maxTemp auf 1.5
        << "0 "         //printe Konfigurationen
        << "3 250000 "  //setze iterations auf 250000
        << "4 0.75 "    //setze maxTemp auf 0.75
        << "9 y\n";       //beende
    //setze decimals
    cin.rdbuf(input.rdbuf());

    //ruft die zu testende Funktion auf
    Configurations config = setConfigurations();

    //prüft, ob Header Informationen korrekt gelesen wurden
    REQUIRE(config.accuracy == 0);   //Standardwert
    REQUIRE(config.decimals == 1);   //wurde auf 1.2 gesetzt und sollte durch casten auf 1 gesetzt werden
    REQUIRE(config.iterations != 550000); //Änderung wurde danach wieder überschrieben
    REQUIRE(config.iterations == 250000); //verändert
    REQUIRE(config.maxMovement == 0.75);  //verändert
    REQUIRE(config.maxTemp == 1.5);   //verändert
    REQUIRE(config.minTemp == 0.01);  //Standardwert
}