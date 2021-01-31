//
// Created by Igor on 30.01.2021.
//

#include <unordered_set>
#include "operation.h"

auto transform_chain(
        const std::pair<bool, std::shared_ptr<chain>> *first,
        const std::pair<bool, std::shared_ptr<chain>> *second
) {
    // contract:
    //  if second.delete => first.delete
    //  if any short => first is short

    // кажется что ничего нигде не нужно копировать

    if (!first->second->get_root() && !second->second->get_root()) {
        // both short
        return std::make_pair()
    }
}

std::pair<std::shared_ptr<operation>, std::shared_ptr<operation>> operation::transform(
        const operation &rhs
//        const bool &copy_right_part
) const {
    // при трансформировании цепи не пересекаются. но могут из одной начинаться

    auto left = std::make_shared<operation>();
    auto right = std::make_shared<operation>();

    std::unordered_set<int> common_keys;
    for (const auto&[key, value] : rhs.lists) {
        if (lists.count(key)) {
            common_keys.insert(key);
        } else {
            // unique in right
            left->lists[key] = value; // shallow copy
        }
    }

    for (const auto&[key, value] : lists) {
        if (!common_keys.count(key)) {
            // unique in left
//            right->lists[key] = copy_right_part
//                                ? std::make_pair(value.first, std::make_shared<chain>(*value.second))
//                                : value;
            right->lists[key] = value; // also shallow copy. seems ok
        }
    }

    for (const int &key : common_keys) {
        const auto *a = &lists.at(key);
        const auto *b = &rhs.lists.at(key);

        bool swapped = false;
        // first, reorder
        if (!a->second->get_root() && !b->second->get_root()) {
            // if both are short, delete comes first
            if (b->first && !a->first) {
                swap(a, b);
                swapped = true;
            }
        } else if (!a->second->get_root()) {
            // first is short, second is not. do nothing
        } else if (!b->second->get_root()) {
            // second is short, first is not
            swap(a, b);
            swapped = true;
        } else {
            // both are not short
            if (b->second->get_root()->value.id < a->second->get_root()->value.id) {
                swap(a, b);
                swapped = true;
            }
            if (b->second->get_root()->value.id == a->second->get_root()->value.id) {
                throw std::runtime_error("Id's can't be equal!");
            }
        }

        auto res = transform_chain(a, b);

        if (swapped) {
            left->lists[key] = res.second;
            right->lists[key] = res.first;
        } else {
            left->lists[key] = res.first;
            right->lists[key] = res.second;
        }
    }

    return std::make_pair(left, right);
}
