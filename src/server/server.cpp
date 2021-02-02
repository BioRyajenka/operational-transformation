//
// Created by Igor on 30.01.2021.
//

#include "server.h"

//std::tuple<int, std::shared_ptr<document>, int> connect(const std::shared_ptr<client_peer> &client) {
//    clients.push_back(client);
//    int client_id = (int) clients.size();
//
//    auto document_copy = std::make_shared(document);
//
//    return std::make_tuple(client_id, document_copy, history->last_state());
//}

std::tuple<int, std::shared_ptr<operation>, int>
server::connect(const std::shared_ptr<client_peer> &client, const int &last_known_state) {
    clients.push_back(client);
    int client_id = (int) clients.size() - 1;

    const std::shared_ptr<operation> &op = history->fetch(last_known_state);

    return std::make_tuple(client_id, op, history->last_state());
}

std::shared_ptr<operation> server::apply(const std::shared_ptr<operation> &op, const int &parent_state) {
    if (op->lists.empty()) return op;

    std::shared_ptr<operation> appl;

    if (history->last_state() == parent_state) {
        appl = op;
    } else {
        const std::shared_ptr<operation> &since = history->fetch(parent_state);
        appl = op->transform(*since).second;
    }

    history->push(appl);

    return appl;
}

void server::on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state) {
    // TODO:
    //  !!! на самом деле, если от клиента прилетает "удали", а на сервере идет
    //  трансформация с другим "добавь после удаленной", проблемы нет. Можно
    //  соптимизить так, чтобы такая команда все равно выполнялась, просто возвращала бы
    //  только одну сторону функции (т.к. вторая серверу не нужна).
    //   .
    //   .
    //   V нормально ли что я тут с document_ids сравниваюсь?

    // check all ids are valid
    std::shared_ptr<operation> best_effort_op = std::make_shared<operation>();
    std::vector<int> conflicting_ids;
    for (const auto& [key, value] : op->lists) {
        // if adding or updating based on deleted id
        if (!document_ids.count(key)) {
            if (value.second->get_root()) {
                conflicting_ids.push_back(key);
            }
        } else {
            best_effort_op->lists[key] = value;
        }
    }

    // TODO: best_effort_op now can be empty
    const auto &appl = apply(best_effort_op, parent_state);

    for (auto i = 0; i < clients.size(); i++) {
        if (i == from_client_id - 1) {
            if (conflicting_ids.empty()) {
                // the op itself is sent only for validation
                clients[i]->on_ack(*appl, history->last_state());
            } else {
                clients[i]->on_recover(*appl, history->last_state());
            }
        } else {
            clients[i]->on_receive(*appl, history->last_state());
        }
    }
}
