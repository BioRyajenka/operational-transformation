//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CLIENT_H
#define OT_VARIATION_CLIENT_H


#include <memory>
#include <stdexcept>
#include "../core/operation.h"
#include "../core/change.h"
#include "server_peer.h"

class client {
public:
    std::unique_ptr<document> doc;

    client(server &serv) : server(serv) {
        std::pair<std::unique_ptr<document>, int> p = server.connect(this);
        doc = p.first;
        last_known_server_state = p.second;
    }

private:
    server_peer server;
    // bridge from the latest known point in the server state
    //  to the latest point in the client state
//    operation bridge;
    // а буффер - это как bridge, но без on-flight операции. потому что ее в таком виде нет на сервере
    // буффер - это такая штука что (on-flight'+buffer) приведет в client-state.
    // причем on-flight'=то, что придет в ack

    std::shared_ptr<operation> in_flight = nullptr;

    // on-flight' (which is ack from server) + buffer = bridge
    std::shared_ptr<operation> buffer = nullptr;

    int last_known_server_state = -1;

//    static operation compose(const operation &target);

public:
    void apply_change(const int &pos, const change &change) {
        doc.apply(pos, change);

        if (in_flight) {
            if (buffer) {
                buffer->apply_change(pos, change);
            } else {
                buffer = std::make_shared<operation>(pos, change);
            }
        } else {
            in_flight = std::make_shared<operation>(pos, change);
            server.send(in_flight, last_known_server_state);
        }
    }

    // op is needed only for validation purposes
    void on_ack(const operation &op, const int &new_server_state) {
        if (op.hash() != in_flight->hash()) {
            throw std::runtime_error("Error validating acknowledged operation.");
        }

        last_known_server_state = new_server_state;

        if (buffer) {
            in_flight = buffer;
            server.send(in_flight, last_known_server_state);
            buffer = nullptr;
        }
    }

    void on_receive(const operation &op, const int &new_server_state) {
        // надо проверять что предок есть общий?

        last_known_server_state = new_server_state;

        if (in_flight) {
            const std::pair<operation, operation> &infl_transform = in_flight->transform(op);
            in_flight = infl_transform.second;

            if (buffer) {
                const std::pair<operation, operation> &buff_transform = buffer->transform(infl_transform.first);
                doc.apply(buff_transform.first);
                buffer = buff_transform.second;
            } else {
                doc.apply(infl_transform.first);
            }
        } else {
            if (buffer) {
                throw std::runtime_error("Invariant is not satisfied! in_flight==null -> buffer==null");
            }
            doc.apply(op)
        }
    }

//private:
//    static operation apply_change(const operation &target, const int &pos, const change &change);
};


#endif //OT_VARIATION_CLIENT_H
