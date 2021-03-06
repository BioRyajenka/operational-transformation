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
    std::unordered_map<int, std::shared_ptr<client_peer>> _clients;
    std::unique_ptr<operations_history> _history;

public:
    server(int initial_doc_size, std::unique_ptr<operations_history> &&history);

    std::pair<std::unique_ptr<operation>, int> connect(client* cl, int last_known_state);

    void disconnect(int client_id);

    void on_receive(int from_client_id, const std::shared_ptr<operation> &op, int parent_state);

    std::shared_ptr<client_peer> get_peer(int client_id) const;
};


#endif //OT_VARIATION_SERVER_H
