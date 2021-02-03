//
// Created by Igor on 30.01.2021.
//

#include <unordered_set>
#include <cassert>
#include "operation.h"
#include "../client/document.h"

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
        const std::unordered_map<node_id_t, chain> &insertions,
        const operation &state_with_deletion,
        std::unordered_set<node_id_t> &processed,
        const std::shared_ptr<operation> &target
) {
    // validate that server_copy != nullptr

    // сначала ищем prev_node, двигаясь сначала влева по удаленным,
    // как только нашли не удаленную, двигаемся максимально вправо по инсертам, если есть
    // в итоге prev_node - либо инсертнутая самая правая, либо не удаленная (в т.ч. корневая)

    // затем двигаемся начиная с k по удаленным вправо

    // потом, стартуя с этой (правой) вершины, двигаемся влево по удаленным
    // на каждой итерации, если в левой части есть инсерт в вершину, удаленную в правой части,
    // добавляем операцию INSERT prev_node: цепочка инсерта

    assert(state_with_deletion.get_deletions()->count(search_initial_id));

    // first, find leftmost
    node<symbol> const *initial_node = root_state->get_node(search_initial_id);
    assert(initial_node != nullptr);

    node<symbol> const *leftmost = initial_node;
    while (state_with_deletion.get_deletions()->count(leftmost->value.id)) {
        leftmost = leftmost->prev;
        assert(leftmost && "Root node can't be deleted!");
    }

    const auto &tmp = state_with_deletion.get_insertions()->find(leftmost->value.id);
    if (tmp != state_with_deletion.get_insertions()->end()) {
        leftmost = tmp->second.get_tail();
    }

    // now, find rightmost deleted
    node<symbol> const *cur = initial_node;
    while (cur->next != nullptr && state_with_deletion.get_deletions()->count(cur->next->value.id)) {
        cur = cur->next;
    }

    while (cur != leftmost) {
        assert(cur != nullptr && state_with_deletion.get_deletions()->count(cur->value.id));
        const auto &insertion = insertions.find(cur->value.id);
        if (insertion != insertions.end()) {
            processed.insert(cur->value.id);

            target->insert(leftmost->value.id, insertion->second);
        }
    }
}

void process_deleted_insertions(
        const std::unordered_map<node_id_t, chain> &insertions,
        const operation &state_with_deletion,
        const std::shared_ptr<document> &root_state,
        const std::shared_ptr<operation> &target
) {
    std::unordered_set<node_id_t> processed;
    for (const auto&[k, ch]: insertions) {
        if (state_with_deletion.get_deletions()->count(k) && !processed.count(k)) {
            assert(root_state != nullptr);
            // arguments:
            //  - starting position of search
            //  - root state
            //  - insertions of "inserting" side
            //  - whole "deleting" operation (insertions and deletions are used)
            //  - reference to set with processed node ids
            //  - target - op to apply operations over
            apply_corrected_insert_operations(
                    k, root_state, insertions, state_with_deletion, processed, target
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
    std::unordered_set<node_id_t> temp;
    for (const auto&[k, ch]: insertions) {
        ch.iterate([&temp](const node_id_t &val) {
            assert(!temp.count(val));
            temp.insert(val);
        });
    }
    for (const auto&[k, ch]: rhs.insertions) {
        assert(!temp.count(k));
    }

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
    if (!only_right_part) process_deleted_insertions(rhs.insertions, *this, root_state, left);
    process_deleted_insertions(insertions, rhs, root_state, right);

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

    // TODO:
    //  !!! возможно тут чето не так
    //      deleted insertions' tailings всегда идут первыми, но операции внутри
    //      unique insertions не упорядочены относительно операций внутри common insertions' tailings
    //      кажется что можно придумать такие (a.b) что a в unique, а (a.b) в common tailings или типа
    //      того. тогда если будет еще какое-то c, то порядок (a.b).c и другой штуки будет разный...
    //      короче, сложна
    //      .
    //      интуитивно, речь о том, что операции из левой части могут быть в той или иной группе
    //      (common,unique,deleted) в зависимости от правой части. может это что-то сломать?
    //      .
    //      вроде нет: бывают пересечения только по (common,deleted) и (unique,deleted), а они
    //      упорядочены

    return std::make_pair(left, right);
}
