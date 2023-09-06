#include "catch2/catch_test_macros.hpp"
#include "fibonachi.hpp"

TEST_CASE("fibonachi test")
{
    REQUIRE(makeFibunach(0) == 0);
}
