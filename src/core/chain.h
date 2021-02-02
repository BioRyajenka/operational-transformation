//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_CHAIN_H
#define OT_VARIATION_CHAIN_H

#include <cassert>
#include <functional>
#include "symbol.h"
#include "node.h"

class chain {
    node<symbol> *root;
public:
//    cant be null
    chain(node<symbol> *root) : root(root) {
        assert(root->prev == nullptr);
    }
//    chain(const chain &rhs); // deep copy

    node<symbol> *get_root() const; // TODO: добавить сюда проверку что не пустая цепь и что prev == null

    node<symbol> *get_last() const; // TODO: make it simple, without precalc

    // TODO: is it for debug only?
    /**
        auto cur = ch.get_root();
        while (cur != nullptr) {
            ;
            cur = cur->next;
        }
     */
    void iterate(std::function<void(const node_id_t &)> consumer) const;

    node<symbol> *deep_copy() const;

    void add_to_beginning(node<symbol> *prefix_chain);
};

#endif //OT_VARIATION_CHAIN_H
