//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_SYMBOL_H
#define OT_VARIATION_SYMBOL_H

#include <cassert>
#include "../util.h"

class symbol {
public:
    // 16 bit - номер юзера, 48 bit - сам айди
    // или 6 bit - номер юзера, 26 bit - сам айди
    node_id_t id;
    int value;

    symbol(const int &client_id, const int &node_id, const int &value) : value(value) {
        assert(client_id > 0);
        if (node_id >> 26) throw std::runtime_error("Id num is too large");
        id = (client_id << 26) | node_id;
    }
};

#endif //OT_VARIATION_SYMBOL_H
