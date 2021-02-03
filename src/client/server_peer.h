//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SERVER_PEER_H
#define OT_VARIATION_SERVER_PEER_H


#include "../core/operation.h"
#include "document.h"
#include "../server/client_peer.h"

class server;
class client;

class server_peer {
private:
    std::shared_ptr<server> serv;

public:
    int client_id = -1; // receive id upon connect

    server_peer(const std::shared_ptr<server> &serv);

    std::pair<std::shared_ptr<operation>, int> connect(client* client);

    void send(const std::shared_ptr<operation> &op, const int &parent_state);
};


#endif //OT_VARIATION_SERVER_PEER_H
