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
            const std::shared_ptr<server_peer> &peer,
            const std::function<void(const operation &)> &operation_listener
    );

private:
    int client_id;

    std::shared_ptr<server_peer> peer;

    std::shared_ptr<operation> in_flight = nullptr;

    // on-flight' (which is ack from server) + buffer = bridge
    std::shared_ptr<operation> buffer = nullptr;

    int last_known_server_state = -1;

    static int free_node_id;

    const std::function<void(const operation &)> operation_listener;

    void send_to_server(const operation &op, const int &parent_state);

public:
    // public for testing
    std::shared_ptr<document> server_doc;
    std::shared_ptr<document> server_doc_plus_infl;

    void apply_user_op(const std::shared_ptr<operation> &op);

    void on_ack(const operation &op, const int &new_server_state);

    void on_receive(const operation &op, const int &new_server_state);

    symbol generate_symbol(const int &value) const;

    int id() const;
};


#endif //OT_VARIATION_CLIENT_H
