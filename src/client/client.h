//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CLIENT_H
#define OT_VARIATION_CLIENT_H


#include <memory>
#include <stdexcept>
#include <functional>
#include "../core/operation.h"
#include "document.h"
#include "server_peer.h"

class client {
public:
    client(
            const std::shared_ptr<server> &serv,
            const std::unique_ptr<std::function<void(const operation &)>> &operation_listener
    );

private:
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

    static int free_node_id;

    const std::unique_ptr<std::function<void(const operation &)>> &operation_listener;

public:
    // public for testing
    document server_doc;
    document server_doc_plus_infl;

    void apply_user_op(const std::shared_ptr<operation> &op);

    void on_ack(const operation &op, const int &new_server_state);

    void on_receive(const operation &op, const int &new_server_state);

    node<symbol> *generate_node(const int &value);
};


#endif //OT_VARIATION_CLIENT_H
