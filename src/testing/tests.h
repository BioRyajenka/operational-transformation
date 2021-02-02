//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_TESTS_H
#define OT_VARIATION_TESTS_H

/*
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <string>
#include <climits>
#include <numeric>
#include <windows.h>
#include <stdexcept>
#include "../client/client.h"
#include "../server/server.h"
#include "test_document.h"

static const int TYPE_DELETE = 0;
static const int TYPE_INSERT = 1;
static const int TYPE_UPDATE = 2; // update doesn't create new node

class client_runner {
    std::thread thread;

public:
    client nested_client;
//    const std::shared_ptr<test_document> doc = std::make_shared<test_document>();
// тут можно док не создавать, т.к. в клиенте уже есть
    const std::shared_ptr<test_document> gauge;

    client_runner(
            const std::shared_ptr<server> &serv,
            const std::shared_ptr<test_document> &gauge,
            const std::atomic<bool> &started
    ) : nested_client(doc, serv), gauge(gauge) {

        thread = std::thread([this, &started, &gauge]() {
            const double mean = .2;
            const double variance = .15;
            std::uniform_real_distribution<double> sleep_rand(mean - variance, mean + variance);
            std::uniform_int_distribution<int> change_type_rand(0, 2);
            std::uniform_int_distribution<int> new_val_rand(INT_MIN, INT_MAX);
            std::default_random_engine rand_engine;

            while (started) {
                Sleep(sleep_rand(rand_engine));

                const auto &node_id = doc->get_random_node_id();
                const int &type = change_type_rand(rand_engine);
                const node<symbol> *initial_node = type == TYPE_INSERT || type == TYPE_UPDATE
                                                   ? nested_client.generate_node(new_val_rand(rand_engine))
                                                   : nullptr;

                auto op = std::make_shared<operation>(
                        node_id, type == TYPE_DELETE || type == TYPE_UPDATE, initial_node
                );

                nested_client.apply_user_op(op);
                // also apply to gauge
                gauge->apply(*op);
            }
        });
    }

    void join() {
        thread.join();
    }
};

// todo: pass delay in argument?
std::vector<client_runner> start_n_clients(
        const int &clientsNum,
        const std::shared_ptr<server> &server,
        const std::shared_ptr<test_document> &gauge,
        const std::atomic<bool> &started_flag
) {
    std::vector<client_runner> clients;
    for (int i = 0; i < clientsNum; i++) {
        clients.emplace_back(server, gauge, started_flag);
    }
    return clients;
}

void check_consistency(
        const std::shared_ptr<test_document> &gauge, const std::vector<client_runner> &clients
) {
    int inconsistent_clients = std::accumulate(clients.begin(), clients.end(), 0,
                                               [&gauge](const client_runner &client) {
                                                   return client.doc->hash() != gauge->hash();
                                               });
    if (inconsistent_clients) {
        throw std::runtime_error(
                "Found " + std::to_string(inconsistent_clients) + "inconsistent clients!"
        );
    }
}

void test() {
    int clients_num = 20;

    std::atomic<bool> started(true);
    auto serv = std::make_shared<server>();
    auto gauge = std::make_shared<test_document>();
    auto clients = start_n_clients(clients_num, serv, gauge, started);

    Sleep(30000);
    started = false;
    std::for_each(clients.begin(), clients.end(), [](client_runner &client) { client.join(); });

    check_consistency(gauge, clients);
}

// TODO: отдельно потестить кейс из доки
// TODO: лучше сначала тот же кейс, но маленькие его кусочки
// TODO: потом еще тест на отваливание клиента посредине, с проверкой consistency между ним и сервером
//          (??? это как? звучит сомнительно)
// TODO: и еще тест на отваливание клиента совсем надолго (оффлайн)


 */
#endif //OT_VARIATION_TESTS_H
