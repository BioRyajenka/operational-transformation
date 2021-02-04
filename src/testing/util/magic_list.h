//
// Created by Igor on 30.01.2021.
//

#ifndef OT_VARIATION_MAGIC_LIST_H
#define OT_VARIATION_MAGIC_LIST_H

#include <unordered_map>
#include <cassert>

template<typename T>
class magic_list {
    std::unordered_map<T, int> idx;
    std::vector<T> nums = {};
//    std::uniform_real_distribution<double> dice(0., 1.);

public:
    void insert(const T& val) {
        assert(idx.find(val) == idx.end());

        nums.push_back(val);
        idx.emplace(val, nums.size() - 1);
    }

    void remove(const T& val) {
        assert(idx.find(val) != idx.end());

        std::swap(nums[idx[val]], nums.back());
        idx[nums[idx[val]]] = idx[val];
        idx.erase(val);
        nums.pop_back();
    }

    T get_random() const {
        return nums[(rand() * rand()) % nums.size()];
    }
};

#endif //OT_VARIATION_MAGIC_LIST_H
