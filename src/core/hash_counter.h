//
// Created by Igor on 02.02.2021.
//

#ifndef OT_VARIATION_HASH_COUNTER_H
#define OT_VARIATION_HASH_COUNTER_H

#include "../util.h"

class hash_counter {
public:
    void add_item(const int &item);

    [[nodiscard]] ll hash() const;
};


#endif //OT_VARIATION_HASH_COUNTER_H
