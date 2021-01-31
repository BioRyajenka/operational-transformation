//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CLIENT_H
#define OT_VARIATION_CLIENT_H


#include <memory>
#include <stdexcept>
#include "../core/operation.h"
#include "document.h"
#include "server_peer.h"

class client {
public:
    client(const std::shared_ptr<document> &doc, const std::shared_ptr<server> &serv);

private:
    std::shared_ptr<document> doc;

    server_peer serv;
    // bridge from the latest known point in the server state
    //  to the latest point in the client state
//    operation bridge;
    // а буффер - это как bridge, но без on-flight операции. потому что ее в таком виде нет на сервере
    // буффер - это такая штука что (on-flight'+buffer) приведет в client-state.
    // причем on-flight'=то, что придет в ack

    std::shared_ptr<operation> in_flight = nullptr;

    // on-flight' (which is ack from server) + buffer = bridge
    std::shared_ptr<operation> buffer = nullptr;

    int last_known_server_state = -1;

    static int free_node_id = 0;

public:
    void apply_user_op(const std::shared_ptr<operation> &op);

    // op is needed only for validation purposes
    void on_ack(const operation &op, const int &new_server_state);

    void on_receive(const operation &op, const int &new_server_state);

    node<symbol>* generate_node(const int &value);
};


#endif //OT_VARIATION_CLIENT_H
