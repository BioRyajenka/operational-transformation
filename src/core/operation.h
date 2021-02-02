//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_OPERATION_H
#define OT_VARIATION_OPERATION_H


#include <utility>
#include <memory>
#include <unordered_map>
#include "node.h"
#include "chain.h"
#include "../client/document.h"

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

class operation {
public:
    std::unordered_set<node_id_t> deletions;
    std::unordered_map<node_id_t, int> updates;
    std::unordered_map<node_id_t, chain> insertions; // TODO: chain получать по ссылке, складывать через emplace_back

public:
    operation() = default; // empty

    // creates new operation with provided chain. note that ch is not copied
    operation(const node_id_t &node_id, const chain& ch);

//    operation(const int &node_id, const bool &del, node<symbol> *ins) {
//        lists[node_id] = std::make_pair(del, std::make_shared<chain>(ins));
//    }

    /**
     * for (this, rhs) returns (rhs', this')
     */
    [[nodiscard]] std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> transform(
            const operation &rhs,
            const std::shared_ptr<document> &root_state, // nullptr for server, smth for client
            const bool &only_right_part
//            const bool &copy_right_part // if result's right part need to be deep-copied
    ) const;

    /**
    это когда в начало добавляем
     node<symbol> *copy = insertion->second.deep_copy();
    const auto &existing_chain = target->insertions.find(leftmost->value.id);
    if (existing_chain != target->insertions.end()) {
        existing_chain->second.add_to_beginning(copy);
    } else {
        target->insertions[leftmost->value.id] = copy;
    }
     */
    void apply(const operation &rhs);

    // only for validation purposes. should be precalculated
    int hash() const;

    /**
     * Split current operation 'this' on two operations: 'v' and 'x'. Such that:
     *  - this = v.x
     *  - x contains only inserts which start in the nodes listed in 'deletions'
     *  - v and x not overlaps
     * Motivation is the following: v is something server can process and x is
     *  something that can't be processed
     * @return x or nullptr if x is empty. Modifies this, making it v.
     */
    std::shared_ptr<operation> detach_unprocessable_by_server(std::unordered_set<node_id_t> deletions);
};


#endif //OT_VARIATION_OPERATION_H
