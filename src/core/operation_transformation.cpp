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

void apply_corrected_insert_operations(
        const node_id_t &search_initial_id,
        const std::shared_ptr<document> &root_state,
        const operation &op_with_insertion,
        const operation &op_with_deletion,
        std::unordered_set<node_id_t> &processed,
        const std::shared_ptr<operation> &target
) {
    // validate that server_copy != nullptr

    // сначала ищем новый корень, двигаясь сначала влево по удаленным. вершина не подходит,
    // если она удалена любой из двух операций и из нее нет инсертов в op_with_deletion.
    // если инсерты есть, новый корень - tail этих инсертов. если нет инсертов, но не удалена,
    // то сама вершина.

    // затем двигаемся начиная с этой вершины по всем вершинам, у которых, если бы в них был
    // инсерт в op_with_insertion, новый корень был бы new_root

    // потом, стартуя с этой (правой) вершины, двигаемся влево по удаленным
    // на каждой итерации, если в op_with_insertions есть инсерт в вершину,
    // которую мы сейчас рассматриваем (удаленную),
    // добавляем операцию INSERT new root: цепочка инсерта

    assert(op_with_deletion.get_deletions()->count(search_initial_id));

    // first, find new root
    node<symbol> const *initial_node = root_state->get_node(search_initial_id);
    assert(initial_node != nullptr);

    node<symbol> const *leftmost = initial_node;
//    printf("starting from %d\n", new_root->value.id);
    while (!op_with_deletion.get_insertions()->count(leftmost->value.id) &&
           (op_with_deletion.get_deletions()->count(leftmost->value.id)
            || op_with_insertion.get_deletions()->count(leftmost->value.id))) {
        leftmost = leftmost->prev;
//        printf("going to %d\n", new_root->value.id);
        assert(leftmost && "Document root can't be deleted!");
    }

    node<symbol> const *new_root;
    const auto &tmp = op_with_deletion.get_insertions()->find(leftmost->value.id);
    if (tmp != op_with_deletion.get_insertions()->end()) {
        new_root = tmp->second.get_tail();
    } else {
        new_root = leftmost;
    }
//    printf("now going to %d\n", new_root->value.id);

    // now, find rightmost node which can have new_root as new root in case
    // of op_with_insertions insertions in that node
    node<symbol> const *cur = initial_node;
    while (cur->next != nullptr &&
           (op_with_deletion.get_deletions()->count(cur->next->value.id)
            || op_with_insertion.get_deletions()->count(cur->next->value.id))
           && !op_with_deletion.get_insertions()->count(cur->value.id)) {
        cur = cur->next;
    }

    while (cur != leftmost) {
        assert(cur != nullptr);

        if (!(op_with_deletion.get_deletions()->count(cur->value.id)
              || op_with_insertion.get_deletions()->count(cur->value.id))) {
//            print_doc("doc", *root_state);
//            printf("new_root: %d (initial node %d)\n", new_root->value.id, search_initial_id);
//            printf("cur: %d\n", cur->value.id);
//            print_operation("op_with_deletion", op_with_deletion);
//            print_operation("op_with_insertion", op_with_insertion);
        }

        assert(op_with_deletion.get_deletions()->count(cur->value.id)
               || op_with_insertion.get_deletions()->count(cur->value.id));
        assert(!op_with_deletion.get_insertions()->count(cur->value.id));
        const auto &insertion = op_with_insertion.get_insertions()->find(cur->value.id);
        if (insertion != op_with_insertion.get_insertions()->end()) {
            processed.insert(cur->value.id);

            target->insert(new_root->value.id, insertion->second);
        }
        cur = cur->prev;
    }
}

void process_deleted_insertions(
        const operation &op_with_insertion,
        const operation &op_with_deletion,
        const std::shared_ptr<document> &root_state,
        const std::shared_ptr<operation> &target
) {
    std::unordered_set<node_id_t> processed;
    for (const auto&[node_id, ch]: *op_with_insertion.get_insertions()) {
        if (op_with_deletion.get_deletions()->count(node_id) && !processed.count(node_id)) {
            assert(root_state != nullptr);
            assert(!op_with_deletion.get_insertions()->count(node_id));
            // arguments:
            //  - starting position of search
            //  - root state
            //  - insertions of "inserting" side
            //  - whole "deleting" operation (insertions and deletions are used)
            //  - reference to set with processed node ids
            //  - target - op to apply operations over
            apply_corrected_insert_operations(
                    node_id, root_state, op_with_insertion, op_with_deletion, processed, target
            );
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
        const std::unordered_set<node_id_t> &deletions,
        const std::unordered_set<node_id_t> &rhs_deletions,
        const std::shared_ptr<operation> &target
) {
    for (const auto &node_id : deletions) {
        if (!rhs_deletions.count(node_id)) {
            // root_state is not needed here
            target->del(node_id, nullptr);
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
    for (const auto&[k, ch]: *a.get_insertions()) {
        ch.iterate([&temp](const auto &s) {
            assert(!temp.count(s.id));
            temp.insert(s.id);
        });
    }
    for (const auto&[k, ch]: *b.get_insertions()) {
        assert(!temp.count(k));
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
        const std::shared_ptr<document> &root_state, // nullptr for server, smth for client
        const bool &only_right_part
) const {
    const auto left = only_right_part ? nullptr : std::make_shared<operation>();
    const auto right = std::make_shared<operation>();

    // === validating ops (only for debug) ===

    // validate all new nodes are unique
    validate_insertion_starting_nodes_unique(*this, rhs);

    // validate deleted nodes can't be inserted or updated
    for (const auto &k: deletions) {
        assert(!insertions.count(k) && !updates.count(k));
    }
    for (const auto &k: rhs.deletions) {
        assert(!rhs.insertions.count(k) && !rhs.updates.count(k));
    }

    // === process deletions ===
    if (!only_right_part) process_deletions(rhs.deletions, deletions, left);
    process_deletions(deletions, rhs.deletions, right);

    // === process updates ===
    if (!only_right_part) process_updates(rhs.updates, *this, left);
    process_updates(updates, rhs, right);

    // === process insertions ===

    // deleted insertions first. it is important
    if (!only_right_part) process_deleted_insertions(rhs, *this, root_state, left);
    process_deleted_insertions(*this, rhs, root_state, right);

    // unique insertions
    if (!only_right_part) process_unique_insertions(rhs.insertions, *this, left);
    process_unique_insertions(insertions, rhs, right);

    // common insertions
    // if only_right_part , then left = nullptr
    process_common_insertions(insertions, rhs, left, right);

    // unique insertions and common insertions' tailings can not overlap,
    // because common insertions' tailings will always start in the new node
    //
    // but common insertions' tailings and deleted insertions' tailings
    // can overlap. in this case, deleted insertion tailing comes first
    //
    // unique insertions and deleted insertions' tailings also can overlap

    // === additional validation ===
    if (!only_right_part) validate_insertion_starting_nodes_unique(*left, *right);

    return std::make_pair(left, right);
}
