//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SIMPLE_HISTORY_H
#define OT_VARIATION_SIMPLE_HISTORY_H


#include <memory>
#include "operations_history.h"

class simple_history : public operations_history {
public:
    void push(const std::shared_ptr<operation> &op) override {
todo
    };

    std::shared_ptr<operation> fetch(const int &from) const override {

    };
};


#endif //OT_VARIATION_SIMPLE_HISTORY_H
