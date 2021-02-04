//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_DOCUMENT_H
#define OT_VARIATION_DOCUMENT_H

#include "../core/node.h"
#include "../core/symbol.h"
#include "../core/operation.h"

class document {
    chain data;
    std::unordered_map<node_id_t, node<symbol>*> map;

    // hash will track only node ids and their values (ignoring order)
    ll content_hash = 0;

public:
    document() : data(symbol::initial) {
        map[symbol::initial.id] = data.get_head();
    }

    void apply(const operation &s);

    // returns nullptr if there is no such node
    [[nodiscard]] node<symbol> const *get_node(const node_id_t &node_id) const;

    // assuming they were in last applied operation
    void undo_insertions(const std::unordered_map<node_id_t, chain> &insertions);

    [[nodiscard]] ll hash() const;
};

#endif //OT_VARIATION_DOCUMENT_H
