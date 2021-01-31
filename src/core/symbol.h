//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_SYMBOL_H
#define OT_VARIATION_SYMBOL_H

class symbol {
public:
    // 5 bit - номер юзера
    // 27 bit - сам айди
    unsigned int id;
    int value;

    symbol(const unsigned int &id, const int &value) : id(id), value(value) {}
};

#endif //OT_VARIATION_SYMBOL_H
