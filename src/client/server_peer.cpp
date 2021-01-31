//
// Created by Igor on 31.01.2021.
//

#include "server_peer.h"

std::pair<std::shared_ptr<operation>, int> server_peer::connect(const std::shared_ptr<client> &client) {
    int last_known_state = 0; // load from scratch
    const auto &resp = serv->connect(std::make_shared<client_peer>(client), last_known_state);
    client_id = std::get<0>(resp);
    return std::make_pair(std::get<1>(resp), std::get<2>(resp));
}

void server_peer::send(const std::shared_ptr<operation> &op, const int &parent_state)  {
    if (parent_state == -1) {
        throw std::runtime_error("Forgot to load initial state!");
    }
    serv->on_receive(client_id, op, parent_state);
}

