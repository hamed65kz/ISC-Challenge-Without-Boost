#ifndef SESSIONS_H
#define SESSIONS_H

#include "session.h"
#include <unordered_map>

#define MAX_CLIENTS_COUNT 999

/**
 * @class Sessions
 * @brief Manages connected client sessions.
 *
 * This class provides static methods to initialize, accept, retrieve,
 * add, find, and remove client sessions. It maintains internal mappings
 * between sockets and session IDs for efficient session management.
 */
class Sessions
{
public:
    /**
     * @brief Initialize the session management with a maximum number of clients.
     * @param max_clients_count Maximum number of clients to support.
     */
    static void init_sessions(int max_clients_count);

    /**
     * @brief Accepts a new client connection.
     * @param client_socket Socket descriptor of the client to accept.
     */
    static void accept_client(int client_socket);

    /**
     * @brief Retrieve the list of accepted client sockets.
     * @return Const reference to vector of accepted client sockets.
     */
    static const std::vector<int>& get_accpeted_sockets();

    /**
     * @brief Add a new node with its associated socket and node ID.
     * @param socket Socket descriptor for the session.
     * @param node_id Unique identifier for the session node.
     */
    static void add_node(int socket, int node_id);

    /**
     * @brief Find a session by its node ID.
     * @param node_id The ID of the session node to find.
     * @return Shared pointer to the found Session, or nullptr if not found.
     */
    static std::shared_ptr<Session> find_session_by_id(int node_id);

    /**
     * @brief Find a session by its client socket.
     * @param client_socket The socket descriptor of the client.
     * @return Shared pointer to the found Session, or nullptr if not found.
     */
    static std::shared_ptr<Session> find_session_by_socket(int client_socket);

    /**
     * @brief Remove a session associated with the given socket.
     * @param socket Socket descriptor of the session to remove.
     */
    static void removeSession(int socket);

private:
    /// Map of socket descriptors to Session objects.
    /// we need access to session by its socket, we use unordered_map for access by O(1)
    static std::unordered_map<int, std::shared_ptr<Session>> sessions_by_socket_;

    /// Map of node IDs to Session objects.
    /// we need access to session by its id, we use unordered_map for access by O(1)
    static std::unordered_map<int, std::shared_ptr<Session>> sessions_by_id_;

    /// List of accepted client sockets.
    /// we need a continues memory for keeps accepted sockets, we will iterate it for fill fd_set.
    static std::vector<int> accepted_clients_;

    /// Mutex for thread-safe access to sessions add/read/remove.
    static std::shared_ptr<std::shared_mutex> sessions_mutex_;
};

#endif