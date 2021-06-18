#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <iostream>

#include "packages.hpp"
#include "server-functionality.hpp"
#include "server-io.hpp"
#include "util.hpp"

using namespace std;

int *heartbeat_resp;
int *pid_clients;
int *clients_num;
int ind;
struct sockaddr_in client_addr;

void cmd_switch(ustring recvline, int n, int connfd) {
    byte package_type;
    ssize_t len;
    string user, password, cur_password, new_password;
    uchar sendline[MAXLINE + 1];
    package_type = recvline[0];
    PackageTemplate *return_package = nullptr;

    switch (package_type) {
        case CREATE_USER_PACKAGE: {
            cout << "Creating user" << endl;
            CreateUserPackage create_user_package = CreateUserPackage(recvline);

            user_t *new_user;
            new_user = create_user(create_user_package.username,
                                   create_user_package.password);

            if (new_user == nullptr) {
                return_package = new CreateUserAckPackage((byte) 0);
            } else {
                cout << *new_user << endl;
                return_package = new CreateUserAckPackage((byte) 1);
            }
            break;
        }
        case LOGIN_PACKAGE: {
            cout << "Logining in" << endl;
            LoginPackage login_package = LoginPackage(recvline);
            log_struct_t log_struct;

            bool success_login =
                user_login(login_package.user_login,
                           login_package.user_password, client_addr);

            log_struct.username = login_package.user_login;
            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);

            if (success_login == true) {
                cout << "Sucesso" << endl;
                write_log_line(SUCCESS_LOGIN, log_struct);
                return_package = new LoginAckPackage((byte) 1);
            } else {
                cout << "Não sucesso" << endl;
                write_log_line(UNSUCCESS_LOGIN, log_struct);
                return_package = new LoginAckPackage((byte) 0);
            }
            break;
        }
        case CHANGE_PASSWORD_PACKAGE: {
            cout << "Password change" << endl;
            ChangePasswordPackage change_password_package =
                ChangePasswordPackage(recvline);

            bool success_change_pass =
                change_password(change_password_package.cur_password,
                                change_password_package.new_password);

            if (success_change_pass == true) {
                cout << "Sucesso" << endl;
                return_package = new ChangePasswordAckPackage((byte) 1);
            } else {
                cout << "Não sucesso" << endl;
                return_package = new ChangePasswordAckPackage((byte) 0);
            }
            break;
        }
        case LOGOUT_PACKAGE: {
            cout << "Logout" << endl;
            bool success_logout = user_logout();

            if (success_logout == true) {
                cout << "Sucesso" << endl;
            } else {
                cout << "Não sucesso" << endl;
            }
            break;
        }

        case REQUEST_ALL_CONNECTED_USERS_PACKAGE: {
            cout << "Listing" << endl;
            return_package = new ResConnectedUsersPackage();

            show_all_connected_users();

            break;
        }

        case REQUEST_CLASSIFICATIONS_PACKAGE: {
            cout << "Leaders" << endl;
            return_package = new ResClassificationsPackage();
            break;
        }
        case PINGBACK_PACKAGE: {
            *heartbeat_resp = 1;
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
            printf("Jogador %d com pontuação %d\n", ind, (int) recvline[1]);
            break;
        }
    }

    if (return_package != nullptr) {
        len = return_package->header_to_string(sendline);
        print_in_hex(sendline, len);
        write(connfd, sendline, len);
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t childpid;
    unsigned char recvline[MAXLINE + 1], sndline[MAXLINE + 1];
    ssize_t n;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    // inicializa os usuários lendo eles do banco de dados
    deserialize_users();
    /*
        HARADA - VARIÁVEIS
    */
    const int MAX_CLIENTS = 10;
    pid_clients = (int *) global_malloc(MAX_CLIENTS * sizeof(int));
    clients_num = (int *) global_malloc(sizeof(int));
    /*
        FIM - HARADA - VARIÁVEIS
    */
    *clients_num = 1;

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n", argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    log_struct_t log_struct;
    write_log_line(SERVER_STARTED, log_struct);

    while (1) {
        socklen_t clen;

        if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr,
                             &clen)) == -1) {
            perror("accept :(\n");
            exit(5);
        }

        if ((childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            fprintf(stdout, "[Uma conexão aberta (PID = %d)]\n", getpid());
            close(listenfd);
            printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));
            printf("port is: %d\n", (int) ntohs(client_addr.sin_port));

            log_struct_t log_struct;
            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
            write_log_line(CLIENT_CONNECTED, log_struct);

            current_user = (int *) global_malloc(sizeof(int));
            *current_user = -1;

            fprintf(stdout, "Usuários:\n");
            for (int i = 0; i < total_users[0]; i++) {
                cout << *users[i] << endl;
            }

            /*
                TODO : semaforizar isso
            */
            ind = *clients_num;
            pid_clients[ind] = getpid();
            *clients_num += 1;

            int pipefds[2];
            if (pipe(pipefds)) {
                fprintf(stderr, "Erro ao criar pipe\n");
                exit(1);
            }

            pid_t *pid_pai = (pid_t *) global_malloc(sizeof(pid_t));
            pid_t *pid_heartbeat = (pid_t *) global_malloc(sizeof(pid_t));
            pid_t *pid_invitation = (pid_t *) global_malloc(sizeof(pid_t));
            heartbeat_resp = (int *) global_malloc(sizeof(int));
            *pid_pai = getpid();

            if ((childpid = fork()) == 0) {
                *pid_heartbeat = getpid();

                // Heartbeats
                while (true) {
                    if (pingreq(connfd, heartbeat_resp) == 0) {
                        fprintf(stderr, "Cliente Morreu :(\n");
                        close(connfd);
                        break;
                    }
                }
                kill(*pid_invitation, SIGTERM), kill(*pid_pai, SIGTERM);
            } else if ((childpid = fork()) == 0) {
                *pid_invitation = getpid();

                // Invitation
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
                        if (is_invited(
                                users[*current_user]->client_invitation)) {
                            int invitor_id =
                                users[*current_user]->client_invitation /
                                (1 << 5);
                            user_t *invitor_user = find_user(invitor_id);
                            send_invitation_package(invitor_user->name, connfd);
                            users[*current_user]->client_invitation ^= (1 << 3);
                        } else {
                            int invited =
                                users[*current_user]->client_invitation /
                                (1 << 5);
                            int resp = users[*current_user]->client_invitation %
                                       (1 << 3);

                            user_t *invited_user = find_user(invited);

                            send_invitation_ack_package(resp, invited_user->ip,
                                                        invited_user->port,
                                                        connfd);

                            if (resp == 0)
                                users[*current_user]->client_invitation = 0;
                            else
                                users[*current_user]->client_invitation ^=
                                    (1 << 3);
                        }
                    }
                    sleep(1);
                }
            } else {
                // Entrada
                while ((n = read(connfd, recvline, MAXLINE)) > 0) {
                    recvline[n] = 0;
                    fprintf(stdout, "Recebido: ");
                    print_in_hex(recvline, n);
                    cmd_switch(recvline, n, connfd);
                }
                kill(*pid_heartbeat, SIGTERM), kill(*pid_invitation, SIGTERM);
            }
            close(connfd);
            users[*current_user]->connected = false;
            users[*current_user]->in_match = false;
            serialize_users(false);
            global_free(pid_pai, sizeof(pid_t));
            global_free(pid_heartbeat, sizeof(pid_t));
            global_free(pid_invitation, sizeof(pid_t));
            global_free(heartbeat_resp, sizeof(int));
            printf("[Uma conexão fechada (PID = %d)] %ld\n", getpid(), n);

            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
            write_log_line(CLIENT_DISCONNECT, log_struct);

            exit(0);

        } else
            /**** PROCESSO PAI ****/
            close(connfd);
    }

    serialize_users(true);
    global_free(pid_clients, MAX_CLIENTS);

    exit(0);
}
