#include "node.h"

#include "logger.h"
#include "message.h"

#define MAX_RETRY 3
constexpr size_t MESSAGE_LENGTH = 32;

/**
 * @brief Cross-platform sleep function.
 * 
 * This function pauses the execution for a specified number of milliseconds.
 * It uses the appropriate sleep function based on the platform (Windows or Unix-like).
 * 
 * @param milliseconds Duration to sleep in milliseconds.
 */
void sleep(int milliseconds)  
{
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif  // _WIN32
}

/**
 * @brief Starts the Node's main operation loop.
 * 
 * This method continuously attempts to connect to the server, subscribes to a router,
 * and handles incoming messages. If any errors occur during the process,
 * it will attempt to reconnect after a specified delay.
 */
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

/**
 * @brief Sends a message to the server with retry logic.
 * 
 * This method attempts to send a given message up to a maximum number of retries
 * (defined by MAX_RETRY) if the initial send fails.
 * 
 * @param msg The message to be sent.
 * @return int Returns NO_ERR (0) if the message was sent successfully,
 *             otherwise returns an error code (1).
 */
int Node::send_message(std::string msg){
  int byte_send = 0;
  int retry_count = 0;
  while (byte_send <= 0 && retry_count < MAX_RETRY) {
    byte_send = this->_tcp_socket->sendMessage(msg);
    retry_count++;
  }

  if (byte_send > 0) return NO_ERR;  // successfully sent
  return 1; // error occurred
}

/**
 * @brief Subscribes to the router by sending an identification message.
 * 
 * This method constructs an ID message using the node's ID and sends it to the router.
 * 
 * @return int Returns the result of the send operation.
 */
int Node::subscribe_to_router() {
  int byte_send = 0;
  std::string id_msg = Message::buildIdMessage(this->_id);
  return send_message(id_msg);
}

/**
 * @brief Sends an initiator message to the destination if the messaging is initiated.
 * 
 * This method constructs the first message using the node's ID and destination ID
 * and sends it if the `_initiate_messaging` flag is set to true.
 * 
 * @return int Returns 0 if no message is sent (if messaging is not initiated),
 *             otherwise returns the result of the send operation.
 */
int Node::send_initiator_message() {
  if (_initiate_messaging) {
    std::string first_msg = Message::buildFirstMessage(this->_id, this->_dstId);
    return send_message(first_msg);
  } else {
    return 0;  // successfully sent (no action taken)
  }
}

/**
 * @brief Closes the connection to the server.
 * 
 * This method closes the TCP socket associated with the Node,
 * effectively terminating the connection to the server.
 */
void Node::closeConnection(){
  this->_tcp_socket->closeSocket();
}
