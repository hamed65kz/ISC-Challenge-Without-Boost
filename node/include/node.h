#ifndef NODE_H
#define NODE_H


#include "tcp_socket.h"


/**
 * @class Node
 * @brief Represents a node that communicates with a destination node through a router.
 *
 * This class facilitates the establishment of a connection to a router and allows
 * for messaging between nodes. The node can either initiate messaging or wait for
 * messages from a destination node.
 */
class Node {
  public:
   /**
    * @brief Constructor for the Node class.
    * 
    * Initializes a Node object with the given parameters.
    *
    * @param nodeid The unique identifier for this node.
    * @param dstid The unique identifier for the destination node.
    * @param initiate_messaging A boolean indicating whether this node should initiate messaging.
    * @param router_ip The IP address of the router to which this node connects.
    * @param router_port The port number on the router for TCP communication.
    */
   Node(int nodeid, int dstid, bool initiate_messaging, std::string router_ip,
        int router_port) {
     this->_id = nodeid;
     this->_dstId = dstid;
     this->_initiate_messaging = initiate_messaging;
     this->_tcp_socket = new TCPSocket(router_ip, router_port);
   }
 
   /**
    * @brief Destructor for the Node class.
    * 
    * Cleans up the resources used by the Node object, particularly the TCP socket.
    */
   ~Node() { delete this->_tcp_socket; }
 
   /**
    * @brief Starts the messaging process for the node.
    * 
    * This function will handle the logic to start sending or receiving messages
    * based on the node's configuration.
    */
   void start();
 
  private:
   int _id;                 ///< Unique identifier for this node.
   int _dstId;              ///< Unique identifier for the destination node.
   TCPSocket* _tcp_socket;  ///< Pointer to the TCP socket used for communication.
   bool _initiate_messaging;///< Flag indicating if this node should initiate messaging.
 
   /**
    * @brief Sends a message to the destination node.
    * 
    * @param msg The message to be sent.
    * @return An integer indicating the status of the operation.  0 on success, or non-zero on failure.
    */
   int send_message(std::string msg);
 
   /**
    * @brief Subscribes to the router for incoming messages.
    * 
    * @return An integer indicating the status of the subscription operation. 0 on success, or non-zero on failure.
    */
   int subscribe_to_router();
 
   /**
    * @brief Sends an initiator message to the destination node.
    * 
    * @return An integer indicating the status of the message sending operation. 0 on success, or non-zero on failure.
    */
   int send_initiator_message();
 
   /**
    * @brief Closes the connection to the router.
    * 
    * This function cleans up any resources associated with the TCP connection.
    */
   void closeConnection();
 };
 
#endif