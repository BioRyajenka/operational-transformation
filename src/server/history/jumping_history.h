//
// Created by Igor on 05.02.2021.
//

#ifndef OT_VARIATION_JUMPING_HISTORY_H
#define OT_VARIATION_JUMPING_HISTORY_H

#include "../../core/operation.h"
#include "operations_history.h"

class jumping_history : public operations_history {
private:
    std::vector<std::vector<std::shared_ptr<operation>>> stack;

public:
    ~jumping_history() override = default;

    void push(const std::shared_ptr<operation> &new_op) override;

    [[nodiscard]] std::unique_ptr<operation> fetch(int from) const override;

    [[nodiscard]] int last_state() const override;
};

#endif //OT_VARIATION_JUMPING_HISTORY_H
