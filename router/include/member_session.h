#ifndef SESSION_H
#define SESSION_H

#include <boost/asio.hpp>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>


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
   
     void start();
    private:
     void handle_handshake();
     void start_reading();
     void process_message() ;
     void forward_message(const std::string& message) ;
     tcp::socket socket_;
     std::array<char, 3> id_buffer_;
     std::array<char, MESSAGE_LENGTH> message_buffer_;
     int id_ = -1;
     std::unordered_map<int, std::shared_ptr<MemberSession>>& members_;
   };

   #endif