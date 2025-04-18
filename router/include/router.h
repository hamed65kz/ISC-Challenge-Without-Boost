#ifndef ROUTER_H
#define ROUTER_H

#include <queue>
#include <list>
#include "sessions.h"
#include "signaling_queue.h"

/**
 * @class Router
 * @brief Manages network routing and handling of client connections.
 */
class Router
{
public:
    /**
     * @brief Starts the router with specified thread count and port.
     * @param thread_count Number of worker threads to spawn.
     * @param port Port number to listen on.
     * @return Status code indicating success or failure.
     */
    static int start(unsigned thread_count, unsigned port);

private:
    /**
     * @brief a handler for worker threads to handle received events.
     * @param thread_id identifier of thread.
     */
    static void worker_thread_read_handler(int thread_id);

    /**
     * @brief a handler for worker threads to handle write events.
     * @param thread_id identifier of thread.
     */
    static void worker_thread_write_handler(int thread_id);

    /**
     * @brief Starts the event listener or Event loop to monitor socket intruppt and handle them to worker threads.
     * @param server_socket Listening socket descriptor.
     */
    static void start_event_listener(int server_socket);

    /**
     * @brief Performs read operations on sockets.
     * @param ready_read_socket Socket descriptor ready for reading.
     * @param recv_buffer Buffer to store received data. each thread has specific recv_buffer for itself and pass it to do_reads() 
     */
    static void do_reads(int ready_read_socket,char* recv_buffer);

    /**
    * Processes an incoming message from the specified socket.
    * Reads a 32-byte message, extracts the destination ID, and forwards it if valid.
    * @param ready_read_socket Socket descriptor ready for reading.
    * @param recv_buffer Buffer to store received data.
    * @return Number of bytes read from the socket. -1 on Socket Error. 0 on dst not found */
    static int process_message(int ready_read_socket, char* recv_buffer);

    /**
    * Handles the handshake process by reading the initial 3-byte ID message.
    * Extracts the source ID and registers the session.
    * @param ready_read_socket Socket descriptor ready for reading.
    * @param recv_buffer Buffer to store received data.
    * @return Number of bytes read from the socket. -1 on Socket Error.
    */
    static int handle_handshake(int ready_read_socket,
                          char* recv_buffer);

    /**
     * @brief insert write task in write queue, worker threads will pop write task and do them
     * @param task Pair containing socket descriptor and message string.
     */
    static void do_writes(std::pair<int, std::string> task);

    /**
     * @brief Forwards a message to the destination session. it push message on write queue, worker threads will pop write task and do them
     * @param dst_session pointer to the destination session.
     * @param msg Message to be forwarded.
     */
    static void forward(std::shared_ptr<Session> dst_session, std::string msg);




    // Static member variables managing concurrency and socket states.

    /** Queue of sockets ready for reading, they signals by select
     * use std::list as internal queuing mechanism for building queue with linked-list. it helps efficient add/remove to/from FIFO heads.
    */
    static SignalingQueue<int> ready_read_sockets_queue_;
    
    /** Queue of sockets ready for writing along with messages, worker threads push to it after process received msg
    * use std::list as internal queuing mechanism for building queue with linked-list. it helps efficient add/remove to/from FIFO heads.
    */
    static SignalingQueue<std::pair<int, std::string>> ready_write_sockets_queue_;

};

#endif