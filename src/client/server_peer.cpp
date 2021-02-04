//
// Created by Igor on 31.01.2021.
//

#include "server_peer.h"
#include "../server/server.h"

server_peer::server_peer(const std::shared_ptr<server> &serv) : serv(serv) {}

std::tuple<int, std::shared_ptr<operation>, int> server_peer::connect(client *client) {
    int last_known_state = 0; // load from scratch
    return serv->connect(client, last_known_state);
}

void server_peer::send(const int& client_id, const operation &op, const int &parent_state) {
    if (parent_state == -1) {
        throw std::runtime_error("Forgot to load initial state!");
    }
    std::shared_ptr<operation> copy = std::make_shared<operation>(op);
    queue.push(std::make_tuple(client_id, copy, parent_state));
}

void server_peer::proceed_one_task() {
    const auto&[client_id, op, parent_state] = queue.pop();
    serv->on_receive(client_id, op, parent_state);
}

int server_peer::get_pending_queue_size() const {
    return queue.size();
}

