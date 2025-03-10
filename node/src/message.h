#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#define MSG_LEN 32
class Message{
    public:

    static std::string processMessage(int current_node_id, const char* message, int message_len) { 
        if(message_len != MSG_LEN)
            return "";

        int index=0;
        std::string src_id(message, 3);
        index+=3;
        std::string MTI(message + index,  4);
        index+=4;
        std::string TRACE(message + index  , 6);
        index+=6;
        std::string PAN(message   + index  , 16);
        index+=16;
        std::string dstid(message + index  , 3);

        int msg_dst = std::stoi(dstid);

        if(msg_dst != current_node_id)
            return "";

        int new_mti = std::stoi(MTI) + 10;
        new_mti = new_mti % 10000; 
        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << new_mti;
        std::string mti_str = oss.str();

        std::string response = dstid + mti_str + TRACE + PAN + src_id;
        return response;


    }
    static std::string buildFirstMessage(int src_node_id ,int dst_node_id ){
        std::string MTI="2200";
        std::string TRACE="123456";
        std::string PAN="1111111111111111";

        std::ostringstream oss;
        oss << std::setw(3) << std::setfill('0') << src_node_id;
        std::string srcid = oss.str();

        std::ostringstream oss2;
        oss2 << std::setw(3) << std::setfill('0') << dst_node_id;
        std::string dstid = oss2.str();


        std::string msg = srcid + MTI + TRACE + PAN + dstid;
        return msg; 
    }
    static std::string buildIdMessage(int current_node_id){
        std::ostringstream oss;
        oss << std::setw(3) << std::setfill('0') << current_node_id;
        std::string idmsg = oss.str();
        return idmsg;
    }
};