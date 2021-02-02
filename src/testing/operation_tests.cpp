//
// Created by Igor on 02.02.2021.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"

TEST_CASE("hi") {
    REQUIRE(1 == 2);
    CHECK(2 == 2);
}
