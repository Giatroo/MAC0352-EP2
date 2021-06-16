#ifndef SERVER_FUNCTIONALITY_CPP
#define SERVER_FUNCTIONALITY_CPP

#include "server-functionality.hpp"

#include <cstring>
#include <iostream>
#include <string>

#include "server-io.hpp"
#include "util.hpp"

user_t *current_user;

user_t *find_user(std::string name) {
    user_t *ret_user = nullptr;
    for (int i = 0; i < total_users[0]; i++) {
        if (!strcmp(users[i]->name, name.c_str())) {
            ret_user = users[i];
            break;
        }
    }
    return ret_user;
}

user_t *create_user(std::string name, std::string password) {
    if (find_user(name) != nullptr) {
        // user already exists
        return nullptr;
    }

    printf("Creating user. Total = %d\n", total_users[0]);

    int i = total_users[0];
    set_str(users[i]->name, name);
    set_str(users[i]->password, password);
    users[i]->score = 0;
    users[i]->in_match = false;
    users[i]->connected = false;
    total_users[0]++;
    return users[i];
}

bool user_login(std::string name, std::string password) {
    user_t *user = find_user(name);
    if (user == nullptr) {
        std::cerr << "Usuário não existe" << std::endl;
        return false;
    }

    if (strcmp(user->password, password.c_str())) {
        std::cerr << "Senha errada" << std::endl;
        return false;
    }

    if (user->connected) {
        std::cerr << "Já conectado" << std::endl;
        return false;
    }

    user->connected = true;
    current_user = user;
    std::cout << "Logado com sucesso" << std::endl;
    return true;
}

bool change_password(std::string cur_password, std::string new_password) {
    if (current_user == nullptr) {
        std::cerr << "Not logged in" << std::endl;
        return false;
    }

    if (strcmp(current_user->password, cur_password.c_str())) {
        std::cerr << "Senha incorreta" << std::endl;
        return false;
    }

    set_str(current_user->password, new_password);
    return true;
}

bool user_logout() {
    if (current_user == nullptr) {
        std::cerr << "Not logged in" << std::endl;
        return false;
    }

    current_user->connected = false;
    current_user = nullptr;
    std::cout << "Deslogado com sucesso" << std::endl;
    return true;
}

void show_all_connected_users() {
    std::cout << "Exibindo todos os usuários:" << std::endl;
    for (int i = 0; i < *total_users; i++)
        std::cout << *users[i] << std::endl << std::endl;
}

void show_classifications(int n) { }

bool invite_opponent(std::string client2) { return false; }

void start_match(std::string client1, std::string client2) { }

void end_match(std::string client1, std::string client2, int score1) { }

bool pingreq() { return false; }

#endif /* ifndef SERVER_FUNCTIONALITY_CPP */
