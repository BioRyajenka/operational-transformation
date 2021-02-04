//
// Created by Igor on 02.02.2021.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "util/doctest.h"
#include "../core/operation.h"
#include "util/test_util.h"
#include "../client/document.h"

TEST_CASE("operation") {
    operation base_op;
    base_op.insert(0, chain(create_symbol(1, 0)));

    SUBCASE("initialization") {
        CHECK(base_op.get_updates()->empty());
        CHECK(base_op.get_deletions()->empty());
        CHECK(base_op.get_insertions()->size() == 1);
        CHECK(base_op.get_insertions()->count(0));
    }

    SUBCASE("deleting from inserted") {
        std::shared_ptr<document> doc = std::make_shared<document>();
        doc->apply(base_op);

        CHECK(doc->get_node(0));
        CHECK(doc->get_node(1));
        CHECK(doc->get_node(0)->next == doc->get_node(1));

        operation op;
        op.insert(1, chain(create_symbol(2, 0)));

        op.del(1, doc);
        // should be rehanged to 0. before: 1->2. after: 0->2
        CHECK(op.get_insertions()->find(0) != op.get_insertions()->end());
        CHECK(op.get_insertions()->at(0).get_head()->value.id == 2);
        CHECK(op.get_insertions()->at(0).get_tail()->value.id == 2);

        doc->apply(op);
        // before: 0->1. after: 0->2
        CHECK(doc->get_node(1) == nullptr);
        CHECK(doc->get_node(0)->next == doc->get_node(2));
        CHECK(doc->get_node(2)->next == nullptr);
    }

    SUBCASE("inserting into new node") {
        auto op = std::make_shared<operation>();
        op->insert(1, create_chain({2}));
        base_op.apply(*op, nullptr);
        CHECK(base_op.get_insertions()->find(0)->second.get_tail()->value.id == 2);
    }
}
