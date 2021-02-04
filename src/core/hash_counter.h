//
// Created by Igor on 02.02.2021.
//

#ifndef OT_VARIATION_HASH_COUNTER_H
#define OT_VARIATION_HASH_COUNTER_H

#include "../util.h"

class hash_counter {
    ll data = 0;

public:
    void apply_change(const int &change);

    void reset(const ll &new_value);

    [[nodiscard]] ll hash() const;
};


#endif //OT_VARIATION_HASH_COUNTER_H
