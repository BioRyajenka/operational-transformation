//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_OPERATIONS_HISTORY_H
#define OT_VARIATION_OPERATIONS_HISTORY_H


#include "../../core/operation.h"

class operations_history {
public:
    virtual ~operations_history() {}

    virtual void push(const std::shared_ptr<operation> &op) = 0;
    [[nodiscard]] virtual std::shared_ptr<operation> fetch(const int &from) const = 0;
    // returns last state, which is = to the state, to which last op is pointing
    [[nodiscard]] virtual int last_state() const = 0;
};


#endif //OT_VARIATION_OPERATIONS_HISTORY_H
