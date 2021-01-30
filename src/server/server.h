//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SERVER_H
#define OT_VARIATION_SERVER_H


#include <vector>
#include <memory>
#include "../core/document.h"
#include "../core/operation.h"
#include "client_peer.h"
#include "history/operations_history.h"
#include "history/simple_history.h"

/**
 * в обычном случае:
 * от клиента приходит операция с корнем в той операции, которую он последней получал
 * смотрим че случилось после нее, все комбиним в одну. делаем ot, чето отправляем назад
 *
 * Вариант 0: храним все операции (история же)
 *   write       : fast
 *   read online : medium
 *   read offline: slow
 *
 * Вариант 1: для каждого клиента поддерживаем компрессию всех событий, которые были после того
 * что он знает
 *   write       : slow, O(tC). t - время функции transform, C - кол-во клиентов
 *   read online : fast
 *   read offline: fast
 *
 * Вариант 2: предполагая, что одновременно работают немного (k), для всех онлайн-чуваков
 * поддерживаем компрессию, для остальных чето умное делаем
 *   write       : medium, O(tk)
 *   read online : fast
 *   read offline: ???
 *
 * Вариант 3: сжатие осуществляем скип-листом (т.к. операция ассоциативна)
 *   write       : fast
 *   read online : fast-medium
 *   read offline: fast
 *
 * от клиента всегда приходит операция с корнем в той операции, которую он последней получал,
 * то есть, сервер знает что примерно пришлет клиент.
 *
 *
 * для атомарности read-write-lock или просто очередь?
 */

class server {
private:
    std::vector<client_peer> clients;
    std::unique_ptr<operations_history> history = std::make_unique<simple_history>();

public:
    document doc;

    int connect(const client_peer &client) {
        clients.push_back(std::move(client));
        return (int) clients.size() - 1;
    }

    std::pair<std::unique_ptr<document>, int> download_document() {
        return std::make_pair<std::unique_ptr<document>, int>(
                std::make_shared<document>(doc),
                history->last_state()
        );
    }

    void on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state) {
        std::shared_ptr<operation> appl;

        if (history->last_state() == parent_state) {
            appl = op;
        } else {
            const std::shared_ptr<operation> &since = history->fetch(parent_state);
            const std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> &p = op->transform(*since);
            appl = p.second;
        }

        doc.apply(*appl); // does it copy here?
        history->push(appl);

        for (auto i = 0; i < clients.size(); i++) {
            if (i == from_client_id) {
                // the op itself is sent only for validation
                clients[i].on_ack(*appl, history->last_state());
            } else {
                clients[i].on_receive(*appl, history->last_state());
            }
        }
    }
};


#endif //OT_VARIATION_SERVER_H
