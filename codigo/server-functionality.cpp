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

void invite_opponent(ustring recvline, int *ci, int invitor, int pipe) {
    int invited = (int) recvline[3];
    if (ci[invited] == 0) {
        ci[invitor] = (1 << 5) * invited;
        ci[invited] = (1 << 5) * invitor + (1 << 4) + (1 << 3);
    } else {
        // Jogador está ocupado e o servidor recusa
        unsigned char sndline[MAXLINE + 1];
        InviteOpponentAckPackage p(0);
        ssize_t n = p.header_to_string(sndline);
        if (write(pipe, sndline, n) < 0) {
            printf("Reject :(\n");
            exit(11);
        }
    }
}

void process_invitation_ack(ustring recvline, int *clients_invitation,
                            int *port, int invited) {
    int invitor = clients_invitation[invited] / 32;
    InviteOpponentAckPackage p(0);
    p.string_to_header(recvline);

    if (p.resp == 0)
        clients_invitation[invited] = 0;
    else
        port[invited] = p.port;

    clients_invitation[invitor] += p.resp;
    clients_invitation[invitor] |= (1 << 3);
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

void send_invitation_package(int invitor, int pipe) {
    unsigned char sndline[MAXLINE + 1];
    InviteOpponentPackage p(invitor);
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
