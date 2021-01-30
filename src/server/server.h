//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SERVER_H
#define OT_VARIATION_SERVER_H


#include "../core/document.h"
#include "../core/operation.h"

class server {
public:
    document doc;

    void on_receive(const operation &op) {
        // с самого начала проверяем что эта операция parented by smth in history

        // по операции находим в кеше все операции, которые были применены с тех пор как она случилась

        // затем их комбиним вместе (тут можно же оптимизировать?) и применяем OT (a, комбинация)

        // применяем a' и броадкастим его всем клиентам, включая самого чувака
        // причем в этом месте важный момент: к операции добавляем метадату, и у a' метадата = метадате a
        // это нужно чтобы различать наша/не наша операция вернулась клиенту. но правда ли что достаточно
        // bool присылать?
    }
};


#endif //OT_VARIATION_SERVER_H
