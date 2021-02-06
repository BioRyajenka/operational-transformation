//
// Created by Igor on 30.01.2021.
//

#include "server.h"
#include "../client/client.h"

server::server(const int& initial_doc_size, const std::shared_ptr<operations_history> &history) : history(history) {
    std::shared_ptr<operation> op = std::make_shared<operation>();
    chain c = chain(symbol(0, 1, 1));
    for (int i = 2; i <= initial_doc_size; i++) {
        c.move_to_the_end(chain(symbol(0, i, i)));
    }
    // it will be 999->1->2->3->...
    op->insert(symbol::initial.id, c);
    history->push(op);
}

std::pair<std::shared_ptr<operation>, int> server::connect(client* cl, const int &last_known_state) {
    assert(!clients.count(cl->id()));
    clients.emplace(cl->id(), std::make_shared<client_peer>(cl));
    const std::shared_ptr<operation> &op = history->fetch(last_known_state);

    return std::make_pair(op, history->last_state());
}

void server::disconnect(const int &client_id) {
    const auto &cl = clients.find(client_id);

    assert(cl != clients.end());
    const auto &cl_peer = cl->second;
    while (cl_peer->get_pending_queue_size()) cl_peer->proceed_one_task();

    clients.erase(client_id);
}

void server::on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state) {
    std::shared_ptr<operation> appl;

    if (history->last_state() == parent_state) {
        appl = op;
    } else {
        const std::shared_ptr<operation> &since = history->fetch(parent_state);
        appl = op->transform(*since, true).second;
    }

    if (!appl->get_deletions()->empty() || !appl->get_insertions()->empty() || !appl->get_updates()->empty()) {
        history->push(appl);
    }

    for (const auto &[cl_id, cl] : clients) {
        if (cl_id == from_client_id) {
            cl->send_ack(appl, history->last_state());
        } else {
            cl->send_update(appl, history->last_state());
        }
    }
}

std::shared_ptr<client_peer> server::get_peer(const int &client_id) const {
    auto cl = clients.find(client_id);
    return cl == clients.end() ? nullptr : cl->second;
}
