#include "router.h"

#include "logger.h"

/**
 * @brief Constructs a Router instance.
 *
 * Initializes the acceptor to listen for incoming connections on the specified
 * port.
 *
 * @param io The I/O context used for asynchronous operations.
 * @param router_port the port router listen to it
 */
Router::Router(io_context& io, int router_port)
    : acceptor_(io, tcp::endpoint(tcp::v4(), router_port)),
      socket_(io),
      router_port_(router_port),
      members_mutex_(std::make_shared<std::shared_mutex>()) {
  start_accept();
}

/**
 * @brief Starts the asynchronous accept operation.
 *
 * Listens for incoming connections and handles each accepted connection by
 * creating a new MemberSession. Logs any errors encountered during the accept
 * operation.
 */
void Router::start_accept() {
  acceptor_.async_accept(socket_, [this](boost::system::error_code ec) {
    LOG_INFO("Accept new node request");
    if (!ec) {
      std::shared_ptr<MemberSession> session = std::make_shared<MemberSession>(
          std::move(socket), members_, members_mutex_,
          acceptor_.get_executor());
      session->start();
    } else {
      LOG_ERROR("Error on accepting node request : {}", ec.message());
    }
    start_accept();
  });
}

/**
 * @brief expose members mutex
 */
std::shared_ptr<std::shared_mutex> Router::get_members_mutex() {
  return members_mutex_;
}