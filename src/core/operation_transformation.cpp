//
// Created by Igor on 30.01.2021.
//

#include <unordered_set>
#include <cassert>
#include "operation.h"
#include "../client/document.h"
#include "../testing/util/test_util.h"

// TODO: проверить что во всех циклах не важно в каком порядке иду

void merge_chains(
        const node_id_t &source_node_id,
        const chain &a, // both are not empty
        const chain &b,
        const std::shared_ptr<operation> &left,
        const std::shared_ptr<operation> &right
) {
    if (a.get_head()->value.id > b.get_head()->value.id) {
        // a should come first
        merge_chains(source_node_id, b, a, right, left);
        return;
    }

    if (left != nullptr) {
        assert(!left->get_insertions()->count(source_node_id));
        left->insert(source_node_id, b);
    }

    if (right != nullptr) {
        right->insert(b.get_tail()->value.id, a);
    }
}

void process_deleted_insertions(
        const std::unordered_map<node_id_t, chain> &insertions,
        const operation &rhs,
        const std::shared_ptr<operation> &target
) {
    for (const auto&[node_id, ch]: insertions) {
        const auto &rhs_del = rhs.get_deletions()->find(node_id);
        if (rhs_del != rhs.get_deletions()->end()) {
            assert(!rhs.get_insertions()->count(node_id));

            const node_id_t &del_parent = rhs_del->second;
            const auto &rhs_ins_from_parent = rhs.get_insertions()->find(del_parent);

            node_id_t new_root;
            if (rhs_ins_from_parent != rhs.get_insertions()->end()) {
                new_root = rhs_ins_from_parent->second.get_tail()->value.id;
            } else {
                new_root = del_parent;
            }

            target->insert(new_root, ch);
        }
    }
}

void process_unique_insertions(
        const std::unordered_map<node_id_t, chain> &insertions,
        const operation &rhs,
        const std::shared_ptr<operation> &target
) {
    for (const auto&[node_id, ch]: insertions) {
        if (!rhs.get_deletions()->count(node_id) && !rhs.get_insertions()->count(node_id)) {
            target->insert(node_id, ch);
        }
    }
}

void process_common_insertions(
        const std::unordered_map<node_id_t, chain> &insertions,
        const operation &rhs,
        const std::shared_ptr<operation> &left,
        const std::shared_ptr<operation> &right
) {
    for (const auto&[node_id, ch]: insertions) {
        const auto &rhs_insertion = rhs.get_insertions()->find(node_id);
        if (rhs_insertion != rhs.get_insertions()->end()) {
            assert(!rhs.get_deletions()->count(node_id)); // TODO: too obvious assert. can be deleted

            merge_chains(node_id, ch, rhs_insertion->second, left, right);
        }
    }
}

void process_deletions(
        const std::unordered_map<node_id_t, node_id_t> &deletions,
        const operation &rhs,
        const std::shared_ptr<operation> &target
) {
    for (const auto &[node_id, parent_id] : deletions) {
        const auto &rhs_del_node = rhs.get_deletions()->find(node_id);
        if (rhs_del_node != rhs.get_deletions()->end()) {
            assert(rhs_del_node->second == parent_id);
        } else {
            const auto &rhs_ins = rhs.get_insertions()->find(parent_id);

            if (rhs_ins != rhs.get_insertions()->end()) {
                const node_id_t &new_parent = rhs_ins->second.get_tail()->value.id;
                target->del(node_id, new_parent);
            } else {
                const auto &rhs_del_parent = rhs.get_deletions()->find(parent_id);
                if (rhs_del_parent != rhs.get_deletions()->end()) {
                    target->del(node_id, rhs_del_parent->second);
                } else {
                    target->del(node_id, parent_id);
                }
            }
        }
    }
}

void process_updates(
        const std::unordered_map<node_id_t, int> &updates,
        const operation &rhs,
        const std::shared_ptr<operation> &target
) {
    for (const auto &[node_id, new_value] : updates) {
        if (!rhs.get_deletions()->count(node_id)) {
            const auto &rhs_update = rhs.get_updates()->find(node_id);
            if (rhs_update == rhs.get_updates()->end() || new_value < rhs_update->second) {
                // if there is no update in rhs or our new_value is less than in rhs
                target->update(node_id, new_value);
            }
        }
    }
}

void validate_insertion_starting_nodes_unique(const operation &a, const operation &b) {
    std::unordered_set<node_id_t> temp;
    for (const auto&[_, ch]: *a.get_insertions()) {
        ch.iterate([&temp](const auto &s) {
            assert(!temp.count(s.id));
            temp.insert(s.id);
        });
    }
    for (const auto&[node_id, ch]: *b.get_insertions()) {
        assert(!temp.count(node_id));
    }
}

/**
 * current invariant is that arguments can't be changed after invocation without affecting result
 * and result can be changed without affecting arguments
 *
 * maybe this invariant can be weaken to reduce deep-copying and improve performance
 */

// TODO: единственные места, где я могу поменять операции - это insertions
//  (сейчас в этом файле это все места с .insert(...))
//  сейчас конструктор новой операции не копирует цепь, но сама apply копирует
//  вдруг я могу не копирововать даже в apply? могу, если та цепь, что в
//  конструкторе, никогда не меняется и не используется (не читается)
std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> operation::transform(
        const operation &rhs,
        const bool &only_right_part
) const {
    const auto left = only_right_part ? nullptr : std::make_shared<operation>();
    const auto right = std::make_shared<operation>();

    // === validating ops (only for debug) ===

    // validate all new nodes are unique
    validate_insertion_starting_nodes_unique(*this, rhs);

    // validate deleted nodes can't be inserted or updated
    for (const auto &[k, _]: deletions) {
        assert(!insertions.count(k) && !updates.count(k));
    }
    for (const auto &[k, _]: rhs.deletions) {
        assert(!rhs.insertions.count(k) && !rhs.updates.count(k));
    }

    // === process insertions ===

    // deleted insertions first. it is important
    if (!only_right_part) process_deleted_insertions(rhs.insertions, *this, left);
    process_deleted_insertions(this->insertions, rhs, right);

    // unique insertions
    if (!only_right_part) process_unique_insertions(rhs.insertions, *this, left);
    process_unique_insertions(insertions, rhs, right);

    // common insertions
    // if only_right_part , then left = nullptr
    process_common_insertions(insertions, rhs, left, right);

    // === process updates ===
    if (!only_right_part) process_updates(rhs.updates, *this, left);
    process_updates(updates, rhs, right);

    // === process deletions ===
    if (!only_right_part) process_deletions(rhs.deletions, *this, left);
    process_deletions(deletions, rhs, right);

//    print_operation("a", *this);
//    print_operation("b", rhs);
//    if (!only_right_part)print_operation("left", *left);
//    print_operation("right", *right);

    return std::make_pair(left, right);
}
