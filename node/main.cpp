
#include "node.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    Logger::Initialize();
    //std::cout<<"argc = "<<argc;
    if (argc != 5) {
        LOG_CRITICAL("Insufficient Argument.\nUsage: ISC-Node.exe <id> <dstid> <router_ip> <router_port>");
        return 1;
    }
    try {
         
        int id = std::stoi(argv[1]);
        int dstid = std::stoi(argv[2]);
        std::string router_ip = argv[3];
        int router_port = std::stoi(argv[4]);

        LOG_INFO("Node {} started.", id, " started. ");
        LOG_INFO("Dst Node is : {}", dstid);
        LOG_INFO("Router is on {}:{}", router_ip, router_port);
        bool initiate_messaging = false;
        if (id == 3) {
          initiate_messaging = true;
        }
        Node node(id,dstid,initiate_messaging,router_ip,router_port);
        node.start();
    } catch (const std::exception& e) {
        LOG_ERROR("Error: " + std::string(e.what())) ;
    }

    return 0;
}