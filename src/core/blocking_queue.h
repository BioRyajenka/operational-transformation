//
// Created by Igor on 03.02.2021.
//

#ifndef OT_VARIATION_BLOCKING_QUEUE_H
#define OT_VARIATION_BLOCKING_QUEUE_H


#include <mutex>
#include <condition_variable>
#include <deque>

template<typename T>
class blocking_queue {
private:
    std::mutex mut;
    std::condition_variable condition;
    std::deque<T> queue;
public:
    void push(T const &value) {
        {
            std::unique_lock<std::mutex> lock(this->mut);
            queue.push_front(value);
        }
        this->condition.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(this->mut);
        this->condition.wait(lock, [=] { return !this->queue.empty(); });
        T rc(std::move(this->queue.back()));
        this->queue.pop_back();
        return rc;
    }

    int size() const {
        return queue.size();
    }
};

#endif //OT_VARIATION_BLOCKING_QUEUE_H
