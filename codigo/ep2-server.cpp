#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
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

            bool success_login = user_login(login_package.user_login,
                                            login_package.user_password);
            if (success_login == true) {
                cout << "Sucesso" << endl;
                return_package = new LoginAckPackage((byte) 1);
            } else {
                cout << "Não sucesso" << endl;
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
    unsigned char recvline[MAXLINE + 1];
    ssize_t n;
    current_user = nullptr;

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

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n", argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    while (1) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            perror("accept :(\n");
            exit(5);
        }

        if ((childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            fprintf(stdout, "[Uma conexão aberta (PID = %d)]\n", getpid());
            close(listenfd);

            fprintf(stdout, "Usuários:\n");
            for (int i = 0; i < total_users[0]; i++) {
                cout << *users[i] << endl;
            }

            while ((n = read(connfd, recvline, MAXLINE)) > 0) {
                recvline[n] = 0;
                fprintf(stdout, "Recebido: ");
                print_in_hex(recvline, n);
                cmd_switch(recvline, n, connfd);
            }
            printf("[Uma conexão fechada]\n");
            exit(0);
        } else
            /**** PROCESSO PAI ****/
            close(connfd);
    }

    exit(0);
}
