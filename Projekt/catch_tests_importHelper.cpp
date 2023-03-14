
#include <sstream>
#include <iostream>
#include "catch.h"
#include "importHelper.h"
#include "datatypes.h"

using namespace std;

TEST_CASE("import default target image"){
    //Simuliert Userinput
    stringstream input;
    input << "y 3 11\n";
    cin.rdbuf(input.rdbuf());

    //ruft die zu testende Funktion auf
    FileInformation fileInfo = readImageData(target);

    //prüft, ob Header Informationen korrekt gelesen wurden
    REQUIRE(fileInfo.width == 500);
    REQUIRE(fileInfo.height == 500);
    REQUIRE(fileInfo.maxColor == 255);
    REQUIRE(fileInfo.fileFormat == "P6");

    //erstellt einen Vektor anhand des Bildes
    aligned_vector<Coordinate> vector = createVectorFromImage(fileInfo.imageData, fileInfo.width, fileInfo.height);

    //prüft, ob alle Punkte erkannt wurden
    REQUIRE(vector.size() == 3827);
}

TEST_CASE("import initial data using path"){
    //Simuliert Userinput
    stringstream input;
    //first input is incorrect, so user will be prompt for another input
    input << "n ../data/middle/dots_100 ../data/input/middle/dots_100.ppm\n";
    cin.rdbuf(input.rdbuf());

    //ruft die zu testende Funktion auf
    FileInformation fileInfo = readImageData(initialData);

    //prüft, ob Header Informationen korrekt gelesen wurden
    REQUIRE(fileInfo.width == 283);
    REQUIRE(fileInfo.height == 255);
    REQUIRE(fileInfo.maxColor == 255);
    REQUIRE(fileInfo.fileFormat == "P6");

    //erstellt einen Vektor anhand des Bildes
    aligned_vector<Coordinate> vector = createVectorFromImage(fileInfo.imageData, fileInfo.width, fileInfo.height);

    //prüft, ob alle Punkte erkannt wurden
    REQUIRE(vector.size() == 100);
}