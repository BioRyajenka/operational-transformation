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
#include "hash_counter.h"

/**
 * list of changes should support following operations:
 *  insert(pos, val)
 *  update(pos, val)
 *  delete(pos)
 * All this operations should be as fast as possible
 *
 *
 * [1, 2, 3, 4]
 * 1. получить ноду по индексу
 * 2. вставить, обновить индексы у суффикса
 *
 * ? массив, где в значении будет лежать искомый индекс, а в индексе - место в массиве
 *
 *
 * AVL-tree/treap gives O(nlogn) for each iteration
 *
 * но я могу использовать другой интерфейс - insert(node, val), и тогда эти операции за O(1)
 *  (при условии, что для теста можно реализовать insert(1) delete(1) get_random(1))
 *  https://www.geeksforgeeks.org/design-a-data-structure-that-supports-insert-delete-search-and-getrandom-in-constant-time/
 *
 */

class document;

class operation {
    std::unordered_set<node_id_t> deletions;
    std::unordered_map<node_id_t, int> updates;
    std::unordered_map<node_id_t, chain> insertions;

    hash_counter hasher;

    void rehang_insertions(const node_id_t &node_id, const std::shared_ptr<document> &root_state);

public:
    [[nodiscard]] std::unordered_set<node_id_t> const *get_deletions() const { return &deletions; };
    [[nodiscard]] std::unordered_map<node_id_t, int> const *get_updates() const { return &updates; };
    [[nodiscard]] std::unordered_map<node_id_t, chain> const *get_insertions() const { return &insertions; };

    operation() = default; // empty

    /**
     * for (this, rhs) returns (rhs', this')
     */
    [[nodiscard]] std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> transform(
            const operation &s,
            const std::shared_ptr<document> &root_state, // nullptr for server, smth for client
            const bool &only_right_part
//            const bool &copy_right_part // if result's right part need to be deep-copied
    ) const;

    // root_state is optional and is needed only for cases where
    // user wants to delete node, which has insertions on it
    // in this case insertions are being rehanged
    void apply(const operation &rhs, const std::shared_ptr<document> &root_state);

    void insert(const node_id_t &s, const chain &chain_to_copy);

    void update(const node_id_t &node_id, const int& new_value);

    // fast versions, slightly faster than 'apply', as it don't require intermediate objects
    void del(const node_id_t &node_id, const std::shared_ptr<document> &root_state);

    // only for validation purposes. should be precalculated
    ll hash() const;

    /**
     * Split current operation 'this' on two operations: 'v' and 'x'. Such that:
     *  - this = v.x
     *  - x contains only inserts which start in the nodes listed in 'deletions'
     *  - v and x not overlaps
     * Motivation is the following: v is something server can process and x is
     *  something that can't be processed
     * @return x or nullptr if x is empty. Modifies this, making it v.
     */
    std::shared_ptr<operation> detach_unprocessable_by_server(const std::unordered_set<node_id_t> &dels);
};


#endif //OT_VARIATION_OPERATION_H
