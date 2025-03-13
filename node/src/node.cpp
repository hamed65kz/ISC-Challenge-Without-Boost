#include "node.h"

#include "logger.h"
#include "message.h"

#define MAX_RETRY 3
constexpr size_t MESSAGE_LENGTH = 32;

void sleep(int milliseconds)  // Cross-platform sleep function
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif  // _WIN32
}

void Node::start() {
  while (true) {
    int byte_send = 0;
    int errcode = this->_tcp_socket->connect_to_server();
    if (errcode == NO_ERR) {

      int err = subscribe_to_router();
      if (err != NO_ERR) {closeConnection();continue;}

      err = send_initiator_message();
      if (err != NO_ERR) {closeConnection();continue;}

      err = NO_ERR;
      while (err == NO_ERR) {

        int recv_bytes = 0;
        err = this->_tcp_socket->recvMessage(recv_bytes);
        if (err != NO_ERR) {closeConnection();continue;}

        if (recv_bytes == MESSAGE_LENGTH) {
          auto recv_buffer = this->_tcp_socket->getBuffer();
          std::string reply =
              Message::processMessage(this->_id, recv_buffer, recv_bytes);
              err = send_message(reply);
        }
        else{
          LOG_WARN("Received MSG Len is invalid. Len = {}",recv_bytes);
        }
      }
    } else {
      sleep(5000);
      LOG_WARN("retry connection to router...");
    }
  }
}
int Node::send_message(std::string msg){
  int byte_send = 0;
  int retry_count = 0;
  while (byte_send <= 0 && retry_count < MAX_RETRY) {
    byte_send = this->_tcp_socket->sendMessage(msg);
    retry_count++;
  }

  if (byte_send > 0) return NO_ERR;  // successfuly sent
  return 1; // error occured
}
int Node::subscribe_to_router() {
  int byte_send = 0;
  std::string id_msg = Message::buildIdMessage(this->_id);
  return send_message(id_msg);
}
int Node::send_initiator_message() {
  if (_initiate_messaging) {
    std::string first_msg = Message::buildFirstMessage(this->_id, this->_dstId);
    return send_message(first_msg);
  } else {
    return 0;  // successfuly sent
  }
}
void Node::closeConnection(){
  this->_tcp_socket->closeSocket();
}