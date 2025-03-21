
#ifndef ROUTER_H
#define ROUTER_H
#include "member_session.h"

/**
 * @class Router
 * @brief A class that manages incoming member sessions and facilitates their
 * connection.
 */
class Router {
 public:
  /**
   * @brief Constructs a Router object with the given I/O context.
   * @param io The I/O context to be used for asynchronous operations.
   * @param router_port the port router listen to it
   */
  Router(io_context& io, int router_port);

  /**
   * @brief expose members mutex
   */
  std::shared_ptr<std::shared_mutex> get_members_mutex();

 private:
  /**
   * @brief Initiates the acceptance of new member sessions.
   */
  void start_accept();
  int router_port_;         ///< the port router listen to it
  tcp::acceptor acceptor_;  ///< Accepts incoming TCP connections.
  tcp::socket socket_;      ///< Socket for the current connection.
  std::unordered_map<int, std::shared_ptr<MemberSession>>
      members_;  ///< Active member sessions.
  std::shared_ptr<std::shared_mutex>
      members_mutex_;  ///< mutex for control access to members_ in threads
};

#endif