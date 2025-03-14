#include "tcp_socket.h"

#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include "logger.h"

const int BUFFER_SIZE = 1500;
/**
 * @brief Constructs a TCPSocket instance with the specified server IP and port.
 * 
 * @param server_ip The IP address of the server to connect to.
 * @param server_port The port number of the server to connect to.
 */
TCPSocket::TCPSocket(std::string server_ip, int server_port) {
  this->_buffer = new char[BUFFER_SIZE];
  this->_server_ip = server_ip;
  this->_server_port = server_port;
}

/**
 * @brief Destructor for the TCPSocket class.
 * 
 * Cleans up allocated resources, including the buffer and closes the socket.
 */
TCPSocket::~TCPSocket() {
  delete[] _buffer;
  closeSocket();
}

/**
 * @brief Connects to the server using the specified server IP and port.
 * 
 * Initializes the socket, configures the address structure, and attempts to 
 * establish a connection to the server. Logs errors if the connection fails.
 * 
 * @return int Returns 0 on success, or 1 on failure.
 */
int TCPSocket::connect_to_server() {
#ifdef _WIN32
  // Initialize Winsock
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    LOG_ERROR("WSAStartup failed");
    return 1;
  }
#endif

  // Create socket
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket == -1) {
    LOG_ERROR("Socket creation failed");
    return 1;
  }
  // Configure server address
  _server_addr = sockaddr_in{};
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(this->_server_port);
  inet_pton(AF_INET, this->_server_ip.c_str(), &_server_addr.sin_addr);
  // Connect to server
  if (connect(_socket, reinterpret_cast<sockaddr*>(&_server_addr),
              sizeof(_server_addr)) == -1) {
    LOG_ERROR("Connection failed.");
    closeSocket();
    return 1;
  }

  LOG_INFO("Connected to Server : [{}:{}]", this->_server_ip,
           this->_server_port);
  return NO_ERR;
}

/**
 * @brief Receives a message from the connected server.
 * 
 * Reads data from the socket into the internal buffer and logs the received 
 * message. Sets the number of bytes received in the provided reference parameter.
 * 
 * @param bytes_received Reference to an integer that will hold the number of 
 * bytes received.
 * @return int Returns 0 on success, or 1 on failure.
 */
int TCPSocket::recvMessage(int& bytes_received) {
  // Receive response
  bytes_received = 0;
  bytes_received = recv(_socket, _buffer, BUFFER_SIZE - 1, 0);
  if (bytes_received <= 0) {
    if (bytes_received == 0) {
      LOG_ERROR("Connection closed by server");
      
    } else {
      LOG_ERROR("Receive failed");
    }
    return 1;
  }
  _buffer[bytes_received] = 0;  // zero terminating
  LOG_INFO("Received MSG : {}", std::string(_buffer));
  return NO_ERR;
}

/**
 * @brief Sends a message to the connected server.
 * 
 * Writes the specified message to the socket and logs the sent message.
 * 
 * @param message The message to be sent to the server.
 * @return int Returns the number of bytes sent, or -1 on failure.
 */
int TCPSocket::sendMessage(std::string message) {
  int bytes_sent = send(_socket, message.c_str(), message.size(), 0);
  if (bytes_sent == -1) {
    LOG_ERROR("Send reply failed.");
  }
  LOG_TRACE("Sent MSG : {}", message);
  return bytes_sent;
}

/**
 * @brief Retrieves the internal buffer.
 * 
 * Provides a constant pointer to the internal buffer used for receiving 
 * messages.
 * 
 * @return const char* Pointer to the internal buffer.
 */
const char* TCPSocket::getBuffer() const {
  return this->_buffer;  // Return a const pointer to the internal data
}

/**
 * @brief Closes the socket connection.
 * 
 * Handles the cleanup of the socket and releases any associated resources.
 */
void TCPSocket::closeSocket() { 
  #ifdef _WIN32
    closesocket(_socket);
    WSACleanup();
#else
    close(_socket);
#endif
}
