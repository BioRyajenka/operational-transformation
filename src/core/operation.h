//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_OPERATION_H
#define OT_VARIATION_OPERATION_H


#include <utility>
#include "document.h"
#include "change.h"

class operation {
    // todo: list of changes

    // todo: pointer to operation maybe?
    std::pair<operation, operation> transform(const operation &rhs);

    void apply_to(const document &doc);

    operation apply_change(const int &pos, const change &change);
};


#endif //OT_VARIATION_OPERATION_H
