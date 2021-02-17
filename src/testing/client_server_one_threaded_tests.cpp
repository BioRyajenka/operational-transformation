//
// Created by Igor on 04.02.2021.
//

#include <ctime>
#include <vector>
#include <chrono>
#include <random>
#include <climits>
#include "util/magic_list.h"
#include "../server/server.h"
#include "../client/client.h"
#include "util/test_util.h"
#include "../server/history/simple_history.h"
#include "../server/history/jumping_history.h"

static const int TYPE_DELETE = 0;
static const int TYPE_INSERT = 1;
static const int TYPE_UPDATE = 2;

template<typename T>
void run_in_one_thread(
        const int initial_document_size,
        const int clients_num,
        const double producing_action_weight,
        const int simulation_time,
        const int first_client_connection_time
) {
    assert(clients_num > 0);
    assert(producing_action_weight > 0. && producing_action_weight <= 1.);
    assert(first_client_connection_time < simulation_time);
    if (clients_num >> 11) {
        printf("Max allowed clients number is %d! Increase limit in symbol.cpp\n", (1 << 11) - 1);
        exit(-1);
    }

    printf("Initializing...\n");
    std::shared_ptr<server> serv = std::make_shared<server>(initial_document_size, std::make_unique<T>());
    printf("Server created\n");
    auto serv_peer = std::make_shared<server_peer>(serv);

    std::vector<std::pair<client, magic_list<node_id_t>>> clients;
    clients.reserve(clients_num);
    for (int i = 0; i < clients_num; i++) {
        // emplace client and move magic_list
        clients.emplace_back(i + 1, magic_list<node_id_t>());

        auto &cl = clients[i].first;
        auto &ml = clients[i].second;

        ml.insert(symbol::initial.id);
        cl.set_operation_listener([&ml](const operation &op) {
            for (const auto &[node_id, ch] : op.get_insertions()) {
                ch.iterate([&ml](const auto s) { ml.insert(s.id); });
            }
            for (const auto [node_id, _] : op.get_deletions()) {
                ml.remove(node_id);
            }
        });
    }

    for (int i = 0; i < clients_num; i++) {
        clients[i].first.connect(serv_peer);
        printf("Client %d connected\n", clients[i].first.id());
    }
    clients[0].first.disconnect();
    printf("Client %d disconnected\n", clients[0].first.id());

    ll operations_produced = 0;

    std::uniform_int_distribution<int> value_generator(INT_MIN, INT_MAX);
    std::uniform_int_distribution<int> operation_type_generator(0, 2);
    std::uniform_real_distribution<double> dice(0., 1.);
    std::default_random_engine rand_engine;

    printf("Initialization done. Starting simulation\n");

    const auto &simulation_started = std::chrono::steady_clock::now();
    while (true) {
        const auto &current_time = std::chrono::steady_clock::now();
        const int seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - simulation_started
        ).count();
        if (seconds_elapsed > simulation_time) break;
        if (seconds_elapsed > first_client_connection_time && !serv->get_peer(clients[0].first.id())) {
            printf("Reconnecting client %d with %lld operations missed!\n", clients[0].first.id(), operations_produced);
            clients[0].first.connect(serv_peer);
            const auto &time_after_connection = std::chrono::steady_clock::now();
            const int connection_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                    time_after_connection - current_time
            ).count();
            printf("Connection took %.3lf seconds\n", (double) connection_time / 1000);
        }

        int events_to_consume = serv_peer->get_pending_queue_size();
        for (const auto &cl : clients) {
            const auto &cl_peer = serv->get_peer(cl.first.id());
            if (cl_peer) events_to_consume += cl_peer->get_pending_queue_size();
        }

        double dice_result = dice(rand_engine) * (producing_action_weight * clients_num + events_to_consume);
        if (dice_result < producing_action_weight * clients_num) {
            // producing action on one of the clients

            auto &[cl, ml] = clients[dice(rand_engine) * clients.size()];
            node_id_t random_node_id = ml.get_random();
            int op_type = random_node_id == symbol::initial.id ? TYPE_INSERT : operation_type_generator(rand_engine);

            if (op_type == TYPE_UPDATE) {
                cl.do_update(random_node_id, value_generator(rand_engine));
            } else if (op_type == TYPE_DELETE) {
                cl.do_delete(random_node_id);
            } else if (op_type == TYPE_INSERT) {
                cl.do_insert(random_node_id, value_generator(rand_engine));
            }

            operations_produced++;
        } else {
            // consuming on one of the clients or on the server

            bool client_processed = false;
            int roulette_result = (int) (dice(rand_engine) * events_to_consume);
            for (const auto &cl : clients) {
                const auto &cl_peer = serv->get_peer(cl.first.id());
                if (!cl_peer) continue;

                roulette_result -= cl_peer->get_pending_queue_size();
                if (cl_peer->get_pending_queue_size() && roulette_result <= 0) {
                    cl_peer->proceed_one_task();
                    client_processed = true;
                    break;
                }
            }

            if (!client_processed) {
                assert(serv_peer->get_pending_queue_size());
                serv_peer->proceed_one_task();
            }
        }
    }

    // === finalization ===
    clock_t finalization_started = clock();
    while (true) {
        bool modified = false;
        while (serv_peer->get_pending_queue_size()) {
            serv_peer->proceed_one_task();
            modified = true;
        }
        for (const auto &cl : clients) {
            const auto &cl_peer = serv->get_peer(cl.first.id());
            if (!cl_peer) continue;
            while (cl_peer->get_pending_queue_size()) {
                cl_peer->proceed_one_task();
                modified = true;
            }
        }
        if (!modified) break;
    }
    clock_t finalization_finished = clock();

    // === validating docs ===
    printf("Validation...\n");
    const auto &gauge = doc2vec(*clients[0].first.server_doc);
    for (int i = 1; i < clients_num; i++) {
        assert(check_vectors_equal(gauge, doc2vec(*clients[i].first.server_doc)));
    }
    printf("Validation done. Each doc size is %d\n", (int) gauge.size());

    // === ===
    printf("Each client produced %.3lf ops/sec at average\n", 1.0 * operations_produced / simulation_time / 20);
    printf("Final synchronization took %.8lf seconds\n",
           (double) (finalization_finished - finalization_started) / CLOCKS_PER_SEC);
}

int main() {
//    run_in_one_thread<simple_history>(100000, 20, 1., 10, 5);
    run_in_one_thread<jumping_history>(100000, 20, .99f, 10, 5);
    /**
     * With simple_history:
     *
     * для 20 клиентов
     * Each client produced 832.140 ops/sec at average
     * Final synchronization took 0.00100000 seconds
     *
     * для 2к клиентов
     * Each client produced 8.610 ops/sec at average
     * Final synchronization took 2.7730000 seconds
     *
     * With jumping history:
     * 58.285 ops/sec; 8.0 ops/sec respectively
     */
    return 0;
}
