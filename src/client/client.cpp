//
// Created by Igor on 31.01.2021.
//

#include <string>
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
    last_known_server_state = serv_state;
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

void client::apply_user_op(const std::shared_ptr<operation> &op) {
    operation_listener(*op);//    doc->apply(*op);

    if (in_flight) {
        if (buffer) {
            buffer->apply(*op);
        } else {
            buffer = op;
        }
    } else {
        assert(!buffer && "Invariant is not satisfied! in_flight==null -> buffer==null");
        in_flight = op;
        send_to_server(*in_flight, last_known_server_state);
    }
}

void client::on_ack(const operation &op, const int &new_server_state) {
    if (op.hash() != in_flight->hash()) {
        print_operation("op", op);
        print_operation("in_flight", *in_flight);
        fflush(stdout);
    }
    assert(op.hash() == in_flight->hash() && "Acknowledged operation should be the same as predicted");

    server_doc->apply(op);
    last_known_server_state = new_server_state;
//    print_doc("server_doc", *server_doc);
//    print_doc("server_doc_plus_infl", *server_doc_plus_infl);

    if (buffer) {
        in_flight = buffer;
        send_to_server(*in_flight, last_known_server_state);
        buffer = nullptr;
    } else {
        in_flight = nullptr;
    }
}

void client::on_receive(const operation &op, const int &new_server_state) {
    // === validate ===
    validate_against_state(op, *server_doc);

    if (in_flight) {
        const auto &infl_transform = in_flight->transform(op, false);
        in_flight = infl_transform.second;

        if (buffer) {
            // if without doc, only_right_part can be true here
            const auto &buff_transform = buffer->transform(*infl_transform.first, false);
            operation_listener(*buff_transform.first);//            doc->apply(*buff_transform.first);
            buffer = buff_transform.second;
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
//    printf("client %d generated symbol %d\n", client_id, res.id);
    return res;
}

void client::send_to_server(const operation &op, const int &parent_state) {
    validate_against_state(*in_flight, *server_doc);
//    print_operation("client " + std::to_string(client_id) + " sends an operation", op);
    peer->send(client_id, op, parent_state);
}

int client::id() const {
    return client_id;
}

void client::do_insert(const node_id_t &node_id, const int value) {
    auto op = std::make_shared<operation>();
    op->insert(node_id, chain(generate_symbol(value)));
    apply_user_op(op);
}

void client::do_update(const node_id_t &node_id, const int new_value) {
    auto op = std::make_shared<operation>();
    op->update(node_id, new_value);
    apply_user_op(op);
}

void client::do_delete(const node_id_t &node_id) {
    node_id_t parent_id;
    auto it = server_doc->get_node(node_id);

    if (it) {
        parent_id = it->prev->value.id;
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
        // TODO: я могу все-таки одновременно и в insertions и в deletions ноду держать?
        // it is not in server_doc, which means it is either in in_flight or in buffer

        assert(in_flight);

        bool found_in_infl = false;
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

        if (!found_in_infl) {
            assert(buffer);
            parent_id = -1; // it is not used, because node will be immediately deleted from the buffer
        }
    }

    auto op = std::make_shared<operation>();
    op->del(node_id, parent_id);
    apply_user_op(op);
}
