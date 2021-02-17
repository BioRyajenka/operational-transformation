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

std::unique_ptr<operation> simple_history::fetch(const int &from) const {
    assert(from >= 0 && from <= stack.size());
    auto op = std::make_unique<operation>();

    for (int i = from; i < stack.size(); i++) {
        op->apply(*stack[i]);
    }

    return op;
}

int simple_history::last_state() const {
    return (int) stack.size();
}


