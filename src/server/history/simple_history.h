//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SIMPLE_HISTORY_H
#define OT_VARIATION_SIMPLE_HISTORY_H


#include <memory>
#include <string>
#include "operations_history.h"
#include "../../testing/util/test_util.h"

class simple_history : public operations_history {
    std::vector<std::shared_ptr<operation>> stack;

public:
    void push(const std::shared_ptr<operation> &op) override {
        stack.push_back(op);
    };

    [[nodiscard]] std::shared_ptr<operation> fetch(const int &from) const override {
        assert(from >= 0 && from < stack.size());
        std::shared_ptr<operation> op = std::make_shared<operation>();

//        if (from == 2) printf("stack: \n");
        for (int i = from; i < stack.size(); i++) {
//            if (from == 2) {
//                print_operation("[" + std::to_string(i) + "]", *stack[i]);
//            }
            op->apply(*stack[i]);
        }
        return op;
    };

    [[nodiscard]] int last_state() const override {
        return (int) stack.size();
    }
};


#endif //OT_VARIATION_SIMPLE_HISTORY_H
