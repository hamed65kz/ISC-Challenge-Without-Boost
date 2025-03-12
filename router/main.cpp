// router.cpp : This file contains the 'main' function for message router
// element. Program execution begins and ends there.

#include "src/router.h"


    
int main() {
  

  try {
    Logger::Initialize("logs/router.log"); 
    LOG_INFO("Router started to listen on "+std::to_string(PORT)+" port.");
    io_context io;
    Router router(io);
    io.run();
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}