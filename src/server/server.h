//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SERVER_H
#define OT_VARIATION_SERVER_H


#include <tuple>
#include <vector>
#include <memory>
#include <unordered_set>
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
    std::vector<std::shared_ptr<client_peer>> clients;
    std::unique_ptr<operations_history> history = std::make_unique<simple_history>();

public:
    server();

    std::tuple<int, std::shared_ptr<operation>, int> connect(
            client* cl, const int &last_known_state
    );

    void on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state);

    std::shared_ptr<client_peer> get_peer(const int &client_id);
};


#endif //OT_VARIATION_SERVER_H
