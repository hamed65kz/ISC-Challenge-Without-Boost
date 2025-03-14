// router.cpp : This file contains the 'main' function for message router
// element. Program execution begins and ends there.

#include "logger.h"
#include "router.h"

int main(int argc, char* argv[]) {
  try {
    Logger::Initialize("logs/router.log");
    // std::cout<<"argc = "<<argc;
    if (argc != 2) {
      LOG_CRITICAL(
          "Insufficient Argument.\nUsage: ISC-Router.exe <listen_port>");
      return 1;
    }

    int router_port = std::stoi(argv[1]);

    LOG_INFO("Router started to listen on {} port." , router_port);
    io_context io;
    Router router(io,router_port);
    io.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}