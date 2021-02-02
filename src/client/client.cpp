//
// Created by Igor on 31.01.2021.
//

#include <cassert>
#include "client.h"

client::client(const std::shared_ptr<server> &raw_serv) : serv(raw_serv) {
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
        ch.iterate([](const auto &ins_id) { assert(server_doc.get_node(ins_id) == nullptr); });
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
        const auto &infl_transform = in_flight->transform(op, server_doc);
        in_flight = infl_transform.second;

        if (buffer) {
            const auto &buff_transform = buffer->transform(*infl_transform.first, server_doc_plus_infl);
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

void client::on_recover(const operation &op, const int &new_server_state) {
    // === validating ops (only for debug) ===
    for (const auto &)

    // client is now guaranteed to receive the state where conflictings ids are deleted
    // TODO: do some synchronization queue magic to ensure that
    // TODO: check here doc which is server copy

    // reduce current inflight by v'
    // resend new inflight (appended with buffer)

    // итак, к этому моменту клиент базируется на стейте  который после серверного del
    // inflight сейчас это (v+x)', а buffer тоже соответственно изменен
    // все изменения применены к doc (включая del'), осталось только перебазироваться из
    // текущего стейта в v', т.к. сейчас именно такой стейт на сервере
    // для этого надо разделить (v+x)' на v'+x' и сделать inflight=x'
    // затем заново отправить новый inflight=inflight+buffer на сервер

    // кажется, что весь v' находится в (v+x)' в виде префиксов
    for (const auto &[key, value] : op.lists) {
        assert(in_flight->lists.count(key));
        // TODO: тут можно гораздо оптимальней, но в целях валидации кода пока так
        auto v_cur = value->ch.get_root();
        auto infl_cur = in_flight->lists[key]->ch.get_root();
        // v   : a
        // infl: a -> b -> c
        // res : b -> c
        in_flight->lists.erase(key);

        // TODO: maybe release memory somewhere here?
        if (v_cur != nullptr) {
            infl_cur = infl_cur->next;
            while (v_cur->next != nullptr) {
                assert(infl_cur != nullptr);
                v_cur = v_cur->next;
                infl_cur = infl_cur->next;
            }
        }
        if (infl_cur != nullptr) {
            infl_cur->prev = null ? возможно, можно и
            без этого
            // TODO: assert current server doc has key
            in_flight->lists[key] = std::make_shared<change>(change::TYPE_INSERT, 0, infl_cur);
        }
    }

    todo:
    не
    забыть
    v
    ' к локал-сервер-стейту применить

    // TODO: check that in_flight is not empty

    last_known_server_state = new_server_state;
    if (buffer) {
        in_flight->apply(*buffer);
        buffer = nullptr;
    }
    validate_op_before_send(in_flight, server_doc);
    serv.send(in_flight, last_known_server_state);
}

node<symbol> *client::generate_node(const int &value) {
    if (serv.client_id == -1) {
        throw std::runtime_error("Client is not connected");
    }
    return new node<symbol>(nullptr, nullptr, symbol(serv.client_id, free_node_id++, value));
}
