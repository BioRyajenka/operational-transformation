//
// Created by Igor on 06.02.2021.
//

#include "simple_history.h"


simple_history::~simple_history() {
    stack.clear();
}

void simple_history::push(const std::shared_ptr<operation> &op) {
    stack.push_back(op);
}

void print_history_about(const std::vector<std::shared_ptr<operation>> &stack, const node_id_t &node_id) {
    for (int i = 0; i < (int) stack.size(); i++) {
        const auto &op = stack[i];
        for (const auto &[d, p] : *op->get_deletions()) {
           if (d == node_id || p == node_id) {
               printf("stack[%d]. deletions: %u(%u)\n", i, d, p);
           }
        }

        for (const auto &[n, ch] : *op->get_insertions()) {
            if (n == node_id || ch.find_node(node_id)) {

                printf("stack[%d]. insertions: ", i);
                print_chain("[" + std::to_string(n), ch);
                printf("]\n");
            }
        }
    }
}

std::shared_ptr<operation> simple_history::fetch(const int &from) const {
    assert(from >= 0 && from <= stack.size());
    std::shared_ptr<operation> op = std::make_shared<operation>();

    for (int i = from; i < stack.size(); i++) {
        op->apply(*stack[i]);
    }

    return op;
}

int simple_history::last_state() const {
    return (int) stack.size();
}


