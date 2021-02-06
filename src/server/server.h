//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_SERVER_H
#define OT_VARIATION_SERVER_H


#include <tuple>
#include <vector>
#include <memory>
#include <unordered_set>
#include "../core/operation.h"
#include "client_peer.h"
#include "history/operations_history.h"

class server {
private:
    std::unordered_map<int, std::shared_ptr<client_peer>> clients;
    std::shared_ptr<operations_history> history;

public:
    server(const int &initial_doc_size, const std::shared_ptr<operations_history> &history);

    std::pair<std::shared_ptr<operation>, int> connect(client* cl, const int &last_known_state);

    void disconnect(const int &client_id);

    void on_receive(const int &from_client_id, const std::shared_ptr<operation> &op, const int &parent_state);

    std::shared_ptr<client_peer> get_peer(const int &client_id) const;
};


#endif //OT_VARIATION_SERVER_H
