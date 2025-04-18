#include "tcpserver.h"
#include "logger.h"



int TcpServer::start_tcp_server(int port) {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOG_ERROR("WSAStartup failed");
        return SOCKET_ERROR;
    }
#endif

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        LOG_ERROR("Socket creation failed");
        return SOCKET_ERROR;
    }
    // Configure server address
    auto servAddr = sockaddr_in{};
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);

    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);

    //bind the socket to its local address
    int bindStatus = bind(server_socket, (struct sockaddr*)&servAddr,
        sizeof(servAddr));
    if (bindStatus < 0)
    {
        LOG_ERROR("Error binding socket to local address");
        return SOCKET_ERROR;
    }
    listen(server_socket, 5);
    return server_socket;
}
int TcpServer::accept_client(int server_socket) {
    int new_client = accept(server_socket, nullptr, nullptr);
    int err = set_client_socket_nonblocking(new_client);
    if (err == 0) {
        return new_client;
    }
    return err;
    
}

int TcpServer::set_client_socket_nonblocking(int client_socket) {
#ifdef _WIN32
    // Windows non-blocking setup
    u_long mode = 1;  // 1 = non-blocking, 0 = blocking
    if (ioctlsocket(client_socket, FIONBIO, &mode) != 0) {
        int error = WSAGetLastError();
        closesocket(client_socket);
        // Handle error (WSAGetLastError() codes)
        return SOCKET_ERROR;
    }
#else
    // Linux/Unix non-blocking setup
    int flags = fcntl(client_socket, F_GETFL, 0);
    if (fcntl(client_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        //perror("fcntl failed");
        close(client_socket);
        return SOCKET_ERROR;
    }
#endif
    return 0;
}

bool TcpServer::no_more_data() {
#ifdef _WIN32
    if (GET_SOCKET_ERROR() == WSAEWOULDBLOCK)
        return true;
#else
    if (errno == EWOULDBLOCK || errno == EAGAIN)
        return true;
#endif
    return false;
}
int TcpServer::read_async(int client_socket, char* buffer,int buffer_len) {

    size_t total_read = 0;

    // 3. Read all available data
    while (true) {
        int bytes_read = recv(client_socket, buffer + total_read,
            buffer_len - total_read, 0);

        if (bytes_read > 0) {
            total_read += bytes_read;
            if(total_read == buffer_len)
                break;
        }
        else if (bytes_read == SOCKET_ERROR) {

            if (no_more_data()) {
                // No more data available
                return total_read;
            }
            else {
                // error
                return SOCKET_ERROR;
            }
        }
        else if (bytes_read == 0){
            // 0 = Connection closed
            return SOCKET_ERROR;
        }
        else{
            // will not come here
            return SOCKET_ERROR;
        }
    }

    return total_read;
}

int TcpServer::send_to_client(int client_socket, std::string buffer) {
    int bytesSent = send(client_socket, buffer.c_str(), buffer.size(), 0);
    if (bytesSent != buffer.size()) {
        int err = GET_SOCKET_ERROR() ;
        LOG_ERROR("Error on send. err code : {}",err);
        return SOCKET_ERROR;
    }
    return bytesSent;
}