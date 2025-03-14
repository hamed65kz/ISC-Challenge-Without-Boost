#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

#define NO_ERR 0

/**
 * @class TCPSocket
 * @brief A class for managing TCP socket connections.
 * 
 * This class provides functionalities to connect to a TCP server,
 * send and receive messages, and manage the socket lifecycle.
 */
class TCPSocket {
  public:
   /**
    * @brief Constructs a TCPSocket object.
    * @param server_ip The IP address of the server.
    * @param server_port The port number of the server.
    */
   TCPSocket(std::string server_ip, int server_port);
   
   /**
    * @brief Destroys the TCPSocket object and closes the socket if open.
    */
   ~TCPSocket();
 
   /**
    * @brief Connects to the server.
    * @return 0 on success, or an error code on failure.
    */
   int connect_to_server();
 
   /**
    * @brief Receives a message from the server.
    * @param bytes_received Reference to an integer to store the number of bytes received.
    * @return 0 on success, or an error code on failure.
    */
   int recvMessage(int& bytes_received);
 
   /**
    * @brief Sends a message to the server.
    * @param message The message to send.
    * @return 0 on success, or an error code on failure.
    */
   int sendMessage(std::string message);
 
   /**
    * @brief Retrieves the buffer containing the received message.
    * @return A pointer to the buffer.
    */
   const char* getBuffer() const;
 
   /**
    * @brief Closes the socket connection.
    */
   void closeSocket();
 
  private:
   std::string _server_ip; ///< The IP address of the server.
   int _server_port;       ///< The port number of the server.
 
   sockaddr_in _server_addr; ///< Structure to hold server address information.
   int _socket;              ///< The socket file descriptor.
   char* _buffer;            ///< Buffer to hold received messages.
 };
 

#endif