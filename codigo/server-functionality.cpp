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
struct sockaddr_in client_addr;
int heartbeat_resp;

void cmd_switch(ustring recvline, int n, int connfd) {
    byte package_type;
    ssize_t len;
    std::string user, password, cur_password, new_password;
    uchar sendline[MAXLINE + 1];
    package_type = recvline[0];
    PackageTemplate *return_package = nullptr;

    switch (package_type) {
        case RECONNECT_PACKAGE: {
            std::cout << "User reconnecting" << std::endl;
            ReconnectPackage reconnect_pkg(recvline);
            if (reconnect_pkg.username != "") {
                *current_user = find_user_index(reconnect_pkg.username);
                debug(*current_user);
            }
            break;
        }
        case CREATE_USER_PACKAGE: {
            std::cout << "Creating user" << std::endl;
            CreateUserPackage create_user_package = CreateUserPackage(recvline);
            log_struct_t log_struct;

            user_t *new_user;
            new_user = create_user(create_user_package.username,
                                   create_user_package.password);

            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
            log_struct.username = create_user_package.username;

            if (new_user == nullptr) {
                write_log_line(UNSUCCESS_USER_CREATED, log_struct);
                return_package = new CreateUserAckPackage((byte) 0);
            } else {
                write_log_line(SUCCESS_USER_CREATED, log_struct);
                std::cout << *new_user << std::endl;
                return_package = new CreateUserAckPackage((byte) 1);
            }
            break;
        }
        case LOGIN_PACKAGE: {
            std::cout << "Logining in" << std::endl;
            LoginPackage login_package = LoginPackage(recvline);
            log_struct_t log_struct;

            bool success_login =
                user_login(login_package.user_login,
                           login_package.user_password, client_addr);

            log_struct.username = login_package.user_login;
            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);

            if (success_login == true) {
                std::cout << "Sucesso" << std::endl;
                write_log_line(SUCCESS_LOGIN, log_struct);
                return_package = new LoginAckPackage((byte) 1);
            } else {
                std::cout << "Não sucesso" << std::endl;
                write_log_line(UNSUCCESS_LOGIN, log_struct);
                return_package = new LoginAckPackage((byte) 0);
            }
            break;
        }
        case CHANGE_PASSWORD_PACKAGE: {
            std::cout << "Password change" << std::endl;
            ChangePasswordPackage change_password_package =
                ChangePasswordPackage(recvline);
            log_struct_t log_struct;

            bool success_change_pass =
                change_password(change_password_package.cur_password,
                                change_password_package.new_password);

            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
            log_struct.username = users[*current_user]->name;

            if (success_change_pass == true) {
                std::cout << "Sucesso" << std::endl;
                write_log_line(SUCCESS_CHANGE_PASS, log_struct);
                return_package = new ChangePasswordAckPackage((byte) 1);
            } else {
                std::cout << "Não sucesso" << std::endl;
                write_log_line(UNSUCCESS_CHANGE_PASS, log_struct);
                return_package = new ChangePasswordAckPackage((byte) 0);
            }
            break;
        }
        case LOGOUT_PACKAGE: {
            std::cout << "Logout" << std::endl;
            log_struct_t log_struct;

            if (*current_user == -1) { break; }

            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
            log_struct.username = users[*current_user]->name;

            bool success_logout = user_logout();

            if (success_logout == true) {
                write_log_line(SUCCESS_LOGOUT, log_struct);
                std::cout << "Sucesso" << std::endl;
            } else {
                std::cout << "Não sucesso" << std::endl;
            }
            break;
        }

        case REQUEST_ALL_CONNECTED_USERS_PACKAGE: {
            std::cout << "Listing" << std::endl;
            return_package = new ResConnectedUsersPackage();

            show_all_connected_users();

            break;
        }

        case REQUEST_CLASSIFICATIONS_PACKAGE: {
            std::cout << "Leaders" << std::endl;
            return_package = new ResClassificationsPackage();
            break;
        }
        case PINGBACK_PACKAGE: {
            heartbeat_resp = 1;
            break;
        }
        case INVITE_OPPONENT_PACKAGE: {
            /* TODO: Semaforizar?  */
            std::cout << "Convidando" << std::endl;
            invite_opponent(recvline, users[*current_user], connfd);
            break;
        }
        case INVITE_OPPONENT_ACK_PACKAGE: {
            process_invitation_ack(recvline, users[*current_user]);
            break;
        }
        case END_MATCH_PACKAGE: {
            // write_log_line(MATCH_FINISHED);

            if (is_invited(users[*current_user]->client_invitation)) {
                log_struct_t log_struct;
                int opponent_index =
                    users[*current_user]->client_invitation / (1 << 5);
                user_t *opponent = find_user(opponent_index);

                if (recvline[1] == 2) { // eu ganhei
                    log_struct.winner_ip = users[*current_user]->ip;
                    log_struct.winner_name = users[*current_user]->name;
                    log_struct.loser_ip = opponent->ip;
                    log_struct.loser_name = opponent->name;
                } else if (recvline[1] == 1) { // empate

                } else if (recvline[1] == 0) { // eu perdi
                    log_struct.loser_ip = users[*current_user]->ip;
                    log_struct.loser_name = users[*current_user]->name;
                    log_struct.winner_ip = opponent->ip;
                    log_struct.winner_name = opponent->name;

                }

                write_log_line(MATCH_FINISHED, log_struct);
            }

            users[*current_user]->score += (int) recvline[1];
            users[*current_user]->client_invitation = 0;
            users[*current_user]->in_match = false;

            break;
        }
    }

    if (return_package != nullptr) {
        len = return_package->header_to_string(sendline);
        print_in_hex(sendline, len);
        write(connfd, sendline, len);
    }
}

void * heartbeat_handler_thread(void * args){
	int connfd = *((int *)args);
    while (true) {
       	if (pingreq(connfd, &heartbeat_resp) == 0) {
            fprintf(stderr, "Cliente Morreu :(\n");
            break;
        }
    }

    users[*current_user]->connected = false;
    users[*current_user]->in_match = false;

    close(connfd);

    log_struct_t log_struct;
    log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
    write_log_line(CLIENT_DISCONNECT, log_struct);

    printf("[Uma conexão fechada (PID = %d)]\n", getpid());
    exit (0);

    return NULL;
}

void * invitation_handler_thread(void * args){
	int connfd = *((int *)args);
    while (true) {
       	/*
           	TODO: Semaforizar?
        */
        if (*current_user == -1) {
            sleep(1);
            continue;
        }

        if (new_update_client_invitation(
            users[*current_user]->client_invitation)) {
        	if (is_invited(users[*current_user]->client_invitation)) {
                int invitor_id = users[*current_user]->client_invitation /(1 << 5);
                user_t *invitor_user = find_user(invitor_id);
                send_invitation_package(invitor_user->name, connfd);
               	users[*current_user]->client_invitation ^= (1 << 3);
            } else {
                int invited = users[*current_user]->client_invitation /(1 << 5);
                int resp = users[*current_user]->client_invitation %(1 << 3);

              	user_t *invited_user = find_user(invited);

                send_invitation_ack_package(resp, invited_user->ip,\
                    invited_user->port,connfd);

                if (resp == 0)
                    users[*current_user]->client_invitation = 0;
                else
                    users[*current_user]->client_invitation ^=(1 << 3);
            }
        }

    	sleep(1);
    }

    return NULL;
}

void * entrada_handler_thread(void * args){
	int connfd = *((int *)args);
	unsigned char recvline[MAXLINE + 1];
	ssize_t n;
    while ((n = read(connfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        fprintf(stdout, "Recebido: ");
        print_in_hex(recvline, n);
        cmd_switch(recvline, n, connfd);
    }

    if(*current_user != -1){
	    users[*current_user]->connected = false;
	    users[*current_user]->in_match = false;
    }
    close(connfd);

    log_struct_t log_struct;
    log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
    write_log_line(CLIENT_DISCONNECT, log_struct);
	printf("[Uma conexão fechada (PID = %d)]\n", getpid());


    exit (0);
    return NULL;
}

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

    if (invited_user->client_invitation == 0 && invited_user->connected
        && users[*current_user]->name != invited_user->name) {
        invited_user->in_match = true;
        invitor_user->in_match = true;

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
    else {
        log_struct_t log_struct;
        log_struct.player1_ip = invited_user->ip;
        log_struct.player1_name = invited_user->name;
        log_struct.player2_ip = invitor_user->ip;
        log_struct.player2_name = invitor_user->name;
        write_log_line(MATCH_STARTED, log_struct);
        invited_user->port = p.port;
    }

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
