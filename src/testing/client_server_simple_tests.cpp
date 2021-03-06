//
// Created by Igor on 03.02.2021.
//

#include "util/doctest.h"
#include "../client/client.h"
#include "../server/server.h"
#include "util/test_util.h"
#include "../server/history/simple_history.h"

/*TEST_CASE("client-server simple scenarios") {
    auto serv = std::make_shared<server>(0, std::make_unique<simple_history>());
    auto serv_peer = std::make_shared<server_peer>(serv);
    auto* cl1 = new client(1);
    auto* cl2 = new client(2);
    cl1->connect(serv_peer);
    cl2->connect(serv_peer);

    SUBCASE("clients should have correct ids") {
        CHECK(cl1->id() == 1);
        CHECK(cl2->id() == 2);
    }

    SUBCASE("clients download initial data correctly") {
        CHECK(check_doc_ids(*cl1->server_doc, {0, 1, 2, 3}));
        CHECK(check_doc_ids(*cl2->server_doc, {0, 1, 2, 3}));
    }

    SUBCASE("client sees other client's changes") {
        auto op = std::make_unique<operation>();
        op->insert(2, create_chain({4, 5}));
        cl1->apply_user_op(op);
        CHECK(check_doc_ids(*cl1->server_doc, {0, 1, 2, 3}));

        serv_peer->proceed_one_task(); // sends ack to client1 and update to client2

        serv->get_peer(cl1->id())->proceed_one_task(); // ack
        CHECK(check_doc_ids(*cl1->server_doc, {0, 1, 2, 4, 5, 3}));

        serv->get_peer(cl2->id())->proceed_one_task(); // update
        CHECK(check_doc_ids(*cl2->server_doc, {0, 1, 2, 4, 5, 3}));
    }

    SUBCASE("client 2 sends delete while there is unreceived insert from server") {
        auto first = std::make_unique<operation>();
        first->insert(2, create_chain({4, 5}));
        cl1->apply_user_op(first);
        serv_peer->proceed_one_task();
        // now 1 task pending on cl1
        // and 1 on cl2

        auto second = std::make_unique<operation>();
        second->del(2, 1);
        cl2->apply_user_op(second);
        serv_peer->proceed_one_task();
        // now 2 tasks pending on cl1 and cl2

        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();
        CHECK(check_doc_ids(*cl1->server_doc, {0, 1, 4, 5, 3}));
        CHECK(check_doc_ids(*cl2->server_doc, {0, 1, 4, 5, 3}));
    }

    SUBCASE("client 2 sends insert while there is unreceived delete from server") {
        auto first = std::make_unique<operation>();
        first->del(2, 1);
        cl1->apply_user_op(first);
        serv_peer->proceed_one_task();
        // now 1 task pending on cl1
        // and 1 on cl2

        auto second = std::make_unique<operation>();
        second->insert(2, create_chain({6}));
        cl2->apply_user_op(second);
        serv_peer->proceed_one_task();
        // now 2 tasks pending on cl1 and cl2

        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();

        CHECK(check_doc_ids(*cl1->server_doc, {0, 1, 6, 3}));
        CHECK(check_doc_ids(*cl2->server_doc, {0, 1, 6, 3}));
    }

    SUBCASE("cross deleted insertions 1") {
        auto first = std::make_unique<operation>();
        first->insert(1, create_chain({6}));
        first->del(2, 1);
        cl1->apply_user_op(first);
        serv_peer->proceed_one_task();
        // now 1 task pending on cl1
        // and 1 on cl2

        auto second = std::make_unique<operation>();
        second->insert(2, create_chain({4, 5}));
        second->del(1, 0);
        cl2->apply_user_op(second);
        serv_peer->proceed_one_task();
        // now 2 tasks pending on cl1 and cl2

        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl1->id())->proceed_one_task();
//        print_doc("cl1", *cl1->server_doc);
        serv->get_peer(cl2->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();

        CHECK(check_doc_ids(*cl1->server_doc, {0, 6, 4, 5, 3}));
        CHECK(check_doc_ids(*cl2->server_doc, {0, 6, 4, 5, 3}));
    }

    SUBCASE("cross deleted insertions 2 (swapped)") {
        auto first = std::make_unique<operation>();
        first->insert(2, create_chain({4, 5}));
        first->del(1, 0);
        cl1->apply_user_op(first);
        serv_peer->proceed_one_task();
        // now 1 task pending on cl1
        // and 1 on cl2

        auto second = std::make_unique<operation>();
        second->insert(1, create_chain({6}));
        second->del(2, 1);
        cl2->apply_user_op(second);
        serv_peer->proceed_one_task();
        // now 2 tasks pending on cl1 and cl2

        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl1->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();
        serv->get_peer(cl2->id())->proceed_one_task();

//        printf("queue serv: %d\n", serv_peer->get_pending_queue_size());
//        printf("queue1: %d\n", serv->get_peer(cl1->id())->get_pending_queue_size());
//        printf("queue2: %d\n", serv->get_peer(cl2->id())->get_pending_queue_size());

        CHECK(check_doc_ids(*cl1->server_doc, {0, 6, 4, 5, 3}));
        CHECK(check_doc_ids(*cl2->server_doc, {0, 6, 4, 5, 3}));
    }
}*/
