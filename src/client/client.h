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
    client(int client_id);

private:
    int client_id;

    std::shared_ptr<server_peer> peer;

    std::shared_ptr<operation> in_flight = nullptr;

    // on-flight' (which is ack from server) + buffer = bridge
    std::unique_ptr<operation> buffer = nullptr;

    int last_known_server_state = -1;

    static int free_node_id;

    std::function<void(const operation &)> operation_listener;

private:
    void send_to_server(const std::shared_ptr<operation> &op, int parent_state);

    symbol generate_symbol(int value) const;

public:
    // public for testing
    std::shared_ptr<document> server_doc;

public:
    void connect(const std::shared_ptr<server_peer> &peer);
    void disconnect();

    void apply_user_op(std::unique_ptr<operation> &op);

    void do_insert(node_id_t node_id, const int value);
    void do_update(node_id_t node_id, const int new_value);
    void do_delete(node_id_t node_id);

    void on_ack(const operation &op, int new_server_state);

    void on_receive(const operation &op, int new_server_state);

    int id() const;

    void set_operation_listener(std::function<void(const operation &)> operation_listener);
};


#endif //OT_VARIATION_CLIENT_H
