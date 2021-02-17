//
// Created by Igor on 31.01.2021.
//

#include "server_peer.h"
#include <utility>
#include "../server/server.h"

server_peer::server_peer(std::shared_ptr<server> serv) : serv(std::move(serv)) {}

std::pair<std::unique_ptr<operation>, int> server_peer::connect(client *client, int last_known_state) {
    return serv->connect(client, last_known_state);
}

void server_peer::disconnect(int client_id) {
    serv->disconnect(client_id);
}

void server_peer::send(const int client_id, const std::shared_ptr<operation> &op, int parent_state) {
    if (parent_state == -1) {
        throw std::runtime_error("Forgot to load initial state!");
    }
    queue.push(std::make_tuple(client_id, op, parent_state));
}

void server_peer::proceed_one_task() {
    const auto &[client_id, op, parent_state] = queue.pop();
    serv->on_receive(client_id, op, parent_state);
}

int server_peer::get_pending_queue_size() const {
    return queue.size();
}

