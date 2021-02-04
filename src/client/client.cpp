//
// Created by Igor on 31.01.2021.
//

#include <cassert>
#include "client.h"
#include "../server/server.h"
#include "../testing/util/test_util.h"

int client::free_node_id = 0;

client::client(
        const std::shared_ptr<server_peer> &peer,
        const std::function<void(const operation &)> &operation_listener
) : peer(peer), operation_listener(operation_listener) {
    const auto &[client_id, initial_op, serv_state] = peer->connect(this);

    this->client_id = client_id;

    operation_listener(*initial_op);//    doc->apply(*op);
    server_doc = std::make_shared<document>();
    server_doc->apply(*initial_op);
    server_doc_plus_infl = std::make_shared<document>();
    server_doc_plus_infl->apply(*initial_op);

    last_known_server_state = serv_state;
}

void validate_op_before_send(const operation &op, const document &server_doc) {
    // checking that every change starts at known server doc's state and
    // contains only new nodes in insertions
    // updates can be started in new nodes also
    for (const auto &node_id : *op.get_deletions()) assert(server_doc.get_node(node_id));
//    for (const auto&[node_id, _] : *op.get_updates()) assert(server_doc.get_node(node_id));
    for (const auto &[node_id, ch] : *op.get_insertions()) {
        assert(server_doc.get_node(node_id));
        ch.iterate([&server_doc](const auto &s) { assert(server_doc.get_node(s.id) == nullptr); });
    }
}

void client::apply_user_op(const std::shared_ptr<operation> &op) {
    operation_listener(*op);//    doc->apply(*op);

    if (in_flight) {
        if (buffer) {
            buffer->apply(*op, server_doc_plus_infl);
        } else {
            buffer = op;
        }
    } else {
        assert(!buffer && "Invariant is not satisfied! in_flight==null -> buffer==null");
        in_flight = op;
        validate_op_before_send(*in_flight, *server_doc);
        send_to_server(*in_flight, last_known_server_state);
        server_doc_plus_infl->apply(*in_flight);
    }
}

void client::on_ack(const operation &op, const int &new_server_state) {
    assert(op.hash() == in_flight->hash() && "Acknowledged operation should be the same as predicted");

    last_known_server_state = new_server_state;
    server_doc->apply(op);

//    print_doc("server_doc", *server_doc);
//    print_doc("server_doc_plus_infl", *server_doc_plus_infl);

    assert(server_doc->hash() == server_doc_plus_infl->hash());

    if (buffer) {
        in_flight = buffer;
        validate_op_before_send(*in_flight, *server_doc);
        send_to_server(*in_flight, last_known_server_state);
        server_doc_plus_infl->apply(*in_flight);
        buffer = nullptr;
    } else {
        in_flight = nullptr;
    }
}

void client::on_receive(const operation &op, const int &new_server_state) {
    if (in_flight) {
        // greedy mechanism here: sometimes not whole operation can be processed by server
        // so, we process it by client
        const std::shared_ptr<operation> &x = in_flight->detach_unprocessable_by_server(*op.get_deletions());
        if (x != nullptr) {
            assert(x->get_deletions()->empty());
            assert(x->get_updates()->empty());
            server_doc_plus_infl->undo_insertions(*x->get_insertions());
            if (buffer != nullptr) {
                x->apply(*buffer, server_doc_plus_infl);
            }
            buffer = x;
        }

        const auto &infl_transform = in_flight->transform(op, server_doc, false);
        in_flight = infl_transform.second;

        if (buffer) {
            // without doc, only_right_part can be false here
            const auto &buff_transform = buffer->transform(*infl_transform.first, server_doc_plus_infl, false);
            operation_listener(*buff_transform.first);//            doc->apply(*buff_transform.first);
            buffer = buff_transform.second;
        } else {
            operation_listener(*infl_transform.first);//            doc->apply(*infl_transform.first);
        }
        server_doc_plus_infl->apply(*infl_transform.first);
    } else {
        assert(!buffer && "Invariant is not satisfied! in_flight==null -> buffer==null");
        operation_listener(op);//        doc->apply(op);
        server_doc_plus_infl->apply(op);
    }

    last_known_server_state = new_server_state;
    server_doc->apply(op);
}

symbol client::generate_symbol(const int &value) {
    assert(client_id != -1 && "Client is not connected");
    return symbol(client_id, free_node_id++, value);
}

void client::send_to_server(const operation &op, const int &parent_state) {
    peer->send(client_id, op, parent_state);
}

int client::id() const {
    return client_id;
}
