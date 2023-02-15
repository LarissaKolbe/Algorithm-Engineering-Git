#include "catch.h"
#include "helperFunctions.h"

// roundToNDecimals() Tests
TEST_CASE("Round to 0 decimals - round down", "[correctness]"){
    constexpr int decimals = 0;
    constexpr float value = 13.48972;
    REQUIRE(roundToNDecimals(value, decimals) == 13);
}
TEST_CASE("Round to 0 decimals - round up", "[correctness]"){
    constexpr int decimals = 0;
    constexpr float value = 13.58972;
    REQUIRE(roundToNDecimals(value, decimals) == 14);
}
TEST_CASE("Round to 2 decimals - round down", "[correctness]"){
    constexpr int decimals = 2;
    constexpr float value = 1.123456;
    REQUIRE(roundToNDecimals(value, decimals) == (float)1.12);
}
TEST_CASE("Round to 2 decimals - round up", "[correctness]"){
    constexpr int decimals = 2;
    constexpr float value = 1.56789;
    REQUIRE(roundToNDecimals(value, decimals) == (float)1.57);
}

// getGreaterValue() Tests
TEST_CASE("get greater value - a < b", "[correctness]"){
    constexpr int a = 2, b = 5;
    REQUIRE(getGreaterValue(a,b) == b);
}
TEST_CASE("get greater value - a == b", "[correctness]"){
    constexpr int a = 5, b = 5;
    REQUIRE(getGreaterValue(a,b) == 5);
}

// compareByY() Tests
TEST_CASE("compare by y - same x", "[correctness]"){
    constexpr Coordinate a = {1,2};
    constexpr Coordinate b = {1,3};
    REQUIRE(compareByY(a,b) == true);
}
TEST_CASE("compare by y - same y", "[correctness]"){
    constexpr Coordinate a = {1,2};
    constexpr Coordinate b = {0,2};
    REQUIRE(compareByY(a,b) == false);
}
