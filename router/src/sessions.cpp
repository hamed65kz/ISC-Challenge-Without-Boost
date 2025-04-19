#include "sessions.h"
#include <iostream>
#include "logger.h"

void Sessions::init_sessions(int max_clients_count) {
	accepted_clients_.reserve(max_clients_count);
}
void Sessions::accept_client(int client_socket) {
	std::unique_lock<std::shared_mutex> lock(*sessions_mutex_);
	auto it = std::find(accepted_clients_.begin(), accepted_clients_.end(), client_socket);
	if (it == accepted_clients_.end()) {
		// vector doesnt have client_socket
		accepted_clients_.push_back(client_socket);

	}
	if (sessions_by_socket_.count(client_socket) == 1) {
		removeSession(client_socket);
	}
	sessions_by_socket_[client_socket] = std::make_shared<Session>(client_socket);
}
const std::vector<int>& Sessions::get_accpeted_sockets() {
	return accepted_clients_;
}
void Sessions::add_node(int client_socket,int node_id){
	std::unique_lock<std::shared_mutex> lock(*sessions_mutex_);
	
	if (sessions_by_socket_.count(client_socket) == 0) {
		LOG_ERROR("Session terminated befoe id handshaking. id = {}", node_id);
		return;
	}

	if(node_id >= MAX_CLIENTS_COUNT ){
		LOG_ERROR("node is invalid. id = {}", node_id);
		return;
	}

	if (sessions_by_id_.count(node_id) == 1) {
		LOG_WARN("Nodeid Exist, restart connection , ID : {}", node_id);
	}


	auto& session = sessions_by_socket_[client_socket];
	session->set_id(node_id);
	sessions_by_id_[node_id] = session;
}
std::shared_ptr<Session> Sessions::find_session_by_socket(int client_socket) {
	std::shared_lock<std::shared_mutex> lock(*sessions_mutex_);
	if (sessions_by_socket_.count(client_socket) == 0) {
		return nullptr;
	}
	auto session = sessions_by_socket_[client_socket];
	return session;
}
std::shared_ptr<Session> Sessions::find_session_by_id(int node_id){
	std::shared_lock<std::shared_mutex> lock(*sessions_mutex_);
	if (sessions_by_id_.count(node_id) == 0) {
		return nullptr;
	}
	auto session = sessions_by_id_[node_id];
	return session;
}
void Sessions::removeSession(int socket){

	std::unique_lock<std::shared_mutex> lock(*sessions_mutex_);

	// remove from sessions_by_socket_ , sessions_by_id_
	if (sessions_by_socket_.count(socket)) {
		auto session = sessions_by_socket_[socket];
		sessions_by_socket_.erase(socket);

		int id = session->get_id();
		if (id != NONE) {
			session = sessions_by_id_[id];
			sessions_by_id_.erase(id);
		}
	}

	// remove socket from accepted_clients
	// find first ocuured item and remove it.
	auto it = std::find(accepted_clients_.begin(), accepted_clients_.end(), socket);
	if (it != accepted_clients_.end()) {
		accepted_clients_.erase(it);
	}
}

// sinitialize static variables
std::unordered_map<int, std::shared_ptr<Session>> Sessions::sessions_by_socket_ = std::unordered_map<int, std::shared_ptr<Session>>();
std::unordered_map<int, std::shared_ptr<Session>> Sessions::sessions_by_id_ = std::unordered_map<int, std::shared_ptr<Session>>();
std::vector<int> Sessions::accepted_clients_ = std::vector<int>();
std::shared_ptr<std::shared_mutex> Sessions::sessions_mutex_ = std::make_shared<std::shared_mutex>();