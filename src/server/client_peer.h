//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_CLIENT_PEER_H
#define OT_VARIATION_CLIENT_PEER_H


#include "../core/operation.h"
#include "../core/blocking_queue.h"

class client;

class client_peer {
private:
    enum task_type { ACK, RECEIVE };
    typedef std::tuple<task_type, std::shared_ptr<operation>, int> client_peer_task;

    client *cl;
    blocking_queue<client_peer_task> queue;

public:
    explicit client_peer(client *cl);
    client_peer(const client_peer &) = delete;
    client_peer(client_peer &&) = delete;

    void send_ack(const std::shared_ptr<operation> &op, const int &new_server_state);

    void send_update(const std::shared_ptr<operation> &op, const int &new_server_state);

    // blocking method
    void proceed_one_task();

    int get_pending_queue_size() const;
};


#endif //OT_VARIATION_CLIENT_PEER_H
