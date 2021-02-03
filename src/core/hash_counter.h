//
// Created by Igor on 02.02.2021.
//

#ifndef OT_VARIATION_HASH_COUNTER_H
#define OT_VARIATION_HASH_COUNTER_H

#include "../util.h"

class hash_counter {
    ll data = 0;

public:
    void insert_item(const node_id_t &id, const int &val);

    void delete_item(const node_id_t &id, const int &val);

void update_item(const int &prev_val, const int &val);

    [[nodiscard]] ll hash() const;
};


#endif //OT_VARIATION_HASH_COUNTER_H
