//
// Created by Igor on 03.02.2021.
//

#include "util/doctest.h"
#include "../core/hash_counter.h"

TEST_CASE("hash_counter") {
    hash_counter a, b;

    CHECK(a.hash() == 0);
    CHECK(a.hash() == b.hash());

    a.apply_change(1);
    a.apply_change(7);
    a.apply_change(13);

    b.apply_change(1);
    b.apply_change(7);
    b.apply_change(13);

    CHECK(a.hash() == b.hash());
}

