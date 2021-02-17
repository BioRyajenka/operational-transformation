//
// Created by Igor on 30.01.2021.
//

#include "server.h"
#include "../client/client.h"

server::server(const int& initial_doc_size, std::unique_ptr<operations_history> &&history) : _history(std::move(history)) {
    std::shared_ptr<operation> op = std::make_shared<operation>();
    chain c = chain(symbol(0, 1, 1));
    for (int i = 2; i <= initial_doc_size; i++) {
        c.move_to_the_end(chain(symbol(0, i, i)));
    }
    // it will be 999->1->2->3->...
    op->insert(symbol::initial.id, c);
    _history->push(op);
}

std::pair<std::unique_ptr<operation>, int> server::connect(client* cl, const int &last_known_state) {
    assert(!_clients.count(cl->id()));
    _clients.emplace(cl->id(), std::make_unique<client_peer>(cl));
    auto op = _history->fetch(last_known_state);

    return std::make_pair(std::move(op), _history->last_state());
}

void server::disconnect(const int &client_id) {
    const auto &cl = _clients.find(client_id);

    assert(cl != _clients.end());
    const auto &cl_peer = cl->second;
    while (cl_peer->get_pending_queue_size()) cl_peer->proceed_one_task();

    _clients.erase(client_id);
}

void server::on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state) {
    std::shared_ptr<operation> appl;

    if (_history->last_state() == parent_state) {
        appl = op;
    } else {
        const std::unique_ptr<operation> &since = _history->fetch(parent_state);
        appl = op->transform(*since, true).second;
    }

    if (!appl->empty()) {
        _history->push(appl);
    }

    for (const auto &[cl_id, cl] : _clients) {
        if (cl_id == from_client_id) {
            cl->send_ack(appl, _history->last_state());
        } else {
            cl->send_update(appl, _history->last_state());
        }
    }
}

std::shared_ptr<client_peer> server::get_peer(const int &client_id) const {
    auto cl = _clients.find(client_id);
    return cl == _clients.end() ? nullptr : cl->second;
}
