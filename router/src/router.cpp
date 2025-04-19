#include "router.h"

#include <thread>

#include "logger.h"
#include "message.h"
#include "tcpserver.h"


#define RECV_BUFF_SIZE 32 * 3000

void sleep(int milliseconds) {
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif  // _WIN32
}

int Router::start(unsigned thread_count, unsigned port) {
  // start worker-threads
  std::vector<std::thread> threads;
  threads.reserve(thread_count);

  int read_thread_count = thread_count / 2;
  int write_thread_count = thread_count / 2;
  if (thread_count % 2 == 1) {
    read_thread_count++;
  }

  for (int i = 0; i < read_thread_count; ++i) {
    threads.emplace_back(worker_thread_read_handler, i);
  }

  for (int i = read_thread_count; i < thread_count; ++i) {
    threads.emplace_back(worker_thread_write_handler, i);
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
void Router::worker_thread_read_handler(int thread_id) {
  LOG_TRACE("Read Worker Thread {} started.", thread_id);
  // recv_buffer is a pre-allocated buffer for each thread.
  // this buffer will use for recv messages and we dont new memory for each
  // reception.
  char *recv_buffer = new char[RECV_BUFF_SIZE]();
  while (true) {
    //  Retrieves a socket that is ready for reading from the queue.
    int ready_read_socket = ready_read_sockets_queue_.pop();
    // do existing read event
    do_reads(ready_read_socket, recv_buffer);
  }
  delete[] recv_buffer;
}
void Router::worker_thread_write_handler(int thread_id) {
  LOG_TRACE("Write Worker Thread {} started.", thread_id);

  while (true) {
    //  Retrieves a socket that is ready for writing from the queue.
    auto ready_write_task = ready_write_sockets_queue_.pop();
    // do existing write task on dst sockets
    do_writes(ready_write_task);
  }
}
void Router::start_event_listener(int server_socket) {
  fd_set readfds;

  // this the event loop, listening to new events infinitely.
  while (true) {
    // reset descriptors set, reseting is demanded by select()
    int max_sd = TcpServer::reset_fd_set(readfds, server_socket,
                                         Sessions::get_accpeted_sockets());

    // Wait for activity or event, include connect new client, recv new data,
    // terminate client connections
    int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
    if (activity == SOCKET_ERROR) {
      int err = GET_SOCKET_ERROR();
      LOG_CRITICAL("Error on select() err code : {}", err);
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
      LOG_INFO("Accept new node request {}.",new_client_socket);
    }

    // push intruppted client sockets to the queue
    for (int i = 0; i < activity; i++) {
      int socket = readfds.fd_array[i];
      if (socket != server_socket) {
        bool dont_push_if_repeated_event = true;
        ready_read_sockets_queue_.push(socket, dont_push_if_repeated_event);
      }
    }
  }
}

void Router::do_reads(int ready_read_socket, char *recv_buffer) {
  // find session related to this socket
  auto src_session = Sessions::find_session_by_socket(ready_read_socket);
  if (src_session == nullptr) {
    // it may be removed since its, connection terminated.
    LOG_ERROR("The socket doesnt exist and alive yet.1");
    return;
  } else {
    
    auto socket_mutex = src_session->get_mutex();

    // lock with socket mutex, its serialize each socket reads and writes
    std::unique_lock<std::shared_mutex> lock(*socket_mutex);

    auto src_session = Sessions::find_session_by_socket(ready_read_socket);
    if (src_session == nullptr) {
      // it may be removed since its, connection terminated.
      LOG_ERROR("The socket doesnt exist and alive yet.2");
      return;
    }

    int bytes_read = 0;
    do {
      int session_id = src_session->get_id();
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
  } else if (bytes_read == SOCKET_ERROR) {
    LOG_ERROR(
        "read_async() return error. connection crashed or terminated by "
        "client");
  } else if (bytes_read != 0) {
    // message len is insuffcient. this bytes will drop unfortunately.
    LOG_ERROR(
        "Received MSG length is insufficient for MSG processing, {} bytes will "
        "drop.",
        bytes_read);
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
    recv_buffer[ID_MESSAGE_SIZE] = 0;

    int src_id = Message::extract_src_id(recv_buffer, bytes_read);
    Sessions::add_node(ready_read_socket, src_id);
    // it may be removed since its, connection terminated.
    LOG_INFO("Initiate a node with ID : {}", src_id);
  } else if (bytes_read == SOCKET_ERROR) {
    LOG_ERROR(
        "read_async() return error. connection crashed or terminated by "
        "client");
  } else if( bytes_read != 0) {
    // message len is insuffcient. this bytes will drop unfortunately.
    LOG_ERROR(
        "Received MSG length is insufficient for node handshaking, {} bytes "
        "will drop.",
        bytes_read);
  }
  return bytes_read;
}
void Router::do_writes(std::pair<int, std::string> task) {
  // pick write task from queue and do send on dst socket
  auto dst_socket = task.first;
  std::string msg = task.second;

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
void Router::forward(std::shared_ptr<Session> dst_session, std::string msg) {
  // push msg to write queue, worker will pop and do send on corresponding
  // sockets
  auto dst_socket = dst_session->get_socket();
  ready_write_sockets_queue_.push(std::make_pair(dst_socket, msg));
}

// initialize static variables

SignalingQueue<int> Router::ready_read_sockets_queue_;

SignalingQueue<std::pair<int, std::string>> Router::ready_write_sockets_queue_;