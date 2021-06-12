#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "packages.hpp"
#include "util.hpp"

using namespace std;

void cmd_switch(int sockfd) {
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
            len = create_user_package.header_to_string(sendline);
            break;
        }
        case LOGIN: {
            cin >> username >> password;
            LoginPackage login_package = LoginPackage(username, password);

            len = login_package.header_to_string(sendline);
            break;
        }
        case LOGOUT: {
            cout << "Deslogando" << endl;
            LogoutPackage logout_package = LogoutPackage();

            len = logout_package.header_to_string(sendline);
            break;
        }
        case PASSWD: {
            cin >> cur_password >> new_password;
            ChangePasswordPackage change_password_package =
                ChangePasswordPackage(cur_password, new_password);

            len = change_password_package.header_to_string(sendline);
            break;
        }
        case LIST: {
            ReqConnectedUsersPackage req_connected_users_package =
                ReqConnectedUsersPackage();

            len = req_connected_users_package.header_to_string(sendline);
            break;
        }
        case EXIT: {
            cout << "Exiting" << endl;
            /* TODO: Preciso dar free em algo? <12-06-21, Paiolla> */
            exit(0);
        }
    }
    print_in_hex(sendline, len);
    write(sockfd, sendline, len);
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

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

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton error for %s :(\n", argv[1]);
        exit(3);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stderr, "connect error :(\n");
        exit(4);
    }

    fprintf(stdout, "connected\n");
    string s;
    while (1) { cmd_switch(sockfd); }

    exit(0);
}
