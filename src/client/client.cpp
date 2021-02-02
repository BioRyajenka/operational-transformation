//
// Created by Igor on 31.01.2021.
//

#include <cassert>
#include "client.h"

int client::free_node_id = 0;

client::client(
        const std::shared_ptr<server> &raw_serv,
        const std::unique_ptr<std::function<void(const operation &)>> &operation_listener
) : serv(raw_serv), operation_listener(operation_listener) {
    const auto &[initial_op, serv_state] = serv.connect(this);
    server_doc->apply(*initial_op);
    server_doc_plus_infl.apply(*initial_op);
    last_known_server_state = serv_state;
}

void validate_op_before_send(const operation &op, const document &server_doc) {
    // checking that every change starts at known server doc's state and
    // contains only new nodes in insertions
    for (const auto &node_id : op.deletions) assert(server_doc.get_node(node_id) != nullptr);
    for (const auto&[node_id, _] : op.updates) assert(server_doc.get_node(node_id) != nullptr);
    for (const auto &[node_id, ch] : op.insertions) {
        assert(server_doc.get_node(node_id) != nullptr);
        ch.iterate([&server_doc](const auto &ins_id) { assert(server_doc.get_node(ins_id) == nullptr); });
    }
}

void client::apply_user_op(const std::shared_ptr<operation> &op) {
//    doc->apply(*op);

    if (in_flight) {
        if (buffer) {
            buffer->apply(*op);
        } else {
            buffer = op;
        }
    } else {
        assert(!buffer && "Invariant is not satisfied! in_flight==null -> buffer==null");
        in_flight = op;
        validate_op_before_send(in_flight, server_doc);
        serv.send(in_flight, last_known_server_state);
        server_doc_plus_infl.apply(in_flight);
    }
}

void client::on_ack(const operation &op, const int &new_server_state) {
    assert(op.hash() == in_flight->hash() && "Acknowledged operation should be the same as predicted");

    last_known_server_state = new_server_state;
    server_doc.apply(op);

    assert(server_doc.hash() == server_doc_plus_infl.hash());

    if (buffer) {
        in_flight = buffer;
        validate_op_before_send(in_flight, server_doc);
        serv.send(in_flight, last_known_server_state);
        server_doc_plus_infl.apply(in_flight);
        buffer = nullptr;
    }
}

void client::on_receive(const operation &op, const int &new_server_state) {
    if (in_flight) {
        // greedy mechanism here: sometimes not whole operation can be processed by server
        // so, we process it by client
        const std::shared_ptr<operation> &x = in_flight->detach_unprocessable_by_server(op.deletions);
        if (x != nullptr) {
            assert(x->deletions.empty());
            assert(x->updates.empty());
            server_doc_plus_infl.undo_insertions(x->insertions);
            if (buffer != nullptr) {
                x->apply(buffer);
            }
            buffer = x;
        }

        const auto &infl_transform = in_flight->transform(op, server_doc, false);
        in_flight = infl_transform.second;

        if (buffer) {
            const auto &buff_transform = buffer->transform(*infl_transform.first, server_doc_plus_infl, true);
//            doc->apply(*buff_transform.first);
            buffer = buff_transform.second;
        } else {
//            doc->apply(*infl_transform.first);
        }
        server_doc_plus_infl.apply(*infl_transform.first);
    } else {
        assert(!buffer && "Invariant is not satisfied! in_flight==null -> buffer==null");
//        doc->apply(op);
        server_doc_plus_infl.apply(op);
    }

    last_known_server_state = new_server_state;
    server_doc.apply(op);
}

node<symbol> *client::generate_node(const int &value) {
    if (serv.client_id == -1) {
        throw std::runtime_error("Client is not connected");
    }
    return new node<symbol>(nullptr, nullptr, symbol(serv.client_id, free_node_id++, value));
}
