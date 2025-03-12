#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include "logger.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std::chrono;

constexpr int MAX_MEMBERS = 999;
constexpr size_t MESSAGE_LENGTH = 32;


class MemberSession : public std::enable_shared_from_this<MemberSession> {
    public:
     MemberSession(
         tcp::socket socket,
         std::unordered_map<int, std::shared_ptr<MemberSession>>& members)
         : socket_(std::move(socket)), members_(members){

         }
   
     void start() {  
       async_read(
           socket_, buffer(id_buffer_, 3),
           [self = shared_from_this()](boost::system::error_code ec, size_t) {
             if (!ec) {          
               self->handle_handshake();
             }
           });
     }
   
    private:
     void handle_handshake() {
       id_ = std::stoi(std::string(id_buffer_.data(), 3));
   
       if (members_.size() >= MAX_MEMBERS) {

         LOG_ERROR("Rejected connection - ID: " + std::to_string(id_));
         return;
       }
       if(members_.count(id_)){
         // release previous socket
         LOG_WARN("Nodeid Exist, restart connection , ID : " + std::to_string(id_));
         members_.erase(id_);
       }
       members_[id_] = shared_from_this();
       LOG_INFO("Member connected - ID: " + std::to_string(id_));
       start_reading();
     }
   
     void start_reading() {
       async_read(
           socket_, buffer(message_buffer_, MESSAGE_LENGTH),
           [self = shared_from_this()](boost::system::error_code ec, size_t) {
             if (!ec) {
               self->process_message();
               self->start_reading();
             }
             else{
              LOG_ERROR("Error on Recv : "+ ec.message());
             }
           });
     }
   
     void process_message() {
       std::string message(message_buffer_.data(), MESSAGE_LENGTH);
       LOG_DEBUG("Received MSG : " + message);
       FLOG_INFO("Received MSG  : " + message);
       // Extract destination ID (last 3 characters)
       int dest_id = std::stoi(message.substr(29, 3));
       if (auto it = members_.find(dest_id); it != members_.end()) {
         it->second->forward_message(message);
         LOG_TRACE("MSG Forwarded to: " + std::to_string(dest_id));
         FLOG_INFO("Forwarded MSG : " + message);
       } else {
        LOG_ERROR("Destination not found: " + std::to_string(dest_id));
       }
     }
   
     void forward_message(const std::string& message) {
       auto message_ptr = std::make_shared<std::string>(message);
       async_write(
           socket_, boost::asio::buffer(*message_ptr),
           [self = shared_from_this(),message_ptr](boost::system::error_code ec, size_t) {
             if (ec) {
              LOG_ERROR("Send MSG error: " + ec.message());
             }
           });
     }
   

   
     tcp::socket socket_;
     std::array<char, 3> id_buffer_;
     std::array<char, MESSAGE_LENGTH> message_buffer_;
     int id_ = -1;
     std::unordered_map<int, std::shared_ptr<MemberSession>>& members_;
   };

   #endif