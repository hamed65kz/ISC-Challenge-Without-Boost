#ifndef TCPSERVER_H
#define TCPSERVER_H
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#endif
#include <iostream>

#ifdef _WIN32
#define GET_SOCKET_ERROR() WSAGetLastError()
#else
#define GET_SOCKET_ERROR() errno
#endif


/**
 * @class TcpServer
 * @brief Provides static methods to create, accept, and communicate over a TCP server.
 */
class TcpServer
{
public:
    /**
     * @brief Starts a TCP server listening on the specified port.
     * @param port The port number to bind the server socket.
     * @return The server socket descriptor, or -1 on failure.
     */
    static int start_tcp_server(int port);

    /**
     * @brief Accepts an incoming client connection.
     * @param server_socket The server socket descriptor.
     * @return The client socket descriptor, or error code on failure.
     */
    static int accept_client(int server_socket);

    /**
     * @brief Sets a client socket to non-blocking mode.
     * @param client_socket The client socket descriptor.
     * @return 0 on success, -1 on failure.
     */
    static int set_client_socket_nonblocking(int client_socket); 

    /**
     * @brief Reads data asynchronously from a client socket.
     * @param client_socket The client socket descriptor.
     * @param buffer Pointer to the buffer to store read data.
     * @param buffer_len The maximum length of data to read.
     * @return Number of bytes read, or SOCKET_ERROR on failure.
     */
    static int read_async(int client_socket, char* buffer, int buffer_len);

    /**
     * @brief Sends data to a client socket.
     * @param client_socket The client socket descriptor.
     * @param buffer The data to send.
     * @return Number of bytes sent, or SOCKET_ERROR on failure.
     */
    static int send_to_client(int client_socket, std::string buffer);

private:
    /**
     * @brief Checks if there is no more data to read.
     * @return true if no more data is available, false otherwise.
     */
    static bool no_more_data();
};

#endif // TCPSERVER_H
