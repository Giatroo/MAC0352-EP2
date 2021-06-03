#ifndef SERVER_FUNCTIONALITY_CPP
#define SERVER_FUNCTIONALITY_CPP

#include "server-functionality.hpp"

#include <string>

#include "server-io.hpp"

bool pingreq() { }

user *create_user(std::string name, std::string password) { }

bool user_login(std::string name, std::string password) { }

bool change_password(std::string cur_password, std::string new_password) { }

void user_logout() { }

void show_all_connected_users() { }

bool invite_opponent(std::string client2) { }

void start_match(std::string client1, std::string client2) { }

void end_match(std::string client1, std::string client2, int score1) { }

void show_classifications(int n) { }

#endif /* ifndef SERVER_FUNCTIONALITY_CPP */
