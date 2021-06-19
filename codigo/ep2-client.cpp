#include <arpa/inet.h>
#include <pthread.h>
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

int sockfd;
int uifds[2];
struct sockaddr_in servaddr;
string cur_username;

void cmd_switch() {
    string cmd;
    string username, password, cur_password, new_password;
    ssize_t len;
    uchar sendline[MAXLINE + 1];

    cout << "Digite um comando:" << endl;
    cin >> cmd;
    switch (cmd_str_to_int(cmd)) {
        case ADDUSER: {
            cin >> username >> password;
            CreateUserPackage create_user_package =
                CreateUserPackage(username, password);
            len = create_user_package.package_to_string(sendline);

            if (DEBUG) print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LOGIN: {
            cin >> username >> password;
            LoginPackage login_package = LoginPackage(username, password);

            len = login_package.package_to_string(sendline);

            if (DEBUG) print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            cur_username = username;

            break;
        }
        case LOGOUT: {
            LogoutPackage logout_package = LogoutPackage();

            len = logout_package.package_to_string(sendline);

            if (DEBUG) print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case PASSWD: {
            cin >> cur_password >> new_password;
            ChangePasswordPackage change_password_package =
                ChangePasswordPackage(cur_password, new_password);

            len = change_password_package.package_to_string(sendline);

            if (DEBUG) print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LIST: {
            ReqConnectedUsersPackage req_connected_users_package =
                ReqConnectedUsersPackage();

            len = req_connected_users_package.package_to_string(sendline);

            if (DEBUG) print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case LEADERS: {
            ReqClassificationsPackage req_classifications_package =
                ReqClassificationsPackage();

            len = req_classifications_package.package_to_string(sendline);

            if (DEBUG) print_in_hex(sendline, len);
            write(sockfd, sendline, len);

            break;
        }
        case BEGIN: {
            InviteOpponentAckPackage p(0);
            p = invite_opponent(sockfd, uifds[0]);

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
            p.port = get_free_port();
            len = p.package_to_string(sendline);
            if (write(sockfd, sendline, len) < 0) {
                printf("Erro ao direcionar à saída :(\n");
                exit(11);
            }

            if ((resp & (1 << 0))) {
                int pont = start_match(true, (resp & (1 << 1)),
                                       (resp & (1 << 2)), p.port, NULL);
                end_match(pont, sockfd);
            }

            break;
        }
        case EXIT: {
            cout << "Exiting" << endl;
            /* TODO: Preciso dar free em algo? <12-06-21, Paiolla> */
            close(sockfd);
            close(uifds[0]), close(uifds[1]);
            std::cout << "Matou todo mundo" << std::endl;
            exit(0);
        }
    }
}

void *ui(void *arg) {
    while (1) { cmd_switch(); }
    std::cout << "Alguma coisa deu ruim" << std::endl;
    return NULL;
}

void *entrada(void *arg) {
    uchar sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    ssize_t n;

    while (true) {
        cout << "Servidor voltou!" << endl;
        while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;

            if (DEBUG) {
                fprintf(stdout, "Recebido: ");
                print_in_hex(recvline, n);
            }

            if (recvline[0] == PINGREQ_PACKAGE)
                pingback(sockfd);
            else if (recvline[0] == INVITE_OPPONENT_PACKAGE) {
                InviteOpponentPackage p(recvline);
                cout << "Usuário " << p.cliente
                     << " está te convidando para jogar um jogo!" << endl;
                cout << "Aceita o convite? (yes|no)" << endl;
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
                    cur_username = "";
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
        int new_socket;
        cout << "Servidor caiu!" << getpid() << endl;
        sleep(1);
        if ((new_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "socket error :( \n");
            exit(2);
        }
        close(sockfd);

        bool voltou = false;
        for (int i = 0; i < 180; i++) {
            if (connect(new_socket, (struct sockaddr *) &servaddr,
                        sizeof(servaddr)) < 0) {
                sleep(1);
            } else {
                debug(new_socket);
                sockfd = new_socket;
                voltou = true;

                ReconnectPackage reconnect_pkg(cur_username);
                n = reconnect_pkg.package_to_string(sendline);
                write(sockfd, sendline, n);

                break;
            }
        }
        if (!voltou) {
            cout << "Servidor caiu!" << endl;
            close(sockfd);
            exit(0);
            break;
        }
        sleep(1);
    }
    return NULL;
}

int main(int argc, char **argv) {
    cur_username = "";

    if (argc != 3) {
        fprintf(stderr, "usage: %s <IPaddress> <Port>\n", argv[0]);
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket error :( \n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &(servaddr.sin_addr)) <= 0) {
        fprintf(stderr, "inet_pton error for %s :(\n", argv[1]);
        exit(3);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error :(\n");
        exit(4);
    }

    if (DEBUG) { fprintf(stdout, "connected\n"); }

    if (pipe(uifds)) {
        fprintf(stderr, "Erro ao criar pipe\n");
        exit(1);
    }

    pid_jogo_latencia = (pid_t *) global_malloc(sizeof(pid_t));
    pid_jogo_ui = (pid_t *) global_malloc(sizeof(pid_t));

    pthread_t ui_thread, entrada_thread;
    if (pthread_create(&ui_thread, NULL, ui, NULL)) {
        printf("Erro ao criar thread UI\n");
        exit(1);
    }
    if (pthread_create(&entrada_thread, NULL, entrada, NULL)) {
        printf("Erro ao criar thread Entrada\n");
        exit(1);
    }

    if (pthread_join(ui_thread, NULL)) {
        printf("Erro ao dar join na thread ui\n");
        exit(1);
    }
    if (pthread_join(entrada_thread, NULL)) {
        printf("Erro ao dar join na thread entrada\n");
        exit(1);
    }

    close(sockfd);
    close(uifds[0]), close(uifds[1]);
    global_free(pid_jogo_ui, sizeof(pid_t));
    global_free(pid_jogo_latencia, sizeof(pid_t));
    if (DEBUG) std::cout << "Matou todo mundo" << std::endl;
    exit(0);
}
