//
// Created by Igor on 05.02.2021.
//

#include "jumping_history.h"

void jumping_history::push(const std::shared_ptr<operation> &new_op) {
    stack.emplace_back();
    std::vector<std::shared_ptr<operation>> &jumps = stack[stack.size() - 1];
    // current is stack.size() - 1
    jumps.push_back(new_op); // from current to current + 1 - 2^0
    for (int i = 1; (1 << i) <= (int)stack.size(); i++) {
        // from current to current + 1 - 2^i
        // = (from current - 2^(i-1) to (current + 1 - 2^i))
        // + (from current to current + 1 - 2^(i-1))
        auto op = std::make_shared<operation>();
        op->apply(*stack[stack.size() - 1 - (1 << (i - 1))][i - 1]);
        op->apply(*jumps[i - 1]);
        jumps.push_back(op);
    }
}

std::unique_ptr<operation> jumping_history::fetch(const int &from) const {
    // from "from" to "stack.size() - 1"
    int mask = (int)stack.size() - from;
    int cur = from;

    auto op = std::make_unique<operation>();
    for (int i = 0; (1 << i) <= mask; i++) {
        if (mask & (1 << i)) {
            cur += 1 << i;
            op->apply(*stack[cur - 1][i]);
        }
    }

    return op;
}

int jumping_history::last_state() const {
    return stack.size();
}

