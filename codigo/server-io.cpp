#ifndef SERVER_IO_CPP
#define SERVER_IO_CPP

#include "server-io.hpp"

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "util.hpp"

using namespace std;

user_t **users;
int *total_users;

std::ostream &operator<<(std::ostream &os, const user_t &user) {
    os << "Usuário [" << user.name << "]:" << endl;
    os << "\tSenha: " << user.password << endl
       << "\tScore: " << user.score << endl
       << "\tConectado: " << user.connected << endl
       << "\tEm jogo: " << user.in_match << endl;
    return os;
}

void serialize_users(bool server_shutdown) {
    ofstream user_file;
    user_file.open(USER_PATH);

    for (int i = 0; i < *total_users; i++) {
        string name(users[i]->name);
        string password(users[i]->password);
        user_file << "U" << endl;
        user_file << name << endl;
        user_file << password << endl;
        user_file << users[i]->score << endl;
        if (server_shutdown) {
            user_file << 0 << endl << 0 << endl;
        } else {
            user_file << users[i]->connected << endl;
            user_file << users[i]->in_match << endl;
        }
    }

    user_file.close();
}

void deserialize_users() {
    total_users = (int *) global_malloc(sizeof(int));
    *total_users = 0;
    users = (user_t **) global_malloc(MAX_USERS * sizeof(user_t *));
    for (int i = 0; i < MAX_USERS; ++i) {
        users[i] = (user_t *) global_malloc(sizeof(user_t));
        users[i]->name = (char *) global_malloc(MAX_STR_LEN * sizeof(char));
        users[i]->password = (char *) global_malloc(MAX_STR_LEN * sizeof(char));
    }

    ifstream user_file;
    user_file.open(USER_PATH);
    string in_str;
    int i = 0;

    while (user_file >> in_str) {
        if (in_str != "U") { std::cerr << "Something wrong" << std::endl; }
        user_file >> in_str;
        set_str(users[i]->name, in_str);
        user_file >> in_str;
        set_str(users[i]->password, in_str);

        user_file >> users[i]->score >> users[i]->connected >>
            users[i]->in_match;

        i++;
    }
    *total_users = i;
    user_file.close();
}

string get_log_time_str(tm *time) {
    string time_str = "";
    time_str += '[';
    time_str += time->tm_year + 1900;
    time_str += '-';
    time_str += time->tm_mon + 1;
    time_str += '-';
    time_str += time->tm_mday;
    time_str += ' ';
    time_str += time->tm_hour;
    time_str += ':';
    time_str += time->tm_min;
    time_str += ':';
    time_str += time->tm_sec;
    time_str += ']';
    return time_str;
}

void write_log_line(log_t log_entry, log_struct_t log_struct) {
    ofstream log_file;

    log_file.open(LOG_PATH, std::fstream::out | std::fstream::app);

    time_t t = time(0);
    tm *now = localtime(&t);

    char time_format[25];
    strftime(time_format, 25, "%c", now);
    log_file << "[" << time_format << "]: ";

    switch (log_entry) {
        case SERVER_STARTED: log_file << "server started."; break;
        case CLIENT_CONNECTED:
            log_file << "client connected. ip = " << log_struct.client_ip;
            break;
        case SUCCESS_LOGIN:
            log_file << "user success login. ip = " << log_struct.client_ip
                     << " name = " << log_struct.username;
            break;
        case UNSUCCESS_LOGIN:
            log_file << "user unsuccess login. ip = " << log_struct.client_ip
                     << " name = " << log_struct.username;
            break;
        case CLIENT_DISCONNECT:
            log_file << "client disconnected. ip = " << log_struct.client_ip;
            break;
        case MATCH_STARTED:
            log_file << "match started. ip1 = " << log_struct.player1_ip
                     << " name1 = " << log_struct.player1_name
                     << " ip2 = " << log_struct.player2_ip
                     << " name2 = " << log_struct.player2_name;
            break;
        case MATCH_FINISHED:
            log_file << "match finished";
            log_file << "match finished. ip_winner = " << log_struct.winner_ip
                     << " winner_name = " << log_struct.winner_name
                     << " ip_loser = " << log_struct.loser_ip
                     << " name_loser = " << log_struct.loser_name;
            break;
        case UNEXPECTED_DISCONNECT:
            log_file << "unexpected disconnect. ip = " << log_struct.client_ip;
            break;
        case SERVER_FINISHED: log_file << "server finished"; break;
    }
    log_file << std::endl;
    log_file.close();
}

#endif /* ifndef SERVER_IO_CPP */
