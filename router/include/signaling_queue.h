#pragma once


#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <list>

// Thread-safe queue
template <typename T>
class SignalingQueue {
private:
    // Underlying queue
    std::queue<T, std::list<T>> m_queue;

    // mutex for thread synchronization
    std::mutex m_mutex;

    // Condition variable for signaling
    std::condition_variable m_cond;

public:

    // Pushes an element to the queue
    void push(T item, bool skip_if_repeated=false)
    {

        // Acquire lock
        std::unique_lock<std::mutex> lock(m_mutex);

        if (skip_if_repeated && m_queue.size() > 0) {
            if (m_queue.back() == item) {
              return;
            }
        }

        // Add item
        m_queue.push(item);

        // Notify one thread that
        // is waiting
        m_cond.notify_one();
    }

    // Pops an element off the queue
    T pop()
    {

        // acquire lock
        std::unique_lock<std::mutex> lock(m_mutex);

        // wait until queue is not empty
        m_cond.wait(lock,
            [this]() { return !m_queue.empty(); });

        // retrieve item
        T item = m_queue.front();
        m_queue.pop();

        // return item
        return item;
    }
};

