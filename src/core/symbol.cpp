//
// Created by Igor on 02.02.2021.
//

#include <stdexcept>
#include "symbol.h"

symbol::symbol(const node_id_t &id, const int &value) : id(id), value(value) {}

symbol::symbol(const int &client_id, const int &node_id, const int &value) : value(value) {
    assert(client_id > 0);
    if (node_id >> 26) throw std::runtime_error("Id num is too large");
    if (client_id >> 6) throw std::runtime_error("Client id is too large");
    id = (client_id << 26) | node_id;
}

symbol symbol::initial = symbol(0, 999); // NOLINT(cert-err58-cpp)
