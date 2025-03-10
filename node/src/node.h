#include "tcp_socket.h"
#include "message.h"


void sleep(int milliseconds) // Cross-platform sleep function
{
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif // _WIN32
}

class Node {
 public:
  Node(int nodeid,int dstid,bool initiate_messaging,std::string router_ip, int router_port) {
    this->_id = nodeid;
    this->_dstId = dstid;
    this->_initiate_messaging = initiate_messaging;
    this->_tcp_socket = new TCPSocket(router_ip, router_port);
  }
  ~Node() { 
    delete this->_tcp_socket; 
}
  void start() {
    while (true) {
      int byte_send = 0;
      int errcode = this->_tcp_socket->connect_to_server();
      if (errcode == 0) {
        std::string id_msg = Message::buildIdMessage(this->_id);
        byte_send = this->_tcp_socket->sendMessage(id_msg);
        if (_initiate_messaging) {
          std::string first_msg =
              Message::buildFirstMessage(this->_id, this->_dstId);
          byte_send = this->_tcp_socket->sendMessage(first_msg);
        }

        int recv_err = NO_ERROR;
        while (recv_err == NO_ERROR) {
          int recv_bytes = 0;
          recv_err = this->_tcp_socket->recvMessage(recv_bytes);
          if (recv_bytes > 0) {
            auto recv_buffer = this->_tcp_socket->getBuffer();
            std::string reply =
                Message::processMessage(this->_id, recv_buffer, recv_bytes);
            byte_send = this->_tcp_socket->sendMessage(reply);
          }
        }
      } else {
        sleep(5000);
        std::cout << "retry connection to router...\n";
      }
    }
  }

 private:

  int _id;
  int _dstId;
  TCPSocket* _tcp_socket;
  bool _initiate_messaging;
};