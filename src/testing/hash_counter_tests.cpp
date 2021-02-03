//
// Created by Igor on 03.02.2021.
//

#include "util/doctest.h"
#include "../core/hash_counter.h"

TEST_CASE("hash_counter") {
    hash_counter a, b;

    CHECK(a.hash() == 0);
    CHECK(a.hash() == b.hash());

    a.insert_item(2, 2);

    b.insert_item(3, 4);
    b.insert_item(2, 3);
    b.update_item(4, 5); // updating 3
    b.update_item(3, 2); // updating 2
    b.delete_item(3, 5);

    CHECK(a.hash() == b.hash());
}

