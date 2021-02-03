//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_SYMBOL_H
#define OT_VARIATION_SYMBOL_H

#include <cassert>
#include "../util.h"

class symbol {
private:
    symbol(const node_id_t &id, const int &value);

public:
    // 16 bit - номер юзера, 48 bit - сам айди
    // или 6 bit - номер юзера, 26 bit - сам айди
    node_id_t id;
    int value;

    symbol(const int &client_id, const int &node_id, const int &value);

    static symbol initial;
};

#endif //OT_VARIATION_SYMBOL_H
