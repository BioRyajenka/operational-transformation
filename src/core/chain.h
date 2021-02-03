//
// Created by Igor on 31.01.2021.
//

#ifndef OT_VARIATION_CHAIN_H
#define OT_VARIATION_CHAIN_H

#include <cassert>
#include <functional>
#include "symbol.h"
#include "node.h"

class chain {
    node<symbol> *head, *tail;

public:
    chain(symbol s) {
        head = tail = new node<symbol>(nullptr, nullptr, s);
    }

    chain(const chain &rhs) : head(nullptr), tail(nullptr) { // copy constructor
        copy_to_the_beginning(rhs);
    }

    chain(chain &&rhs) : head(nullptr), tail(nullptr) { // move constructor
        move_to_the_beginning(std::move(rhs));
    }

    ~chain() {
//        iterate_nodes([](const auto& n){ delete n; });
        auto cur = head;
        while (cur != nullptr) {
            const auto tmp = cur;
            cur = cur->next;
            delete tmp;
        }
    }

    node<symbol> *get_head() const {
        assert(head && "Getting head from empty chain is prohibited!");
        return head;
    }

    node<symbol> *get_tail() const {
        assert(tail && "Getting tail from empty chain is prohibited!");
        return tail;
    }

    // doesn't precalc result, to save memory
    node<symbol> *find_node(node_id_t node_id) const {
        auto cur = head;
        while (cur != nullptr) {
            if (cur->value.id == node_id) return cur;
            cur = cur->next;
        }
        return nullptr;
    }

    // assuming node is from this chain (not validating it)
    void remove_node(node<symbol> const *n) {
        assert(head && "The chain is empty. Seems like n is not from the chain");
        assert((n != head || n != tail) && "Removing the only node is prohibited!");
        if (n == head) {
            head = head->next;
            head->prev = nullptr;
        } else if (n == tail) {
            tail = n->prev;
            tail->next = nullptr;
        } else {
            assert(n->prev && n->next);
            n->prev->next = n->next;
            n->next->prev = n->prev;
        }
        delete n;
    }

    void iterate(const std::function<void(const symbol &)> &consumer) const {
        auto cur = head;
        while (cur != nullptr) {
            consumer(cur->value);
            cur = cur->next;
        }
    }

//    void iterate_nodes(const std::function<void(node<symbol> *)> &consumer) const {
//        auto cur = head;
//        while (cur != nullptr) {
//            consumer(cur);
//            cur = cur->next;
//        }
//    }

    std::pair<node<symbol>*, node<symbol>*> copy_to(node<symbol> *target, const chain &rhs) {
        return move_to(target, rhs.copy());
    }

    void copy_to_the_beginning(const chain &rhs) {
        move_to_the_beginning(rhs.copy());
    }

    void move_to_the_end(chain &&rhs) {
        move_to(tail, std::move(rhs));
    }

    std::pair<node<symbol>*, node<symbol>*> move_to(node<symbol> *target, chain &&rhs) {
        if (target == tail) {
            tail = rhs.tail;
        } else {
            rhs.tail->next = target->next;
            target->next->prev = rhs.tail;
        }

        target->next = rhs.head;
        rhs.head->prev = target;

        const auto h = rhs.head;
        const auto t = rhs.tail;
        rhs.head = rhs.tail = nullptr;

        return std::make_pair(h, t);
    }

    void move_to_the_beginning(chain &&rhs) {
        if (head == nullptr) {
            tail = rhs.tail;
        } else {
            rhs.tail->next = head;
            head->prev = rhs.tail;
        }

        head = rhs.head;

        rhs.head = rhs.tail = nullptr;
    }

private:
    chain copy() const {
        assert(head != nullptr);

        chain res = chain(head->value);

        auto cur = head->next;
        while (cur != nullptr) {
            node<symbol> *copy = new node<symbol>(res.tail, nullptr, cur->value);
            res.tail->next = copy;
            res.tail = copy;

            cur = cur->next;
        }

        return res;
    }
};

#endif //OT_VARIATION_CHAIN_H
