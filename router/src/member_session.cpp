#include "member_session.h"

#include "logger.h"

/**
 * @brief Starts the member session by initiating an asynchronous read operation.
 *
 * This method begins reading a handshake message from the member's socket.
 * Once the message is received, it calls the handle_handshake method to process it.
 * 
 * @note This method is intended to be called when a new connection is established.
 */
void MemberSession::start() {
  async_read(socket_, buffer(id_buffer_, 3),
             [self = shared_from_this()](boost::system::error_code ec, size_t) {
               if (!ec) {
                 self->handle_handshake();
               }
             });
}

/**
 * @brief Handles the handshake process upon receiving the initial message.
 *
 * This method processes the handshake message to extract the member ID.
 * It checks if the maximum number of members is reached and logs an error if so.
 * If the member ID already exists, it logs a warning and removes the previous session.
 * Finally, it adds the member to the members map and starts reading subsequent messages.
 */
void MemberSession::handle_handshake() {
  id_ = std::stoi(std::string(id_buffer_.data(), 3));

  if (members_.size() >= MAX_MEMBERS) {
    LOG_ERROR("Rejected connection - ID: {}", id_);
    return;
  }
  if (members_.count(id_)) {
    // release previous socket
    LOG_WARN("Nodeid Exist, restart connection , ID : {}", id_);
    members_.erase(id_);
  }
  members_[id_] = shared_from_this();
  LOG_INFO("Member connected - ID: {}", id_);
  start_reading();
}

/**
 * @brief Initiates reading of messages from the member's socket.
 *
 * This method sets up an asynchronous read operation to receive messages from the member.
 * Upon receiving a message, it calls the process_message method to handle it.
 * If an error occurs during reading, it logs an error message.
 */
void MemberSession::start_reading() {
  async_read(socket_, buffer(message_buffer_, MESSAGE_LENGTH),
             [self = shared_from_this()](boost::system::error_code ec, size_t) {
               if (!ec) {
                 self->process_message();
                 self->start_reading();
               } else {
                 LOG_ERROR("Error on Recv : {}", ec.message());
               }
             });
}

/**
 * @brief Processes the received message and forwards it to the appropriate destination.
 *
 * This method extracts the destination ID from the message and checks if the destination
 * member exists in the members map. If found, it forwards the message to that member.
 * If the destination is not found, it logs an error message.
 */
void MemberSession::process_message() {
  std::string message(message_buffer_.data(), MESSAGE_LENGTH);
  LOG_DEBUG("Received MSG : {}", message);
  FLOG_INFO("Received MSG  : {}", message);
  // Extract destination ID (last 3 characters)
  int dest_id = std::stoi(message.substr(29, 3));
  if (auto it = members_.find(dest_id); it != members_.end()) {
    it->second->forward_message(message);
    LOG_TRACE("MSG Forwarded to : {}", dest_id);
    FLOG_INFO("Forwarded MSG : {}", message);
  } else {
    LOG_ERROR("Destination not found: {}", dest_id);
  }
}


/**
 * @brief Forwards a message to the connected member's socket.
 *
 * This method sends the specified message to the member's socket using an asynchronous write operation.
 * If an error occurs during the sending process, it logs an error message.
 *
 * @param message The message to be sent to the member.
 */
void MemberSession::forward_message(const std::string& message) {
  auto message_ptr = std::make_shared<std::string>(message);
  async_write(socket_, boost::asio::buffer(*message_ptr),
              [self = shared_from_this(), message_ptr](
                  boost::system::error_code ec, size_t) {
                if (ec) {
                  LOG_ERROR("Send MSG error: {}", ec.message());
                }
              });
}
