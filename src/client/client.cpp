//
// Created by Igor on 31.01.2021.
//

#include "client.h"

client::client(const std::shared_ptr<document> &doc, const std::shared_ptr<server> &raw_serv) : doc(doc), serv(raw_serv) {
    const auto &[initial_op, serv_state] = serv.connect(this);
    doc->apply(*initial_op);
    last_known_server_state = serv_state;
}

void client::apply_user_op(const std::shared_ptr<operation> &op) {
    doc->apply(*op);

    if (in_flight) {
        if (buffer) {
            buffer->apply(*op);
        } else {
            buffer = op;
        }
    } else {
        if (buffer) {
            throw std::runtime_error("Invariant is not satisfied! in_flight==null -> buffer==null");
        }
        in_flight = op;
        serv.send(in_flight, last_known_server_state);
    }
}

void client::on_ack(const operation &op, const int &new_server_state) {
    if (op.hash() != in_flight->hash()) {
        throw std::runtime_error("Error validating acknowledged operation.");
    }

    last_known_server_state = new_server_state;

    if (buffer) {
        in_flight = buffer;
        serv.send(in_flight, last_known_server_state);
        buffer = nullptr;
    }
}

void client::on_receive(const operation &op, const int &new_server_state) {
    last_known_server_state = new_server_state;

    if (in_flight) {
        const auto &infl_transform = in_flight->transform(op);
        in_flight = infl_transform.second;

        if (buffer) {
            const auto &buff_transform = buffer->transform(*infl_transform.first);
            doc->apply(*buff_transform.first);
            buffer = buff_transform.second;
        } else {
            doc->apply(*infl_transform.first);
        }
    } else {
        if (buffer) {
            throw std::runtime_error("Invariant is not satisfied! in_flight==null -> buffer==null");
        }
        doc->apply(op);
    }
}

node<symbol> *client::generate_node(const int &value) {
    // 5 bit - номер юзера
    // 27 bit - сам айди
    int free_id = free_node_id++;
    if ((free_id >> 27) || serv.client_id == -1) {
        throw std::runtime_error("Id num is too large or client is not connected");
    }
    const int unique_id = (serv.client_id) << 27 | free_id;

    return new node<symbol>(nullptr, nullptr, symbol(unique_id, value));
}
