//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_TESTS_H
#define OT_VARIATION_TESTS_H


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

class tests {
    std::thread thread;

public:
    client nested_client; // todo: init

    tests(int id, const server &server, const std::atomic<bool> &started) : thread([this, &started]() {
        const double mean = .2;
        const double variance = .15;
        std::uniform_real_distribution<double> dice(0, 1);
        std::uniform_real_distribution<double> sleep_rand(mean - variance, mean + variance);
        std::uniform_int_distribution<int> change_type_rand(0, 2);
        std::uniform_int_distribution<int> new_val_rand(INT_MIN, INT_MAX);
        std::default_random_engine rand_engine;

        while (started) {
            Sleep(sleep_rand(rand_engine));

            int pos = (int) (dice(rand_engine) * nested_client.doc.size());
            change change(
                    static_cast<change_type>(change_type_rand(rand_engine)),
                    new_val_rand(rand_engine)
            );

            nested_client.apply_change(pos, change);
        }
    }) {

    }

    void join() {
        thread.join();
    }
};

// todo: pass delay in argument?
std::vector<tests> start_n_clients(
        const int &clientsNum, const server &server, const std::atomic<bool> &started_flag
) {
    std::vector<tests> clients;
    for (int i = 0; i < clientsNum; i++) {
        clients.emplace_back(i, server, started_flag);
    }
    return clients;
}

void check_consistency(const server &server, const std::vector<tests> &clients) {
    int server_hash = server.doc.hash();
    int inconsistent_clients = std::accumulate(clients.begin(), clients.end(), 0,
                                               [&server_hash](const tests &client) {
                                                   return client.nested_client.doc.hash() != server_hash;
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
    auto serv = server{};
    auto clients = start_n_clients(clients_num, serv, started);

    Sleep(30000);
    started = false;
    std::for_each(clients.begin(), clients.end(), [](const tests &client) { client.join() });

    check_consistency(serv, clients);
}

// TODO: отдельно потестить кейс из доки
// TODO: лучше сначала тот же кейс, но маленькие его кусочки
// TODO: потом еще тест на отваливание клиента посредине, с проверкой consistency между ним и сервером
//          (??? это как? звучит сомнительно)
// TODO: и еще тест на отваливание клиента совсем надолго (оффлайн)

#endif //OT_VARIATION_TESTS_H
