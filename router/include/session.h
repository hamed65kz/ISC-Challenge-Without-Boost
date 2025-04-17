#ifndef SESSION_H
#define SESSION_H
#include <shared_mutex>

#define NONE -1 

/**
 * @brief Represents a network session.
 */
class Session
{
public:
    /**
     * @brief Constructor to initialize a session with a socket descriptor.
     * @param socket The socket descriptor associated with the session.
     */
    Session(int socket) {
        this->socket_ = socket;
        this->mutex_ = std::make_shared<std::shared_mutex>();
        this->id_ = NONE;
    }

    /**
     * @brief Destructor for the session.
     */
    ~Session() {
        
    }

    /**
     * @brief Gets the session's unique identifier.
     * @return The session ID.
     */
    int get_id() const {
        return this->id_;
    }

    /**
     * @brief Gets the socket descriptor associated with the session.
     * @return The socket descriptor.
     */
    int get_socket() const {
        return this->socket_;
    }

    /**
     * @brief Sets the session's unique identifier.
     * @param id The new session ID.
     */
    void set_id(int id) {
        this->id_ = id;
    }

    /**
     * @brief Gets the shared mutex used for thread synchronization for avoid read/write on single socket from multiple thread.
     * @return A shared pointer to the mutex.
     */
    std::shared_ptr<std::shared_mutex> get_mutex() const {
        return this->mutex_;
    }

private:
    int socket_; ///< Socket descriptor associated with the session
    int id_; ///< Unique identifier for the session
    std::shared_ptr<std::shared_mutex> mutex_; ///< Mutex for thread-safe read/write on single socket
};


#endif