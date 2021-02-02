//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_DOCUMENT_H
#define OT_VARIATION_DOCUMENT_H

#include "../core/node.h"
#include "../core/symbol.h"
#include "../testing/magic_list.h"
#include "../core/chain.h"
#include "../core/operation.h"

class operation;

class document {
public:
    document() = default;
//    todo: корневую нормально обработать. изначально на сервере (и мб клиенте?)
//     документ создается не пустым, а с корневой вершиной 0. от нее дальше скачим

    virtual void apply(const operation &op) = 0;

    // returns nullptr if there is no such node
    node<symbol>* get_node(const node_id_t &node_id) const;

    int hash() const;
};

#endif //OT_VARIATION_DOCUMENT_H
