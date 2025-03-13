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

class TCPSocket {
 public:
  TCPSocket(std::string server_ip, int server_port) ;
  ~TCPSocket() ;

  int connect_to_server() ;
  int recvMessage(int& bytes_received) ;   
  int sendMessage(std::string message);
  const char* getBuffer() const;

 private:
  std::string _server_ip;
  int _server_port;

  sockaddr_in _server_addr;
  int _socket;
  char* _buffer;
};

#endif