
#include "member_session.h"

// Configuration
constexpr short PORT = 8080;



class Router {
    public:
     Router(io_context& io)
         : acceptor_(io, tcp::endpoint(tcp::v4(), PORT)),
           socket_(io),
           logger_("router.log", std::ios::app) {
       start_accept();
     }
   
    private:
     void start_accept() {
       acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
         cout<<"\nnew incoming request";
         if (!ec) {
           cout<<"\nmake new session and start it";
           std::make_shared<MemberSession>(std::move(socket_), members_, logger_)->start();
         }
         start_accept();
       });
     }
   
     tcp::acceptor acceptor_;
     tcp::socket socket_;
     std::unordered_map<int, std::shared_ptr<MemberSession>> members_;
     std::ofstream logger_;
   };