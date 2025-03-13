
#ifndef ROUTER_H
#define ROUTER_H
#include "member_session.h"
constexpr short PORT = 8090;


class Router {
    public:
     Router(io_context& io);
   
    private:
     void start_accept() ;
   
     tcp::acceptor acceptor_;
     tcp::socket socket_;
     std::unordered_map<int, std::shared_ptr<MemberSession>> members_;

   };
   
   #endif