#ifndef SIGNALINGQUEUE_H
#define SIGNALINGQUEUE_H

/**
 * @brief A thread-safe queue implementation with signaling capabilities.
 */
template <typename T>
class SignalingQueue {
private:
    /// Underlying queue storing elements
    std::queue<T> m_queue;

    /// Mutex for synchronizing access to the queue.
    std::mutex m_mutex;

    /// Condition variable for signaling availability of new items.
    std::condition_variable m_cond;

public:
    /**
     * @brief Pushes an element into the queue and notifies one waiting thread
     * @param item The element to be added to the queue.
     * @param skip_if_repeated If true, skips insertion if the last element is equal to item.
     */
    void push(T item, bool skip_if_repeated=false)
    {

        std::unique_lock<std::mutex> lock(m_mutex);

        if (skip_if_repeated && m_queue.size() > 0) {
            if (m_queue.back() == item) {
                return; // Skip inserting duplicate at the end
            }
        }


        m_queue.push(item);

        // Notify one waiting thread that an item is available
        m_cond.notify_one();
    }

    /**
     * @brief Pops an element from the queue.
     * This operation blocks if the queue is empty until an item is available.
     * @return The element at the front of the queue.
     */
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Wait until the queue is not empty
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });

        T item = m_queue.front();
        m_queue.pop();
        return item;
    }
};
#endif