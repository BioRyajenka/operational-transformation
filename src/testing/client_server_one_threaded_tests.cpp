//
// Created by Igor on 04.02.2021.
//

#include <ctime>
#include <vector>
#include <chrono>
#include <random>
#include <string>
#include <climits>
#include "util/magic_list.h"
#include "../server/server.h"
#include "../client/client.h"

static const int TYPE_DELETE = 0;
static const int TYPE_INSERT = 1;
static const int TYPE_UPDATE = 2;

void run_in_one_thread(
        const int &initial_document_size,
        const int &clients_num, const double &producing_action_weight, const int &simulation_time
) {
    assert(producing_action_weight > 0. && producing_action_weight < 1.);
    if (clients_num >> 11) {
        printf("Max allowed clients is %d! Increase limit in symbol.cpp\n", clients_num);
        exit(-1);
    }

    printf("Initializing...\n");
    auto serv = std::make_shared<server>(initial_document_size);
    printf("Server created\n");
    auto serv_peer = std::make_shared<server_peer>(serv);

    std::vector<std::pair<client *, std::shared_ptr<magic_list<node_id_t>>>> clients(clients_num);
    for (int i = 0; i < clients_num; i++) {
        auto ml = std::make_shared<magic_list<node_id_t>>();
        ml->insert(symbol::initial.id);
        auto *cl = new client(serv_peer, [ml](const operation &op) {
            for (const auto &[node_id, ch] : *op.get_insertions()) {
                ch.iterate([ml](const auto &s) { ml->insert(s.id); });
            }
            for (const auto &[node_id, _] : *op.get_deletions()) {
                ml->remove(node_id);
            }

        });
        clients[i] = std::make_pair(cl, ml);

        printf("Client %d created\n", i);
    }

    ll operations_produced = 0;

    std::uniform_int_distribution<int> value_generator(INT_MIN, INT_MAX);
    std::uniform_int_distribution<int> operation_type_generator(0, 2);
    std::uniform_real_distribution<double> dice(0., 1.);
    std::default_random_engine rand_engine;

    printf("Initialization done. Starting simulation\n");

    std::chrono::steady_clock::time_point simulation_started = std::chrono::steady_clock::now();
    while (true) {
        std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
        const int seconds_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - simulation_started).count();
        if (seconds_elapsed > simulation_time) break;

        int events_to_consume = serv_peer->get_pending_queue_size();
        for (const auto &cl : clients) events_to_consume += serv->get_peer(cl.first->id())->get_pending_queue_size();

        double dice_result = dice(rand_engine) * (producing_action_weight * clients_num + events_to_consume);
        if (dice_result < producing_action_weight * clients_num) {
            // producing action on one of the clients

            auto &[cl, ml] = clients[dice(rand_engine) * clients.size()];
            node_id_t random_node_id = ml->get_random();
            int op_type = random_node_id == symbol::initial.id ? TYPE_INSERT : operation_type_generator(rand_engine);

            if (op_type == TYPE_UPDATE) {
                cl->do_update(random_node_id, value_generator(rand_engine));
            } else if (op_type == TYPE_DELETE) {
                cl->do_delete(random_node_id);
            } else if (op_type == TYPE_INSERT) {
                cl->do_insert(random_node_id, value_generator(rand_engine));
            }

            operations_produced++;
        } else {
            // consuming on one of the clients or on the server

            bool client_processed = false;
            int roulette_result = (int) (dice(rand_engine) * events_to_consume);
            for (const auto &cl : clients) {
                roulette_result -= serv->get_peer(cl.first->id())->get_pending_queue_size();
                if (serv->get_peer(cl.first->id())->get_pending_queue_size() && roulette_result <= 0) {
                    serv->get_peer(cl.first->id())->proceed_one_task();
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
//        printf("Server queue size: %d\n", serv_peer->get_pending_queue_size());
//        fflush(stdout);

        bool modified = false;
        while (serv_peer->get_pending_queue_size()) {
            serv_peer->proceed_one_task();
            modified = true;
        }
        for (const auto &cl : clients) {
            while (serv->get_peer(cl.first->id())->get_pending_queue_size()) {
                serv->get_peer(cl.first->id())->proceed_one_task();
                modified = true;
            }
        }
        if (!modified) break;
    }
    clock_t finalization_finished = clock();

    // === validating docs ===
    printf("Validation...\n");
//    ll gauge = clients[0].first->server_doc->hash();
//    for (int i = 1; i < (int) clients.size(); i++) {
//        assert(clients[i].first->server_doc->hash() == gauge);
//    }
    const auto &gauge = doc2vec(*clients[0].first->server_doc);
    for (int i = 1; i < clients_num; i++) {
        assert(check_vectors_equal(gauge, doc2vec(*clients[i].first->server_doc)));
    }
    printf("Validation done. Each doc size is %d\n", (int) gauge.size());

    // === clean up ===
    for (const auto &cl : clients) delete cl.first;

    // === ===
    printf("Each client produced %.3lf ops/sec at average\n", 1.0 * operations_produced / simulation_time /  20);
    printf("Final synchronization took %.8lf seconds\n", (double) (finalization_finished - finalization_started) / CLOCKS_PER_SEC);
}

int main() {
    run_in_one_thread(1000, 2000, .99f, 10);
    /**
     * для 20 клиентов
     * Each client produced 832.140 ops/sec at average
     * Final synchronization took 0.00100000 seconds
     */
    /**
     * для 2к клиентов
     * Each client produced 8.610 ops/sec at average
     * Final synchronization took 2.7730000 seconds
     */
//    run_in_one_thread(0, 2, .5f, 10);
    return 0;
}
