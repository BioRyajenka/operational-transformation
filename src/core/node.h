//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_NODE_H
#define OT_VARIATION_NODE_H

template<typename T>
class node {
public:
    node(node<T> *prev, node<T> *next, T value) : prev(prev), next(next), value(value) {}

    node<T> *prev;
    node<T> *next;
    T value;
};

#endif //OT_VARIATION_NODE_H
