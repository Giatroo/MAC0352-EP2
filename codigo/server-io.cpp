#ifndef SERVER_IO_CPP
#define SERVER_IO_CPP

#include "server-io.hpp"

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
}

void write_log_line(log_t) { }

#endif /* ifndef SERVER_IO_CPP */
