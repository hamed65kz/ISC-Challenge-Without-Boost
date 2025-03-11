#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <chrono>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>

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

const int BUFFER_SIZE = 1500;
#define NO_ERR 0

class TCPSocket {
 public:
  TCPSocket(std::string server_ip, int server_port) {
    this->_buffer = new char[BUFFER_SIZE];
    this->_server_ip = server_ip;
    this->_server_port = server_port;
  }
  ~TCPSocket() {
    delete[] _buffer;
#ifdef _WIN32
    closesocket(_socket);
    WSACleanup();
#else
    shutdown(_socket, SHUT_RDWR);
    close(_socket);
#endif
  }

  int connect_to_server() {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      std::cerr << "WSAStartup failed\n";
      return 1;
    }
#endif

    // Create socket
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1) {
      std::cerr << "Socket creation failed\n";
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
      std::cerr << "Connection failed\n";
#ifdef _WIN32
      closesocket(_socket);
      WSACleanup();
#else
      close(_socket);
#endif
      return 1;
    }

    std::stringstream ss;
    ss << "Connected to Server : [" << this->_server_ip <<":"<<this->_server_port<<"] \n" ;

    std::cout << ss.str();
    return NO_ERR;
  }
  int recvMessage(int& bytes_received) {
    // Receive response
    bytes_received =0;
    bytes_received = recv(_socket, _buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
      if (bytes_received == 0) {
        std::cout << "Connection closed by server\n";
      } else {
        std::cerr << "Receive failed\n";
      }
      return 1;
    }
    _buffer[bytes_received] =0;// zero terminating
    std::cout << "Server response: " << _buffer << "\n";
    return NO_ERR;
  }
  int sendMessage(std::string message) {
    int bytes_sent = send(_socket, message.c_str(), message.size(), 0);
    if (bytes_sent == -1) {
      std::cerr << "Send reply failed.\n";
    }
    std::cout << "Node response: " << message << "\n";
    return bytes_sent;
  }
  const char* getBuffer() const {
    return this->_buffer;  // Return a const pointer to the internal data
  }

 private:
  std::string _server_ip;
  int _server_port;

  sockaddr_in _server_addr;
  int _socket;
  char* _buffer;
};

#endif