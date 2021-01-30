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

public:
    operation(const int &pos, const change &change);

    int hash() const; // only for validation purposes

    // todo: pointer to operation maybe?
    /**
     * for (this, rhs) returns (rhs', this')
     */
    std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> transform(const operation &rhs) const;

    operation apply_change(const int &pos, const change &change);
};


#endif //OT_VARIATION_OPERATION_H
