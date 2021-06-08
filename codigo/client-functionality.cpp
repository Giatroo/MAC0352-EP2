#ifndef CLIENT_FUNCTIONALITY_CPP
#define CLIENT_FUNCTIONALITY_CPP

#include "client-functionality.hpp"

#include <string>

std::string opponent;

void create_user(std::string name, std::string password) { }

bool user_login(std::string name, std::string password) { return false; }

bool change_password(std::string cur_password, std::string new_password) {
    return false;
}

void user_logout() { }

void show_all_connected_users() { }

void show_classifications(int n) { }

bool invite_opponent(std::string user) { return false; }

void start_match(bool moving_first, bool x) { }

void send_move(int row, int col) { }

void surrender() { }

void end_match(int score1) { }

void pingback() { }

int get_ping() { return 0; }

void quit() { }

#endif /* ifndef CLIENT_FUNCTIONALITY_CPP */
