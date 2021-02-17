//
// Created by Igor on 31.01.2021.
//

#include <string>
#include <cassert>
#include <utility>
#include "client.h"
#include "../server/server.h"

int client::free_node_id = 0;

client::client(
        const int &client_id
) : operation_listener([](const operation &) {}) {
    assert(client_id > 0 && "Client id should be positive!");
    this->client_id = client_id;
    server_doc = std::make_shared<document>();
    last_known_server_state = 0;
}

void validate_against_state(const operation &op, const document &state) {
    // checking that every change starts at known server doc's state and
    // contains only new nodes in insertions
    // updates can be started in new nodes also
    for (const auto &[node_id, parent_id] : *op.get_deletions()) {
        assert(state.get_node(node_id));
        assert(state.get_node(parent_id));
    }
//    for (const auto&[node_id, _] : *op.get_updates()) assert(server_doc.get_node(node_id));
    for (const auto &[node_id, ch] : *op.get_insertions()) {
        assert(state.get_node(node_id));
        ch.iterate([&state](const auto &s) { assert(!state.get_node(s.id)); });
    }
}

void client::connect(const std::shared_ptr<server_peer> &serv_peer) {
    peer = serv_peer;
    const auto &[initial_op, serv_state] = peer->connect(this, last_known_server_state);
    in_flight = std::make_shared<operation>(); // new empty operation
    on_receive(*initial_op, serv_state);
    on_ack(*in_flight, serv_state);// acknowledged empty
}

void client::disconnect() {
    peer->disconnect(client_id);
    assert(!in_flight);
    assert(!buffer);
    peer = nullptr;
}

void client::apply_user_op(std::unique_ptr<operation> &op) {
    operation_listener(*op);//    doc->apply(*op);

    if (in_flight || !peer) { // if something sended or not connected
        if (buffer) {
            buffer->apply(*op);
        } else {
            buffer = std::move(op);
        }
    } else {
        assert(!buffer && "Invariant is not satisfied! connected -> in_flight==null -> buffer==null");
        in_flight = std::move(op);
        send_to_server(in_flight, last_known_server_state);
    }
}

void client::on_ack(const operation &op, const int &new_server_state) {
    assert(op.hash() == in_flight->hash() && "Acknowledged operation should be the same as predicted");

    server_doc->apply(op);
    last_known_server_state = new_server_state;

    if (buffer) {
        in_flight = std::move(buffer);
        send_to_server(in_flight, last_known_server_state);
    } else {
        in_flight = nullptr;
    }
}

void client::on_receive(const operation &op, const int &new_server_state) {
    // === validate ===
    validate_against_state(op, *server_doc);

    if (in_flight) {
        auto infl_transform = in_flight->transform(op, false);
        in_flight = std::move(infl_transform.second);

        if (buffer) {
            // if without doc, only_right_part can be true here
            auto buff_transform = buffer->transform(*infl_transform.first, false);
            operation_listener(*buff_transform.first);//            doc->apply(*buff_transform.first);
            buffer = std::move(buff_transform.second);
        } else {
            operation_listener(*infl_transform.first);//            doc->apply(*infl_transform.first);
        }
    } else {
        assert(!buffer && "Invariant is not satisfied! in_flight==null -> buffer==null");
        operation_listener(op);//        doc->apply(op);
    }

    server_doc->apply(op);
    last_known_server_state = new_server_state;
}

symbol client::generate_symbol(const int &value) const {
    assert(client_id != -1 && "Client is not connected");
    const symbol &res = symbol(client_id, free_node_id++, value);
    return res;
}

void client::send_to_server(const std::shared_ptr<operation> &op, const int &parent_state) {
    assert(peer);
    validate_against_state(*in_flight, *server_doc);
    peer->send(client_id, op, parent_state);
}

int client::id() const {
    return client_id;
}

void client::do_insert(const node_id_t &node_id, const int value) {
    auto op = std::make_unique<operation>();
    op->insert(node_id, chain(generate_symbol(value)));
    apply_user_op(op);
}

void client::do_update(const node_id_t &node_id, const int new_value) {
    auto op = std::make_unique<operation>();
    op->update(node_id, new_value);
    apply_user_op(op);
}

void client::do_delete(const node_id_t &node_id) {
    node_id_t parent_id;
    auto node_in_doc = server_doc->get_node(node_id);

    if (node_in_doc) {
        parent_id = node_in_doc->prev->value.id;
        if (in_flight) {
            const auto &infl_deleted = in_flight->get_deletions()->find(parent_id);
            if (infl_deleted != in_flight->get_deletions()->end()) {
                parent_id = infl_deleted->second;
            }
            const auto &infl_inserted = in_flight->get_insertions()->find(parent_id);
            if (infl_inserted != in_flight->get_insertions()->end()) {
                parent_id = infl_inserted->second.get_tail()->value.id;
            }

            if (buffer) {
                const auto &buf_deleted = buffer->get_deletions()->find(parent_id);
                if (buf_deleted != buffer->get_deletions()->end()) {
                    parent_id = buf_deleted->second;
                }
            }
        }
    } else {
        // it is not in server_doc, which means it is either in in_flight or in buffer

        bool found_in_infl = false;

        if (peer) { // if connected
            assert(in_flight);

            for (const auto &[n_id, ch] : *in_flight->get_insertions()) {
                const auto &n = ch.find_node(node_id);
                if (n) {
                    if (n == ch.get_head()) {
                        parent_id = n_id;
                    } else {
                        parent_id = n->prev->value.id;
                    }
                    found_in_infl = true;
                    break;
                }
            }
        }

        if (!found_in_infl) {
            assert(buffer);
            parent_id = -1; // it is not used, because node will be immediately deleted from the buffer
        }
    }

    auto op = std::make_unique<operation>();
    op->del(node_id, parent_id);
    apply_user_op(op);
}

void client::set_operation_listener(std::function<void(const operation &)> new_listener) {
    this->operation_listener = std::move(new_listener);
}
