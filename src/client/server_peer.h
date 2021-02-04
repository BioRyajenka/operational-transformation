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
    typedef std::tuple<int, std::shared_ptr<operation>, int> server_peer_task;

    std::shared_ptr<server> serv;
    blocking_queue<server_peer_task> queue;

public:
    server_peer(const std::shared_ptr<server> &serv);

    std::tuple<int, std::shared_ptr<operation>, int> connect(client *client);

    // operation will be copied
    void send(const int& client_id, const operation &op, const int &parent_state);

    // blocking method
    void proceed_one_task();

    int get_pending_queue_size() const;
};


#endif //OT_VARIATION_SERVER_PEER_H
