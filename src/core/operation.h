//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_OPERATION_H
#define OT_VARIATION_OPERATION_H


#include <utility>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include "node.h"
#include "chain.h"

class document;

class operation {
    // value is a parent
    std::unordered_map<node_id_t, node_id_t> deletions;
    std::unordered_map<node_id_t, int> updates;
    std::unordered_map<node_id_t, chain> insertions;

    ll content_hash = 0;

    void rehang_insertions(const node_id_t &from_node_id, const node_id_t &to_node_id);

public:
    [[nodiscard]] std::unordered_map<node_id_t, node_id_t> const *get_deletions() const { return &deletions; };
    [[nodiscard]] std::unordered_map<node_id_t, int> const *get_updates() const { return &updates; };
    [[nodiscard]] std::unordered_map<node_id_t, chain> const *get_insertions() const { return &insertions; };

    operation() = default; // empty

    /**
     * for (this, rhs) returns (rhs', this')
     */
    [[nodiscard]] std::pair<std::unique_ptr<operation>, std::unique_ptr<operation>> transform(
            const operation &s,
            const bool &only_right_part
    ) const;

    void apply(const operation &rhs);

    void insert(const node_id_t &s, const chain &chain_to_copy);

    void update(const node_id_t &node_id, const int& new_value);

    void del(const node_id_t &node_id, node_id_t parent_id);

    bool empty() const;

    // only for validation purposes. should be precalculated
    ll hash() const;
};


#endif //OT_VARIATION_OPERATION_H
