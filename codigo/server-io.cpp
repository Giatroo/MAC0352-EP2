#ifndef SERVER_IO_CPP
#define SERVER_IO_CPP

#include "server-io.hpp"

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

void serialize_users() { }

void deserialize_users() {
    total_users = (int *) global_malloc(sizeof(int));
    total_users[0] = 0;
    users = (user_t **) global_malloc(MAX_USERS * sizeof(user_t *));
    for (int i = 0; i < MAX_USERS; ++i) {
        users[i] = (user_t *) global_malloc(sizeof(user_t));
        users[i]->name = (char *) global_malloc(MAX_STR_LEN * sizeof(char));
        users[i]->password = (char *) global_malloc(MAX_STR_LEN * sizeof(char));
    }
}

void write_log_line(log_t) { }

#endif /* ifndef SERVER_IO_CPP */
