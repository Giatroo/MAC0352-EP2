#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

#include "client-functionality.hpp"
#include "packages.hpp"
#include "util.hpp"

using namespace std;

void cmd_switch(int sockfd, int uifds0, int invite_port) {
    string cmd;
    string username, password, cur_password, new_password;
    ssize_t len;
    uchar sendline[MAXLINE + 1], recvline[MAXLINE + 1];

    cout << getpid() << endl;
    cout << "Digite um comando:" << endl;
    cin >> cmd;

    switch (cmd_str_to_int(cmd)) {
        case ADDUSER: {
            cin >> username >> password;
            CreateUserPackage create_user_package =
                CreateUserPackage(username, password);
            len = create_user_package.header_to_string(sendline);
            print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LOGIN: {
            cin >> username >> password;
            LoginPackage login_package = LoginPackage(username, password);

            len = login_package.header_to_string(sendline);
            print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LOGOUT: {
            cout << "Deslogando" << endl;
            LogoutPackage logout_package = LogoutPackage();

            len = logout_package.header_to_string(sendline);
            print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case PASSWD: {
            cin >> cur_password >> new_password;
            ChangePasswordPackage change_password_package =
                ChangePasswordPackage(cur_password, new_password);

            len = change_password_package.header_to_string(sendline);
            print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LIST: {
            ReqConnectedUsersPackage req_connected_users_package =
                ReqConnectedUsersPackage();

            len = req_connected_users_package.header_to_string(sendline);
            print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LEADERS: {
            ReqClassificationsPackage req_classifications_package =
                ReqClassificationsPackage();

            len = req_classifications_package.header_to_string(sendline);
            print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case BEGIN: {
            InviteOpponentAckPackage p(0);
            p = invite_opponent(sockfd, uifds0);

            if ((p.resp & (1 << 0))) {
                printf("Usuário aceitou o jogo!\n");
                int pont =
                    start_match(false, ((p.resp & (1 << 1)) == 0),
                                ((p.resp & (1 << 2)) == 0), p.port, p.ip);
                end_match(pont, sockfd);
            } else {
                printf("Usuário recusou o jogo!\n");
            }
            break;
        }

        case YES:
        case NO: {
            int resp = answer_opponent(cmd);

            InviteOpponentAckPackage p(resp);
            p.port = invite_port;
            len = p.header_to_string(sendline);
            if (write(sockfd, sendline, len) < 0) {
                printf("Erro ao direcionar à saída :(\n");
                exit(11);
            }

            if ((resp & (1 << 0))) {
                int pont = start_match(true, (resp & (1 << 1)),
                                       (resp & (1 << 2)), invite_port, NULL);
                end_match(pont, sockfd);
            }

            break;
        }
        case EXIT: {
            cout << "Exiting" << endl;
            /* TODO: Preciso dar free em algo? <12-06-21, Paiolla> */
            exit(0);
        }
    }
}

int main(int argc, char **argv) {
    int sockfd;
    ssize_t n;
    struct sockaddr_in servaddr;
    uchar sendline[MAXLINE + 1], recvline[MAXLINE + 1];

    if (argc != 4) {
        fprintf(stderr, "usage: %s <IPaddress> <Port> <Port>\n", argv[0]);
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error :( \n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s :(\n", argv[1]);
        exit(3);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error :(\n");
        exit(4);
    }

    fprintf(stdout, "connected\n");

    int uifds[2];
    if (pipe(uifds)) {
        fprintf(stderr, "Erro ao criar pipe\n");
        exit(1);
    }

    pid_t childpid;
    pid_t *pid_pai = (pid_t *) global_malloc(sizeof(pid_t));
    pid_t *pid_ui = (pid_t *) global_malloc(sizeof(pid_t));
    pid_jogo_latencia = (pid_t *) global_malloc(sizeof(pid_t));
    pid_jogo_ui = (pid_t *) global_malloc(sizeof(pid_t));
    pid_jogo_pai = (pid_t *) global_malloc(sizeof(pid_t));
    *pid_pai = getpid();

    if ((childpid = fork()) == 0) {
        *pid_ui = getpid();

        cout << "UI cliente: " << getpid() << endl;
        // UI
        while (1) { cmd_switch(sockfd, uifds[0], atoi(argv[3])); }

        std::cout << "Alguma coisa deu ruim" << std::endl;
        kill(*pid_pai, SIGTERM);
    } else {
        cout << "Entrada cliente: " << getpid() << endl;
        // Entrada
        while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;
            
            // fprintf(stdout, "Recebido: ");
            // print_in_hex(recvline, n);

            if (recvline[0] == PINGREQ_PACKAGE)
                pingback(sockfd);
            else if (recvline[0] == INVITE_OPPONENT_PACKAGE) {
                printf("Usuário %d está te convidando para jogar um jogo!\n",
                       (int) recvline[3]);
                printf("Aceita o convite?(Digite 1 se sim e 0 se não)\n");
            } else if (recvline[0] == INVITE_OPPONENT_ACK_PACKAGE) {
                if (write(uifds[1], recvline, n) < 0) {
                    printf("Erro ao direcionar à saída :(\n");
                    exit(11);
                }
            } else if (recvline[0] == CREATE_USER_ACK_PACKAGE) {
                CreateUserAckPackage create_user_ack_package =
                    CreateUserAckPackage(recvline);

                if (create_user_ack_package.return_code) {
                    cout << "Criado com sucesso." << endl;
                } else {
                    cout << "Falha ao criar." << endl;
                }
            } else if (recvline[0] == LOGIN_ACK_PACKAGE) {
                LoginAckPackage login_ack_package = LoginAckPackage(recvline);

                if (login_ack_package.return_code) {
                    cout << "Logado com sucesso." << endl;
                } else {
                    cout << "Falha ao logar." << endl;
                }
            } else if (recvline[0] == CHANGE_PASSWORD_ACK_PACKAGE) {
                ChangePasswordAckPackage change_password_ack_package =
                    ChangePasswordAckPackage(recvline);

                if (change_password_ack_package.return_code) {
                    cout << "Senha trocada com sucesso." << endl;
                } else {
                    cout << "Falha ao trocar a senha." << endl;
                }
            } else if (recvline[0] == RESPONSE_ALL_CONNECTED_USERS_PACKAGE) {
                ResConnectedUsersPackage res_con_users_package =
                    ResConnectedUsersPackage(recvline);
                res_con_users_package.show_users();
            } else if (recvline[0] == RESPONSE_CLASSIFICATIONS_PACKAGE) {
                ResClassificationsPackage res_classifications_package =
                    ResClassificationsPackage(recvline);
                res_classifications_package.show_users();
            }
        }
        kill(*pid_jogo_pai, SIGTERM);
        kill(*pid_jogo_ui, SIGTERM);
        kill(*pid_jogo_latencia, SIGTERM);
    }

    global_free(pid_pai, sizeof(pid_t)), global_free(pid_ui, sizeof(pid_t));
    global_free(pid_jogo_pai, sizeof(pid_t));
    global_free(pid_jogo_ui, sizeof(pid_t));
    global_free(pid_jogo_latencia, sizeof(pid_t));

    close(uifds[0]), close(uifds[1]);
    std::cout<< "Matou todo mundo" << std::endl;
    exit(0);
}

