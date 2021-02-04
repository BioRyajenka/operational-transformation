//
// Created by Igor on 31.01.2021.
//

#include "client_peer.h"
#include "../client/client.h"

client_peer::client_peer(client *cl) : cl(cl) {}

void client_peer::send_ack(const std::shared_ptr<operation> &op, const int &new_server_state) {
    queue.push(std::make_tuple(task_type::ACK, op, new_server_state));
}

void client_peer::send_update(const std::shared_ptr<operation> &op, const int &new_server_state) {
    queue.push(std::make_tuple(task_type::RECEIVE, op, new_server_state));
}

void client_peer::proceed_one_task() {
    const auto &[type, op, new_server_state] = queue.pop();

    if (type == task_type::ACK) cl->on_ack(*op, new_server_state);
    else cl->on_receive(*op, new_server_state);
}

int client_peer::get_pending_queue_size() const {
    return queue.size();
}
