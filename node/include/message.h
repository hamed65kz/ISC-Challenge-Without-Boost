#ifndef MESSAGE_H
#define MESSAGE_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define MSG_LEN 32

/**
 * @class Message
 * @brief A class responsible for generating and processing messages between
 * nodes.
 *
 * This class provides functionality to process received messages, build
 * responses, and create initial messages for communication between nodes. The
 * messages follow a predefined format and has a fixed length.
 */
class Message {
 public:
  /**
   * @brief Processes a received message and generates a response.
   *
   * This method checks if the incoming message length is equal to the expected
   * length (MSG_LEN). If the destination ID in the message matches the current
   * node ID, it increments the Message Type Identifier (MTI) and builds a
   * response message to be sent back.
   *
   * @param current_node_id The ID of the current node processing the message.
   * @param message Pointer to the received message.
   * @param message_len Length of the received message.
   *
   * @return A string containing the response message if the conditions are met;
   *         otherwise, returns an empty string.
   */
  static std::string processMessage(int current_node_id, const char* message,
                                    int message_len) {
    if (message_len != MSG_LEN) return "";

    int index = 0;
    std::string src_id(message, 3);
    index += 3;
    std::string MTI(message + index, 4);
    index += 4;
    std::string TRACE(message + index, 6);
    index += 6;
    std::string PAN(message + index, 16);
    index += 16;
    std::string dstid(message + index, 3);

    int msg_dst = std::stoi(dstid);

    if (msg_dst != current_node_id) return "";

    int new_mti = std::stoi(MTI) + 10;
    new_mti = new_mti % 10000;
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << new_mti;
    std::string mti_str = oss.str();

    std::string response = dstid + mti_str + TRACE + PAN + src_id;
    return response;
  }

  /**
   * @brief Builds the initial message to be sent from one node to another.
   *
   * This method constructs a message with a predefined format, including the
   * source node ID, Message Type Identifier (MTI), a trace number, a PAN,
   * and the destination node ID.
   *
   * @param src_node_id The ID of the source node sending the message.
   * @param dst_node_id The ID of the destination node receiving the message.
   *
   * @return A string containing the constructed initial message.
   */
  static std::string buildFirstMessage(int src_node_id, int dst_node_id) {
    std::string MTI = "2200";
    std::string TRACE = "123456";
    std::string PAN = "1111111111111111";

    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << src_node_id;
    std::string srcid = oss.str();

    std::ostringstream oss2;
    oss2 << std::setw(3) << std::setfill('0') << dst_node_id;
    std::string dstid = oss2.str();

    std::string msg = srcid + MTI + TRACE + PAN + dstid;
    return msg;
  }

  /**
   * @brief Builds a message containing only the ID of the current node.
   *
   * This method constructs a message string representing the current node's ID.
   *
   * @param current_node_id The ID of the current node.
   *
   * @return A string containing the formatted current node ID.
   */
  static std::string buildIdMessage(int current_node_id) {
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << current_node_id;
    std::string idmsg = oss.str();
    return idmsg;
  }
};
#endif