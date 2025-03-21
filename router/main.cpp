// router.cpp : This file contains the 'main' function for message router
// element. Program execution begins and ends there.

#include "logger.h"
#include "router.h"
#include <vector>
#include <thread>
#include <shared_mutex>

#define THREAD_COUNT 4

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

    LOG_INFO("Router started to listen on {} port.", router_port);

    io_context io_context;

    // Add signal handling for graceful shutdown
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { io_context.stop(); });

    Router router(io_context, router_port);

    // Create thread pool
    std::vector<std::thread> threads;
    threads.reserve(THREAD_COUNT);
    for (int i = 0; i < THREAD_COUNT; ++i) {
      threads.emplace_back([&io_context]() { io_context.run(); });
    }

    // Wait for all threads
    for (auto& thread : threads) {
      if (thread.joinable()) thread.join();
    }

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}