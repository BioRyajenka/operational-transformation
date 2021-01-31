//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_MAGIC_LIST_H
#define OT_VARIATION_MAGIC_LIST_H

template<typename T>
class magic_list {
public:
    void insert(const T&);
    void remove(const T&);
    T get_random() const;
};

#endif //OT_VARIATION_MAGIC_LIST_H
