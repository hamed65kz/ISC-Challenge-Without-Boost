#include "router.h"

#include "logger.h"


/**
 * @brief Constructs a Router instance.
 * 
 * Initializes the acceptor to listen for incoming connections on the specified port.
 * 
 * @param io The I/O context used for asynchronous operations.
 */
Router::Router(io_context& io)
    : acceptor_(io, tcp::endpoint(tcp::v4(), PORT)), socket_(io) {
  start_accept();
}

/**
 * @brief Starts the asynchronous accept operation.
 * 
 * Listens for incoming connections and handles each accepted connection by 
 * creating a new MemberSession. Logs any errors encountered during the accept operation.
 */
void Router::start_accept() {
  acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
    LOG_INFO("Accept new node request");
    if (!ec) {
      std::make_shared<MemberSession>(std::move(socket_), members_)->start();
    } else {
      LOG_ERROR("Error on accepting node request : {}", ec.message());
    }
    start_accept();
  });
}