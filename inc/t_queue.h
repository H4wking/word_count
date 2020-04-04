//
// Created by sluzh on 4/3/2020.
//

#ifndef WORD_COUNT_T_QUEUE_H
#define WORD_COUNT_T_QUEUE_H


#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include <condition_variable>

template <class T>
class t_queue {
private:
    std::deque<T> queue;
    mutable std::mutex mut;
    std::condition_variable cond_variable;

public:
    t_queue() = default;

    ~t_queue() = default;

    t_queue(const t_queue &q) = delete;

    const t_queue &operator=(const t_queue &q) = delete;

    void push_back(T d) {
        {
            std::lock_guard<std::mutex> lg(mut);
            queue.push_back(d);
        }
        cond_variable.notify_one();
    }


    T pop() {
        std::unique_lock<std::mutex> lg(mut);
        if (queue.empty()) {
            cond_variable.wait(lg);
        }
        cond_variable.wait(lg, [this]() { return !queue.empty(); });
        std::basic_string<char> d = queue.front();
        queue.pop_front();
        return d;
    }

    size_t get_size() const {
        std::lock_guard<std::mutex> lg(mut);
        return queue.size();
    }
};


#endif //WORD_COUNT_T_QUEUE_H
