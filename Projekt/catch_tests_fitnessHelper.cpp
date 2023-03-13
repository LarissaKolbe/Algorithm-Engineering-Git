#include "catch.h"
#include "datatypes.h"
#include "fitnessHelper.h"


TEST_CASE("is better fit - no better fit", "[correctness]"){
    constexpr Coordinate p1 = {1.5,0.9};
    constexpr Coordinate p2 = {3.2,3.1};
    aligned_vector<Coordinate> target = {{3.2,4},{5.34,6.19},{7.13,2.965},{5.6,2.7}};

    //check all implementations of this function
    REQUIRE(isBetterFit_Naive(p1, p2, target.data(), target.size()) == false);
    REQUIRE(isBetterFit_Parallel(p1, p2, target.data(), target.size()) == false);
    REQUIRE(isBetterFit_VI256(p1, p2, target.data(), target.size()) == false);
    REQUIRE(isBetterFit_VI128(p1, p2, target.data(), target.size()) == false);
    REQUIRE(isBetterFit_VI256_Par(p1, p2, target.data(), target.size()) == false);
    REQUIRE(isBetterFit_VI128_Par(p1, p2, target.data(), target.size()) == false);
}
