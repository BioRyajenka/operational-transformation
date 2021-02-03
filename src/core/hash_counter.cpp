//
// Created by Igor on 02.02.2021.
//

#include "hash_counter.h"

static const int PRIME = 1000000009;

void hash_counter::apply_change(const int &change) {
    data = data * PRIME + change;
}

ll hash_counter::hash() const {
    return data;
}
