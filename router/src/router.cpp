#include "router.h"
#include "logger.h"

// Configuration




Router::Router(io_context& io)
: acceptor_(io, tcp::endpoint(tcp::v4(), PORT)),
  socket_(io){
start_accept();
}


void Router::start_accept() {
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