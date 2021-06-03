#ifndef CLIENT_FUNCTIONALITY_CPP
#define CLIENT_FUNCTIONALITY_CPP

#include "client-functionality.hpp"

#include <string>

std::string opponent;

void pingback() { }

void create_user(std::string name, std::string password) { }

bool user_login(std::string name, std::string password) { }

bool change_password(std::string cur_password, std::string new_password) { }

void user_logout() { }

void show_all_connected_users() { }

bool invite_opponent(std::string user) { }

void start_match(bool moving_first, bool x) { }

void send_move(int row, int col) { }

void surrender() { }

void end_match(int score1) { }

int get_ping() { }

void show_classifications(int n) { }

void quit() { }

#endif /* ifndef CLIENT_FUNCTIONALITY_CPP */
