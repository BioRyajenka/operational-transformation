//
// Created by Igor on 03.02.2021.
//

#include "util/doctest.h"
#include "../client/document.h"
#include "util/test_util.h"

TEST_CASE("document") {
    std::shared_ptr<document> doc = std::make_shared<document>();
    operation base_op;
    base_op.insert(0, create_chain({1, 2, 3}));
    doc->apply(base_op);
    CHECK(check_doc_contents(*doc, {0, 1, 2, 3}));

    SUBCASE("comprehensive test") {
        operation op1;
        op1.insert(2, create_chain({4, 5}));
        doc->apply(op1);
        CHECK(check_doc_contents(*doc, {0, 1, 2, 4, 5, 3}));

        operation op2;
        op2.del(2, nullptr);
        op2.del(3, nullptr);
        op2.del(5, nullptr);
        doc->apply(op2);
        CHECK(check_doc_contents(*doc, {0, 1, 4}));
    }
}
