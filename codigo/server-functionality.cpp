#ifndef SERVER_FUNCTIONALITY_CPP
#define SERVER_FUNCTIONALITY_CPP

#include "server-functionality.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <string>

#include "server-io.hpp"
#include "util.hpp"

int *current_user;

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

user_t *find_user(int i) { return users[i]; }

int find_user_index(std::string name) {
    int i;
    for (i = *total_users - 1; i >= 0; i--)
        if (!strcmp(users[i]->name, name.c_str())) break;
    return i;
}

user_t *create_user(std::string name, std::string password) {
    if (find_user(name) != nullptr) {
        // user already exists
        return nullptr;
    }
    if (*current_user != -1) {
        // user is already logged in
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

bool user_login(std::string name, std::string password,
                struct sockaddr_in client_addr) {
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

    if (*current_user != -1) {
        // user is already logged in
        std::cerr << "Você já está logado" << std::endl;
        return false;
    }

    user->connected = true;
    user->ip = inet_ntoa(client_addr.sin_addr);
    user->port = (int) ntohs(client_addr.sin_port);
    user->client_invitation = 0;
    *current_user = find_user_index(user->name);
    std::cout << "Logado com sucesso" << std::endl;
    return true;
}

bool change_password(std::string cur_password, std::string new_password) {
    if (*current_user == -1) {
        std::cerr << "Not logged in" << std::endl;
        return false;
    }

    if (strcmp(users[*current_user]->password, cur_password.c_str())) {
        std::cerr << "Senha incorreta" << std::endl;
        return false;
    }

    set_str(users[*current_user]->password, new_password);
    return true;
}

bool user_logout() {
    if (*current_user == -1) {
        std::cerr << "Not logged in" << std::endl;
        return false;
    }

    users[*current_user]->connected = false;
    *current_user = -1;
    std::cout << "Deslogado com sucesso" << std::endl;
    return true;
}

void show_all_connected_users() {
    std::cout << "Exibindo todos os usuários:" << std::endl;
    for (int i = 0; i < *total_users; i++)
        std::cout << *users[i] << std::endl << std::endl;
}

void show_classifications(int n) { }

void invite_opponent(ustring recvline, user_t *invitor_user, int pipe) {
    if (*current_user == -1) {
        std::cerr << "Not logged in" << std::endl;
        return;
    }

    InviteOpponentPackage inv_pkg(recvline);

    if (invitor_user == nullptr) {
        std::cerr << "Deve estar logado para convidar" << std::endl;
        unsigned char sndline[MAXLINE + 1];
        InviteOpponentAckPackage p(0);
        ssize_t n = p.header_to_string(sndline);
        if (write(pipe, sndline, n) < 0) {
            printf("Reject :(\n");
            exit(11);
        }
        return;
    }

    user_t *invited_user = find_user(inv_pkg.cliente);

    if (invited_user == nullptr) {
        // O jogador convidado não existe
        std::cerr << "O jogador convidado não existe" << std::endl;
        unsigned char sndline[MAXLINE + 1];
        InviteOpponentAckPackage p(0);
        ssize_t n = p.header_to_string(sndline);
        if (write(pipe, sndline, n) < 0) {
            printf("Reject :(\n");
            exit(11);
        }
        return;
    }

    int invited_id = find_user_index(invited_user->name);
    int invitor_id = find_user_index(invitor_user->name);

    if (invited_user->client_invitation == 0 && invited_user->connected) {
        invitor_user->client_invitation = (1 << 5) * invited_id;
        invited_user->client_invitation =
            (1 << 5) * invitor_id + (1 << 4) + (1 << 3);
        debug(invited_user->name);
        debug(invited_user->client_invitation);
    } else {
        // Jogador está ocupado e o servidor recusa
        std::cerr << "O jogador está ocupado" << std::endl;
        unsigned char sndline[MAXLINE + 1];
        InviteOpponentAckPackage p(0);
        ssize_t n = p.header_to_string(sndline);
        if (write(pipe, sndline, n) < 0) {
            printf("Reject :(\n");
            exit(11);
        }
    }
}

void process_invitation_ack(ustring recvline, user_t *invited_user) {
    int invitor_id = invited_user->client_invitation / (1 << 5);
    user_t *invitor_user = find_user(invitor_id);
    InviteOpponentAckPackage p(0);
    p.string_to_header(recvline);

    if (p.resp == 0)
        invited_user->client_invitation = 0;
    else
        invited_user->port = p.port;

    invitor_user->client_invitation += p.resp;
    invitor_user->client_invitation |= (1 << 3);
    // Comunica o processo do invitor de que teve resposta
}

void start_match(std::string client1, std::string client2) { }

void end_match(std::string client1, std::string client2, int score1) { }

int pingreq(int pipe, int *heartbeat_resp) {
    PingReqPackage p;
    unsigned char sndline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);
    if (write(pipe, sndline, n) < 0) {
        printf("Reject :(\n");
        exit(11);
    }
    *heartbeat_resp = 0;
    sleep(10);

    return *heartbeat_resp;
}

bool new_update_client_invitation(int client_invitation) {
    return (((1 << 3) & client_invitation) != 0);
}

void send_invitation_package(std::string invitor_name, int pipe) {
    unsigned char sndline[MAXLINE + 1];
    InviteOpponentPackage p(invitor_name);
    ssize_t n = p.header_to_string(sndline);
    if (write(pipe, sndline, n) < 0) {
        printf("Reject :(\n");
        exit(11);
    }
}

void send_invitation_ack_package(int resp, char *ip, int port, int pipe) {
    unsigned char sndline[MAXLINE + 1];
    InviteOpponentAckPackage p(resp);
    if (resp != 0) {
        p.ip = ip;
        p.port = port;
    }

    ssize_t n = p.header_to_string(sndline);
    if (write(pipe, sndline, n) < 0) {
        printf("Reject :(\n");
        exit(11);
    }
}

bool is_invited(int client_invitation) {
    return (((1 << 4) & client_invitation) != 0);
}

#endif /* ifndef SERVER_FUNCTIONALITY_CPP */
