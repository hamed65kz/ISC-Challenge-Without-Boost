#ifndef NODE_H
#define NODE_H


#include "tcp_socket.h"


class Node {
 public:
  Node(int nodeid, int dstid, bool initiate_messaging, std::string router_ip,
       int router_port) {
    this->_id = nodeid;
    this->_dstId = dstid;
    this->_initiate_messaging = initiate_messaging;
    this->_tcp_socket = new TCPSocket(router_ip, router_port);
  }
  ~Node() { delete this->_tcp_socket; }
  void start();

 private:
  int _id;
  int _dstId;
  TCPSocket* _tcp_socket;
  bool _initiate_messaging;
};
#endif