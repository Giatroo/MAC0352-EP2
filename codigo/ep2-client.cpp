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
    string user, password, cur_password, new_password;
    size_t len;
    ustring line;
    cout << "Digite um comando:" << endl;
    cin >> cmd;
    cout << "cmd '" << cmd << "'" << endl;

    switch (cmd_str_to_int(cmd)) {
        case ADDUSER: {
            cin >> user >> password;
            CreateUserPackage *cup = new CreateUserPackage();
            cup->username = user;
            cup->password = password;
            line = cup->header_to_string(len);
            print_in_hex(line, len);
            write(sockfd, line, len);
            break;
        }
        case LOGIN: break;

        case PASSWD: break;
    }
}

int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
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
    while (1) {
        cmd_switch(sockfd);
    }

    exit(0);
}
