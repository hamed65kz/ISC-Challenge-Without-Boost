
#ifndef ROUTER_H
#define ROUTER_H
#include "member_session.h"

// Configuration
constexpr short PORT = 8090;



class Router {
    public:
     Router(io_context& io)
         : acceptor_(io, tcp::endpoint(tcp::v4(), PORT)),
           socket_(io){
       start_accept();
     }
   
    private:
     void start_accept() {
       acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
        LOG_INFO("Accept new node request");
         if (!ec) {
           std::make_shared<MemberSession>(std::move(socket_), members_)->start();
         }
         else{
          LOG_ERROR("Error on accepting node request : "+ ec.message());
         }
         start_accept();
       });
     }
   
     tcp::acceptor acceptor_;
     tcp::socket socket_;
     std::unordered_map<int, std::shared_ptr<MemberSession>> members_;

   };
   
   #endif