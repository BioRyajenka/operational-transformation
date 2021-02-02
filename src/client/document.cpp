//
// Created by Igor on 02.02.2021.
//

#include "document.h"

void document::apply(const operation &op) {
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

int document::hash() const {
    return root.hash();
}

