//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_DOCUMENT_H
#define OT_VARIATION_DOCUMENT_H

#include "../core/node.h"
#include "../core/symbol.h"
#include "../core/operation.h"

class document {
    operation root;

public:
    document();
//    todo: корневую нормально обработать. изначально на сервере (и мб клиенте?)
//     документ создается не пустым, а с корневой вершиной 0. от нее дальше скачим

    void apply(const operation &op);

    // returns nullptr if there is no such node
    node<symbol>* get_node(const node_id_t &node_id) const;

    void undo_insertions(const std::unordered_map<node_id_t, int> &insertions);

    [[nodiscard]] int hash() const;
};

#endif //OT_VARIATION_DOCUMENT_H
