//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CLIENT_H
#define OT_VARIATION_CLIENT_H


#include <memory>
#include "../core/operation.h"
#include "../core/change.h"
#include "server_peer.h"

class client {
public:
    document doc;
private:
    server_peer server;
    // bridge from the latest known point in the server state
    //  to the latest point in the client state
    operation bridge;
    // а буффер - это как bridge, но без on-flight операции. потому что ее в таком виде нет на сервере
    // буффер - это такая штука что (on-flight'+buffer) приведет в client-state.
    // причем on-flight'=то, что придет в ack

    // on-flight' (which is ack from server) + buffer = bridge

    std::shared_ptr<operation> flight_operation;

    static operation compose(const operation &target);

public:
    void apply_change(const int &pos, const change &change) {
        // применяем к документу сразу
        // прибывляем к bridge (если уже ждем ответа)
    }

    void on_receive(const operation& op) { // TODO: maybe отдельно выделить on_ack?
        // onAck:
        //   флашим буфер и отправляем новые операции которые есть
        //   но их не так просто отправить - тут какой-то буфер
        // else:
        //   применяем к нашему документу? да, но не тут
        //   надо проверять что предок есть общий или сразу мост?
        //   OT(op, bridge)=op',bridge' . затем применяем op' к bridge. а сам bridge=bridge'
    }

//private:
//    static operation apply_change(const operation &target, const int &pos, const change &change);
};


#endif //OT_VARIATION_CLIENT_H
