//
// Created by Igor on 04.02.2021.
//

#include <ctime>
#include <vector>
#include <chrono>
#include <random>
#include <string>
//#include "util/doctest.h"
#include "util/magic_list.h"
#include "../server/server.h"
#include "../client/client.h"
#include "util/test_util.h"

static const int TYPE_DELETE = 0;
static const int TYPE_INSERT = 1;
static const int TYPE_UPDATE = 2;

void run_in_one_thread(
        const int &initial_document_size, // TODO
        const int &clients_num, const double &producing_action_weight, const int &simulation_time
) {
    assert(producing_action_weight > 0. && producing_action_weight < 1.);

    auto serv = std::make_shared<server>();
    auto serv_peer = std::make_shared<server_peer>(serv);

    std::vector<std::pair<client *, std::shared_ptr<magic_list<node_id_t>>>> clients(clients_num);
    for (int i = 0; i < clients_num; i++) {
        auto ml = std::make_shared<magic_list<node_id_t>>();
        ml->insert(symbol::initial.id);
        auto *cl = new client(serv_peer, [ml, i, &clients](const operation &op) {
//            print_operation("client " + std::to_string(i + 1) + " applies operation", op);
            for (const auto &[node_id, ch] : *op.get_insertions()) {
                chain tmp = ch;
                ch.iterate([ml, &tmp, &op](const auto &s) {
                    if (ml->exists(s.id)) {
//                        print_vector(ml->to_vector());
//                        print_chain("ch", tmp);
//                        print_operation("op", op);
                        exit(0);
//                    } else if (s.id == 134217728) {
//                        print_vector(ml->to_vector());
//                        print_chain("ch", tmp);
//                        print_operation("op", op);
//                        printf("\n");
                    }
                    ml->insert(s.id);
                });
            }
            for (const auto &node_id : *op.get_deletions()) {
                ml->remove(node_id);
            }

//            printf("his ml after that: ");
//            print_vector(ml->to_vector());

//            const auto &tmp3 = op.get_insertions()->find(0);
//            if (tmp3 != op.get_insertions()->end() && tmp3->second.get_head()->value.id == 67108865) {
//                printf("ml1: ");
//                print_vector(clients[0].second->to_vector());
//                printf("ml2: ");
//                print_vector(clients[1].second->to_vector());
//            }
        });
        clients[i] = std::make_pair(cl, ml);
    }

    ll operations_produced = 0;

    std::uniform_int_distribution<int> value_generator(INT_MIN, INT_MAX);
    std::uniform_int_distribution<int> operation_type_generator(0, 2);
    std::uniform_int_distribution<int> consumer_roulette(0, clients.size()); // 0 will be the server
    std::uniform_real_distribution<double> dice(0., 1.);
    std::default_random_engine rand_engine;

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
            auto op = std::make_shared<operation>();
            node_id_t random_node_id = ml->get_random();
            int op_type = random_node_id == symbol::initial.id ? TYPE_INSERT : operation_type_generator(rand_engine);

            if (op_type == TYPE_UPDATE) {
                op->update(random_node_id, value_generator(rand_engine));
            } else if (op_type == TYPE_DELETE) {
                op->del(random_node_id, nullptr);
            } else if (op_type == TYPE_INSERT) {
                op->insert(random_node_id, chain(cl->generate_symbol(value_generator(rand_engine))));
            }

//            printf("client %d arises operation", cl->id());
//            print_operation("", *op);

            cl->apply_user_op(op);
            operations_produced++;
        } else {
            // consuming on one of the clients or on the server

            int initial_roulette_result = consumer_roulette(rand_engine);
            int roulette_result = initial_roulette_result;
            while (roulette_result == 0 && !serv_peer->get_pending_queue_size()
                   || roulette_result && !serv->get_peer(clients[roulette_result - 1].first->id())->get_pending_queue_size()) {
                roulette_result = (roulette_result + 1) % (clients_num + 1);
                assert(roulette_result != initial_roulette_result);
            }

            if (roulette_result == 0) {
//                printf("server consumes operation\n");

                // serv
                assert(serv_peer->get_pending_queue_size());
                serv_peer->proceed_one_task();
            } else {
//                printf("client %d consumes operation\n", roulette_result);

                assert(serv->get_peer(clients[roulette_result - 1].first->id())->get_pending_queue_size());
                serv->get_peer(clients[roulette_result - 1].first->id())->proceed_one_task();
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
            while (serv->get_peer(cl.first->id())->get_pending_queue_size()) {
                serv->get_peer(cl.first->id())->proceed_one_task();
                modified = true;
            }
        }
        if (!modified) break;
    }
    clock_t finalization_finished = clock();

    // === validating docs ===
    ll gauge = clients[0].first->server_doc->hash();
    assert(clients[0].first->server_doc_plus_infl->hash() == gauge);
    for (int i = 1; i < (int) clients.size(); i++) {
        assert(clients[i].first->server_doc->hash() == gauge);
        assert(clients[i].first->server_doc_plus_infl->hash() == gauge);
    }

    // === ===
    printf("Each client produced %.3lf ops/sec at average\n", 1.0 * operations_produced / simulation_time /  20);
    printf("Final synchronization took %.8lf seconds", 1. * (finalization_finished - finalization_started) / CLOCKS_PER_SEC);
}

int main() {
    run_in_one_thread(0, 2, .5f, 10);
    return 0;
}

//TEST_CASE ("2 clients, consuming events twice more often than producing") {
//    run_in_one_thread(0, 1, .5f, 10);
//}

//TEST_CASE ("2 clients, as fast as possible") {
//
//}
//
//TEST_CASE ("20 clients, as fast as possible") {
//
//}
