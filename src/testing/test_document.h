//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_TEST_DOCUMENT_H
#define OT_VARIATION_TEST_DOCUMENT_H

#include <algorithm>
#include "../client/document.h"

class test_document : public document {
    operation root;
    magic_list<int> node_ids;

public:
    void apply(const operation &op) override {
        root.apply(op);

        for (const auto&[node_id, value] : op.lists) {
            if (value.first) node_ids.remove(node_id);

            node<symbol> *cur = value.second->get_root();
            while (cur != nullptr) {
                node_ids.insert(cur->value.id);
                cur = cur ->next;
            }
        }
    }

    [[nodiscard]] int get_random_node_id() const {
        return node_ids.get_random();
    }

    [[nodiscard]] int hash() const {
        return root.hash();
    }
};

#endif //OT_VARIATION_TEST_DOCUMENT_H
