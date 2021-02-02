//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CLIENT_PEER_H
#define OT_VARIATION_CLIENT_PEER_H


#include "../core/operation.h"
#include "../client/client.h"

class client_peer {
    client* cl;

public:
    client_peer(client* cl) : cl(cl) {}

    void on_ack(const operation& op, const int &new_server_state);

    void on_receive(const operation& op, const int &new_server_state);
};


#endif //OT_VARIATION_CLIENT_PEER_H
