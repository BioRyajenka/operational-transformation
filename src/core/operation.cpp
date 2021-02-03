//
// Created by Igor on 30.01.2021.
//

#include "operation.h"
#include "../client/document.h"

void operation::apply(const operation &rhs, const std::shared_ptr<document> &root_state) {
    for (const auto &node_id : rhs.deletions) del(node_id, root_state);
    for (const auto &[node_id, new_value] : rhs.updates) update(node_id, new_value);
    for (const auto &[node_id, ch] : rhs.insertions) insert(node_id, ch);
}

void operation::insert(const node_id_t &node_id, const chain &chain_to_copy) {
    assert(!deletions.count(node_id) && "Deleted nodes can't be inserted to");

    const auto& ch = insertions.find(node_id);
    if (ch != insertions.end()) {
        ch->second.copy_to_the_beginning(chain_to_copy);
    } else {
        insertions.emplace(node_id, chain_to_copy);
    }
}

void operation::update(const node_id_t &node_id, const int &new_value) {
    assert(!deletions.count(node_id) && "Deleted nodes can't be updated");
    updates.emplace(node_id, new_value).first->second = new_value;
}

void operation::del(const node_id_t &node_id, const std::shared_ptr<document> &root_state) {
    // TODO: update hash here

    if (updates.count(node_id)) {
        updates.erase(node_id);
    }

    if (root_state && !root_state->get_node(node_id)) {
        // should be in one of the chains
        for (auto &[k, ch] : insertions) {
            const auto &node = ch.find_node(node_id);
            if (node) {
                if (!node->prev && !node->next) {
                    // the only node in the chain
                    insertions.erase(k);
                    return;
                }
                ch.remove_node(node);
                return;
            }
        }
        assert(false && "Node_id is not found neither in doc nor in chains");
    } else {
        deletions.insert(node_id);
    }

    if (insertions.count(node_id)) {
        rehang_insertions(node_id, root_state);
    }
}

void operation::rehang_insertions(const node_id_t &node_id, const std::shared_ptr<document> &root_state) {
    assert(root_state);
    assert(deletions.count(node_id));
    assert(insertions.count(node_id));

    auto cur = root_state->get_node(node_id);
    assert(cur);
    while (deletions.count(cur->value.id)) {
        cur = cur->prev;
        assert(cur && "Root node can't be deleted!");
    }
    const auto &new_starting_node_id = cur->value.id;

    chain &source_chain = insertions.at(node_id);
    const auto &target_chain = insertions.find(new_starting_node_id);
    if (target_chain != insertions.end()) {
        // chain already exists
        target_chain->second.move_to_the_end(std::move(source_chain));
    } else {
        // creating new chain
        insertions.emplace(new_starting_node_id, std::move(source_chain));
    }
    insertions.erase(node_id);
}

ll operation::hash() const {
    return hasher.hash();
}

std::shared_ptr<operation> operation::detach_unprocessable_by_server(const std::unordered_set<node_id_t> &dels) {
    std::shared_ptr<operation> x = nullptr;
    for (auto it = insertions.begin(); it != insertions.end(); ) {
        if (deletions.count(it->first)) {
            if (x == nullptr) x = std::make_shared<operation>();
            x->insertions.emplace(it->first, std::move(it->second));
            it = insertions.erase(it);
        } else {
            ++it;
        }
    }
    return x;
}
