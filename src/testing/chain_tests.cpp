//
// Created by Igor on 03.02.2021.
//

#include "util/doctest.h"
#include "../core/chain.h"
#include "util/test_util.h"

// для SUBCASE TEST_CASE всегда с самого начала перезапускается
TEST_CASE("chain") {
    chain ch(create_symbol(0, 0));

//    SUBCASE("is created with single node") {
    CHECK(ch.get_head() == ch.get_tail());
//    }

//    SUBCASE("copy to the beginning") {
    ch.copy_to_the_beginning(chain(create_symbol(1, 0)));
    CHECK(ch.get_head()->value.id == 1);
    CHECK(ch.get_tail()->value.id == 0);
//    }

//    SUBCASE("move_to (the last node)") {
    ch.move_to(ch.find_node(0), chain(create_symbol(2, 0)));
    CHECK(ch.get_head()->value.id == 1);
    CHECK(ch.get_head()->next->value.id == 0);
    CHECK(ch.get_tail()->prev->value.id == 0);
    CHECK(ch.get_tail()->value.id == 2);
//    }

//    SUBCASE("remove (first node)") {
    ch.remove_node(ch.get_head());
    CHECK(ch.get_head()->value.id == 0);
    CHECK(ch.get_tail()->value.id == 2);
//    }

//    SUBCASE("remove (last node)") {
    ch.remove_node(ch.get_tail());
    CHECK(ch.get_head()->value.id == 0);
    CHECK(ch.get_tail()->value.id == 0);
//    }
}
