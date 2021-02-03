//
// Created by Igor on 30.01.2021.
//

#include "server.h"

std::tuple<int, std::shared_ptr<operation>, int> server::connect(
        const std::shared_ptr<client_peer> &client, const int &last_known_state
) {
    clients.push_back(client);
    int client_id = (int) clients.size() - 1;

    const std::shared_ptr<operation> &op = history->fetch(last_known_state);

    return std::make_tuple(client_id, op, history->last_state());
}

void server::on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state) {
    // TODO:
    //  !!! на самом деле, если от клиента прилетает "удали", а на сервере идет
    //  трансформация с другим "добавь после удаленной", проблемы нет. Можно
    //  соптимизить так, чтобы такая команда все равно выполнялась, просто возвращала бы
    //  только одну сторону функции (т.к. вторая серверу не нужна).

    std::shared_ptr<operation> appl;

    if (history->last_state() == parent_state) {
        appl = op;
    } else {
        const std::shared_ptr<operation> &since = history->fetch(parent_state);
        op->detach_unprocessable_by_server(*since->get_deletions());
        appl = op->transform(*since, nullptr, true).second;
    }

    if (!appl->get_deletions()->empty() && !appl->get_insertions()->empty() && !appl->get_updates()->empty()) {
        history->push(appl);
    }

    for (auto i = 0; i < clients.size(); i++) {
        if (i == from_client_id - 1) {
            clients[i]->on_ack(*appl, history->last_state());
        } else {
            clients[i]->on_receive(*appl, history->last_state());
        }
    }
}
