//optimierungen zum verwenden von tests mit catch2
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE // für schnellere recompilation

#include "catch.h"

TEST_CASE("testname", "[test tag]"){
    //teste hier code
    constexpr int x = 1;
    constexpr int y = 2;
    //expression im REQUIRE muss true zurückgeben, sonst schlägt der test fehl
    REQUIRE(x<y);
    //hier muss die expression false sein
    REQUIRE_FALSE(x>=y);
}