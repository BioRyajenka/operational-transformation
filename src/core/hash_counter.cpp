//
// Created by Igor on 02.02.2021.
//

#include "hash_counter.h"

//static const int PRIME = 1000000009;
//static const int HASH_FLAG_INS = 1351727964;
//static const int HASH_FLAG_UPD = -472705117;
//static const int HASH_FLAG_DEL = 3552447;
static const int DIRT = -472705117;

void hash_counter::insert_item(const node_id_t &id, const int &val) {
    data ^= (int) (id * DIRT) ^ val;
}

void hash_counter::delete_item(const node_id_t &id, const int &val) {
    data ^= (int) (id * DIRT) ^ val;
}

void hash_counter::update_item(const int &prev_val, const int &val) {
    data ^= prev_val ^ val;
}

ll hash_counter::hash() const {
    return data;
}
