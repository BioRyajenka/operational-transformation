//
// Created by Igor on 31.01.2021.
//

#include "client_peer.h"
#include "../client/client.h"

client_peer::client_peer(client *cl) : cl(cl) {}

void client_peer::on_ack(const operation &op, const int &new_server_state) {
    cl->on_ack(op, new_server_state);
}

void client_peer::on_receive(const operation &op, const int &new_server_state) {
    cl->on_receive(op, new_server_state);
}

//client_peer::client_peer(const std::shared_ptr<client> &cl): cl(cl) {}
