//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_CHAIN_H
#define OT_VARIATION_CHAIN_H

#include "symbol.h"
#include "node.h"

class chain {
    node<symbol> *root;
public:
    chain(node<symbol> *root) : root(root) {}
    chain(const chain &rhs); // deep copy

    // may be null
    node<symbol> *get_root() const;

    // TODO: когда память освобождать?
    node<symbol> *get_node(unsigned int symbol_id);
};

#endif //OT_VARIATION_CHAIN_H
