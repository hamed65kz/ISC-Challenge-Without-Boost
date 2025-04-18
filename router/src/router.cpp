#include "router.h"
#include <thread>
#include "logger.h"
#include "message.h"


#define RECV_BUFF_SIZE 32 * 3000

void sleep(int milliseconds) {
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif  // _WIN32
}

int Router::start(unsigned threadCount, unsigned port) {
  // start worker-threads
  std::vector<std::thread> threads;
  threads.reserve(threadCount);
  for (int i = 0; i < threadCount; ++i) {
    threads.emplace_back(worker_thread_handler, i);
  }

  // init Sessions class, it preserve needed memory
  Sessions::init_sessions(MAX_CLIENTS_COUNT);

  // start TCP server and get its socket
  auto server_socket = TcpServer::start_tcp_server(port);
  if (server_socket == SOCKET_ERROR) return -1;

  // start event loop/listener
  start_event_listener(server_socket);

  return 0;
}
void Router::worker_thread_handler(int thread_id) {
  LOG_TRACE("Worker Thread {} started.", thread_id);
  // recv_buffer is a pre-allocated buffer for each thread.
  // this buffer will use for recv messages and we dont new memory for each reception.
  char *recv_buffer = new char[RECV_BUFF_SIZE]();
  while (true) {

    // do existing read event
    do_reads(recv_buffer);

    // do existing write task on dst sockets
    do_writes();

    sleep(20);
  }
  delete[] recv_buffer;
}
void Router::start_event_listener(int server_socket) {
  fd_set readfds;

  // this the event loop, listening to new events infinitely.
  while (true) {
    // reset descriptors set, reseting is demanded by select()
    int max_sd =
        reset_fd_set(readfds, server_socket, Sessions::get_accpeted_sockets());

    // Wait for activity or event, include connect new client, recv new data,
    // terminate client connections
    int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
    if (activity == SOCKET_ERROR) {
      int err = GET_SOCKET_ERROR() ;
      LOG_CRITICAL("Error on select() err code : {}",err);
      continue;
    }
    // Check server socket for new connection
    if (FD_ISSET(server_socket, &readfds)) {
      int new_client_socket = TcpServer::accept_client(server_socket);
      if (new_client_socket == SOCKET_ERROR) {
        // error_occured on accepting
        continue;
      }
      // register accepted socket on Sessions class, session is responsible for
      // managing connections
      Sessions::accept_client(new_client_socket);
      LOG_INFO("Accept new node request");
    }

    // push intruppted client sockets to the queue
    for (int i = 0; i < activity; i++) {
      int socket = readfds.fd_array[i];
      push_event_queue(socket);
    }
  }
}
int Router::reset_fd_set(fd_set &fd, int server_socket,
                         std::vector<int> clients_socket) {
  // reset fd to clear all
  FD_ZERO(&fd);

  // register server socket for monitoring
  FD_SET(server_socket, &fd);
  int max_sd = server_socket;

  // register clients socket for monitoring
  for (int i = 0; i < clients_socket.size(); i++) {
    int sd = clients_socket[i];
    FD_SET(sd, &fd);
    if (sd > max_sd) max_sd = sd;
  }
  return max_sd;
}
void Router::push_event_queue(int client_socket) {
  // push received events to the queue, workers will pop them and do recv()

  std::unique_lock<std::shared_mutex> lock(*read_queue_mutex_);
  if (ready_read_sockets_queue_.size() > 0) {
    if (ready_read_sockets_queue_.back() == client_socket) {
      return;
    }
  }
  ready_read_sockets_queue_.push(client_socket);
}
void Router::do_reads(char *recv_buffer) {
  int ready_read_socket = pop_ready_read_socket();
  if (ready_read_socket > 0) {
    // pick new incoming event socket
    auto src_session = Sessions::find_session_by_socket(ready_read_socket);
    if (src_session == nullptr) {
      // it may be removed since its, connection terminated.
      LOG_ERROR("The socket doesnt exist and alive yet.");
      return;
    } else {
      int session_id = src_session->get_id();
      auto socket_mutex = src_session->get_mutex();

      // lock with socket mutex, its serialize each socket reads and writes
      std::unique_lock<std::shared_mutex> lock(*socket_mutex);

      auto src_session = Sessions::find_session_by_socket(ready_read_socket);
      if (src_session == nullptr) {
        // it may be removed since its, connection terminated.
        LOG_ERROR("The socket doesnt exist and alive yet.");
        return;
      }

      int bytes_read = 0;
      do {
        if (session_id == NONE) {
          bytes_read = handle_handshake(ready_read_socket, recv_buffer);
        } else {
          bytes_read = process_message(ready_read_socket, recv_buffer);
        }

        // loop until there is no more data, or socket error
        // bytes_read is -1 on socket error
        // bytes_read is 0 on dst not found
        // bytes_read is 0 on no more data
      } while (bytes_read > 0);

      if (bytes_read == SOCKET_ERROR) {
        // remove halted socket and session from Session holder class
        LOG_ERROR("Error on socket recv, session will removed.");
        Sessions::removeSession(ready_read_socket);
      }
    }
  }
}
int Router::process_message(int ready_read_socket, char *recv_buffer) {
  // looking for Following 32 byte messages
  // here we recv 32 byte
  int bytes_read =
      TcpServer::read_async(ready_read_socket, recv_buffer, DATA_MESSAGE_SIZE);
  if (bytes_read == DATA_MESSAGE_SIZE) {
    // if 32 byte received, extract dst id, if destination node register
    // itself, forward msg to destination node

    // std::string message(recv_buffer, DATA_MESSAGE_SIZE);
    LOG_DEBUG("Received MSG : {}", recv_buffer);
    FLOG_INFO("Received MSG  : {}", recv_buffer);

    // add null terminating.
    recv_buffer[DATA_MESSAGE_SIZE] = 0;

    int dst_id = Message::extract_dst_id(recv_buffer, bytes_read);
    auto dst_session = Sessions::find_session_by_id(dst_id);
    if (dst_session == nullptr) {
      LOG_ERROR("Destination not found: {}", dst_id);
      return 0;
    } else {

      // ▄▀Performance Penalty▀▄ :
      // copy received bytes to new allocated string and push it to write queue
      // for better performance we could eliminate this string and
      // dont allocate any other memory and call kernel api anymore
      auto msg = std::string(recv_buffer, DATA_MESSAGE_SIZE);
      forward(dst_session, msg);
    }
  }
  else if(bytes_read  == SOCKET_ERROR){
    LOG_ERROR("read_async() return error. connection crashed or terminated by client");
  } 
  else {
    // message len is insuffcient. this bytes will drop unfortunately.
    LOG_ERROR("Received MSG length is insufficient for MSG processing, {} bytes will drop.", bytes_read);
  }
  return bytes_read;
}
int Router::handle_handshake(int ready_read_socket, char *recv_buffer) {
  // looking for first id msg, 3 byte
  // this session has'nt associated id, it means currently we should wait for id
  // msg here we recv just 3 byte as id msg
  int bytes_read =
      TcpServer::read_async(ready_read_socket, recv_buffer, ID_MESSAGE_SIZE);
  if (bytes_read == ID_MESSAGE_SIZE) {
    // if 3 byte received, convert it to int and keep it in Sessions holder
    // class

    // add null terminating.
    recv_buffer[DATA_MESSAGE_SIZE] = 0;

    int src_id = Message::extract_src_id(recv_buffer, bytes_read);
    Sessions::add_node(ready_read_socket, src_id);
    // it may be removed since its, connection terminated.
    LOG_INFO("Initiate a node with ID : {}", src_id);
  }
  else if(bytes_read  == SOCKET_ERROR){
    LOG_ERROR("read_async() return error. connection crashed or terminated by client");
  }  
  else {
    // message len is insuffcient. this bytes will drop unfortunately.
    LOG_ERROR(
        "Received MSG length is insufficient for node handshaking, {} bytes "
        "will drop.",
        bytes_read);
  }
  return bytes_read;
}
void Router::do_writes() {
  // pick write jobs from queue and do send on dst socket
  auto ready_write_socket_msg = pop_ready_write_socket();
  auto dst_socket = ready_write_socket_msg.first;
  if (dst_socket > 0) {
    std::string msg = ready_write_socket_msg.second;
    auto dst_session = Sessions::find_session_by_socket(dst_socket);
    if (dst_session != nullptr) {
      // socket still alive/exist
      auto socket_mutex = dst_session->get_mutex();
      std::unique_lock<std::shared_mutex> lock(*socket_mutex);
      // send msg to dst
      int sent_byte = TcpServer::send_to_client(dst_socket, msg);

      if (sent_byte == msg.size()) {
        LOG_TRACE("MSG Forwarded to : {}", dst_session->get_id());
        FLOG_INFO("Forwarded MSG : {}", msg);
      } else if (sent_byte == SOCKET_ERROR) {
        // remove halted/Errored socket and session from Session holder class
        LOG_ERROR("Error on socket send, session will removed.");
        Sessions::removeSession(dst_session->get_socket());
      } else {
        LOG_ERROR("Error on MSG sending. invalid sent byte {}", sent_byte);
      }
    } else {
      // dst session is not exist. it may terminated or Errored
      LOG_ERROR("Error on MSG sending: dst session is not exist.");
    }
  }
}

int Router::pop_ready_read_socket() {
  // pick new event/socket from quoue and deliver it to workers threads
  int size = ready_read_sockets_queue_.size();

  if (size > 0) {
    std::unique_lock<std::shared_mutex> lock(*read_queue_mutex_);
    if (ready_read_sockets_queue_.size() == 0) {
      return 0;
    }

    auto ready_socket = ready_read_sockets_queue_.front();
    ready_read_sockets_queue_.pop();

    return ready_socket;
  }
  return 0;
}
std::pair<int, std::string> Router::pop_ready_write_socket() {
  // pick new write job from quoue and deliver it to workers threads
  int size = ready_write_sockets_queue_.size();

  if (size > 0) {
    std::unique_lock<std::shared_mutex> lock(*write_queue_mutex_);
    if (ready_read_sockets_queue_.size() == 0) {
      lock.unlock();
      return std::make_pair(0, "");
    }
    auto ready_socket_msg = ready_write_sockets_queue_.front();
    ready_write_sockets_queue_.pop();
    return ready_socket_msg;
  }
  return std::make_pair(0, "");
}
void Router::forward(std::shared_ptr<Session> dst_session, std::string msg) {
  // push msg to write queue, worker will pop and do send on corresponding
  // sockets
  auto dst_socket = dst_session->get_socket();
  std::unique_lock<std::shared_mutex> lock(*write_queue_mutex_);
  ready_write_sockets_queue_.push(std::make_pair(dst_socket, msg));
  lock.unlock();
}



// initialize static variables
std::shared_ptr<std::shared_mutex> Router::read_queue_mutex_ =
    std::make_shared<std::shared_mutex>();
std::shared_ptr<std::shared_mutex> Router::write_queue_mutex_ =
    std::make_shared<std::shared_mutex>();
std::queue<int, std::list<int>> Router::ready_read_sockets_queue_ =
    std::queue<int, std::list<int>>();
std::queue<std::pair<int, std::string>, std::list<std::pair<int, std::string>>>
    Router::ready_write_sockets_queue_ =
        std::queue<std::pair<int, std::string>,
                   std::list<std::pair<int, std::string>>>();