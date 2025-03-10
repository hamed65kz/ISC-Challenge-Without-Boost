#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
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
         std::unordered_map<int, std::shared_ptr<MemberSession>>& members,
         std::ofstream& logger)
         : socket_(std::move(socket)), members_(members), logger_(logger) {
           cout<<"\nnew session start";
         }
   
     void start() {
       // First read member ID (3 digits)
       cout<<"\nnew session started";
       
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
       cout<<"\nnew session start to handshake";
       id_ = std::stoi(std::string(id_buffer_.data(), 3));
   
       if (members_.size() >= MAX_MEMBERS) {
         log("Rejected connection - ID: " + std::to_string(id_));
         return;
       }
       if(members_.count(id_)){
         // release previous socket
         cout<< "\nNodeid Exist, restart connection , ID : " <<std::to_string(id_);
         log("Nodeid Exist, restart connection , ID : " + std::to_string(id_));
         members_.erase(id_);
       }
       members_[id_] = shared_from_this();
       cout<< "\nMember connected - ID: " <<std::to_string(id_);
       log("Member connected - ID: " + std::to_string(id_));
       start_reading();
     }
   
     void start_reading() {
       cout<< "\nwaiting for next message";
       async_read(
           socket_, buffer(message_buffer_, MESSAGE_LENGTH),
           [self = shared_from_this()](boost::system::error_code ec, size_t) {
             if (!ec) {
               cout<< "\nmessage recved";
               self->process_message();
               self->start_reading();
             }
             else{
               cout<<""<< ec;
             }
           });
     }
   
     void process_message() {
       std::string message(message_buffer_.data(), MESSAGE_LENGTH);
       log("Received: " + message);
       cout<<"\nReceived: " + message;
       // Extract destination ID (last 3 characters)
       int dest_id = std::stoi(message.substr(29, 3));
       if(dest_id == 100){
         int a=0;
       }
       if (auto it = members_.find(dest_id); it != members_.end()) {
         it->second->forward_message(message);
         log("Forwarded to: " + std::to_string(dest_id));
         cout<<"\nForwarded to: " + std::to_string(dest_id);
       } else {
         log("Destination not found: " + std::to_string(dest_id));
         cout<<"\nDestination not found: " + std::to_string(dest_id);
       }
     }
   
     void forward_message(const std::string& message) {
       auto message_ptr = std::make_shared<std::string>(message);
       async_write(
           socket_, boost::asio::buffer(*message_ptr),
           [self = shared_from_this(),message_ptr](boost::system::error_code ec, size_t) {
             if (ec) {
               self->log("Send error: " + ec.message());
               cout<<"\nSend error: " + ec.message();
             }
           });
     }
   
     void log(const std::string& message) {
       auto now = system_clock::now();
       auto time = system_clock::to_time_t(now);
   
       logger_ << "[" << std::put_time(std::localtime(&time), "%F %T") << "] "
               << "Member " << id_ << ": " << message << std::endl;
     }
   
     tcp::socket socket_;
     std::array<char, 3> id_buffer_;
     std::array<char, MESSAGE_LENGTH> message_buffer_;
     int id_ = -1;
     std::unordered_map<int, std::shared_ptr<MemberSession>>& members_;
     std::ofstream& logger_;
   };
   