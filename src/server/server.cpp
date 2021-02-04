//
// Created by Igor on 30.01.2021.
//

#include "server.h"

server::server(const int& initial_doc_size) {
    std::shared_ptr<operation> op = std::make_shared<operation>();
    chain c = chain(symbol(0, 1, 1));
    for (int i = 2; i <= initial_doc_size; i++) {
        c.move_to_the_end(chain(symbol(0, i, i)));
    }
    // it will be 999->1->2->3->...
    op->insert(symbol::initial.id, c);
    history->push(op);
}

std::tuple<int, std::shared_ptr<operation>, int> server::connect(
        client* cl, const int &last_known_state
) {
    clients.emplace_back(std::make_shared<client_peer>(cl));
//clients.push_back(client_peer(client));
    int client_id = (int) clients.size(); // 0th client is a server

    const std::shared_ptr<operation> &op = history->fetch(last_known_state);

    return std::make_tuple(client_id, op, history->last_state());
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

    for (auto i = 0; i < clients.size(); i++) {
        if (i == from_client_id - 1) {
            clients[i]->send_ack(appl, history->last_state());
        } else {
            clients[i]->send_update(appl, history->last_state());
        }
    }
}

std::shared_ptr<client_peer> server::get_peer(const int &client_id) {
    return clients.at(client_id - 1);
}
