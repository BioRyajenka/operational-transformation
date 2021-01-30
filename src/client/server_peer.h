//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SERVER_PEER_H
#define OT_VARIATION_SERVER_PEER_H


#include "../core/operation.h"
#include "../server/server.h"
#include "client.h"

class server_peer {
private:
    int id = -1; // receive id upon connect
    std::shared_ptr<server> server;

public:
    server_peer(const std::shared_ptr<server> &server) : server(server) {}

    std::pair<std::unique_ptr<document>, int> connect(const std::shared_ptr<client> &client) {
        id = server->connect(std::make_shared<client_peer>(client));
        return server->download_document();
    }

    void send(const std::shared_ptr<operation> &op, const int &parent_state) {
        if (parent_state == -1) {
            throw std::runtime_error("Forgot to load initial state!");
        }
        server->on_receive(id, op, parent_state);
    }
};


#endif //OT_VARIATION_SERVER_PEER_H
