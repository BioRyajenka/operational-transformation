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
    // symbol_id -> (delete, chain)
    // delete is true if symbol_id should be deleted
    // chain is never null, but may be empty
    std::unordered_map<int, std::pair<bool, std::shared_ptr<chain>>> lists;

public:
    operation() = default; // empty

    operation(const int &node_id, const bool &del, node<symbol> *ins) {
        lists[node_id] = std::make_pair(del, std::make_shared<chain>(ins));
    }

    // only for validation purposes. should be precalculated
    int hash() const;

    // todo: pointer to operation maybe?
    /**
     * for (this, rhs) returns (rhs', this')
     */
    [[nodiscard]] std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> transform(
            const operation &rhs
//            const bool &copy_right_part // if result's right part need to be deep-copied
    ) const;

    void apply(const operation &rhs);
};


#endif //OT_VARIATION_OPERATION_H
