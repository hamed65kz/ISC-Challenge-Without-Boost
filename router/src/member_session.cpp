#include "member_session.h"

#include "logger.h"

void MemberSession::start() {
  async_read(socket_, buffer(id_buffer_, 3),
             [self = shared_from_this()](boost::system::error_code ec, size_t) {
               if (!ec) {
                 self->handle_handshake();
               }
             });
}

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
