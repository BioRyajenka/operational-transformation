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
    ~simple_history() override;

    void push(const std::shared_ptr<operation> &op) override;

    [[nodiscard]] std::shared_ptr<operation> fetch(const int &from) const override;

    [[nodiscard]] int last_state() const override;
};


#endif //OT_VARIATION_SIMPLE_HISTORY_H
