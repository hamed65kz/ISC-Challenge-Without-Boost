
#ifndef ROUTER_H
#define ROUTER_H
#include "member_session.h"
constexpr short PORT = 8090;


/**
 * @class Router
 * @brief A class that manages incoming member sessions and facilitates their connection.
 */
class Router {
  public:
      /**
       * @brief Constructs a Router object with the given I/O context.
       * @param io The I/O context to be used for asynchronous operations.
       */
      Router(io_context& io);
  
  private:
      /**
       * @brief Initiates the acceptance of new member sessions.
       */
      void start_accept();
  
      tcp::acceptor acceptor_;                         ///< Accepts incoming TCP connections.
      tcp::socket socket_;                             ///< Socket for the current connection.
      std::unordered_map<int, std::shared_ptr<MemberSession>> members_; ///< Active member sessions.
  };
  
   #endif