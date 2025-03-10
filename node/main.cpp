
#include "src/node.h"

int main(int argc, char* argv[]) {
    //std::cout<<"argc = "<<argc;
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <id> <dstid> <router_ip> <router_port>\n";
        return 1;
    }
    try {
        int id = std::stoi(argv[1]);
        int dstid = std::stoi(argv[2]);
        std::string router_ip = argv[3];
        int router_port = std::stoi(argv[4]);

        bool initiate_messaging= false;
        if(id == 3){
            initiate_messaging =true;
        }
        Node node(id,dstid,initiate_messaging,router_ip,router_port);
        node.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}