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

pid_t father_pid;

void exitHandler(int sig) {
    if (getpid() == father_pid) {
        std::cerr << "Exiting" << std::endl;
        log_struct_t log_struct;
        write_log_line(SERVER_FINISHED, log_struct);
    }
    exit(0);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t childpid;

    signal(SIGINT, exitHandler);
    father_pid = getpid();

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

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n", argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    log_struct_t log_struct;
    write_log_line(SERVER_STARTED, log_struct);
    memset(&client_addr, 0, sizeof(client_addr));

    while (1) {
        struct sockaddr_in client_addr_p;
        socklen_t clen;

        if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr_p,
                             &clen)) == -1) {
            perror("accept :(\n");
            exit(5);
        }

        client_addr = client_addr_p;

        // temp
        if (DEBUG) {
            cout << "ip = " << client_addr.sin_addr.s_addr << endl;
            cout << "port = " << client_addr.sin_port << endl;
            debug(connfd);
            debug(clen);
        }

        if ((childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            fprintf(stdout, "[Uma conexão aberta (PID = %d)]\n", getpid());
            close(listenfd);

            if (DEBUG) {
                printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));
                printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
            }

            log_struct_t log_struct;
            log_struct.client_ip = inet_ntoa(client_addr.sin_addr);
            write_log_line(CLIENT_CONNECTED, log_struct);

            current_user = (int *) global_malloc(sizeof(int));
            *current_user = -1;

            if (DEBUG) {
                fprintf(stdout, "Usuários:\n");
                for (int i = 0; i < total_users[0]; i++) {
                    cout << *users[i] << endl;
                }
            }

            int pipefds[2];
            if (pipe(pipefds)) {
                fprintf(stderr, "Erro ao criar pipe\n");
                exit(1);
            }

            pthread_t heartbeat_t, invitation_t, entrada_t;
            if (pthread_create(&heartbeat_t, NULL, heartbeat_handler_thread,
                               (void *) &connfd)) {
                printf("Erro ao criar thread heartbeat\n");
                exit(1);
            }
            if (pthread_create(&invitation_t, NULL, invitation_handler_thread,
                               (void *) &connfd)) {
                printf("Erro ao criar thread Invitation\n");
                exit(1);
            }
            if (pthread_create(&entrada_t, NULL, entrada_handler_thread,
                               (void *) &connfd)) {
                printf("Erro ao criar thread Entrada\n");
                exit(1);
            }

            if (pthread_join(heartbeat_t, NULL)) {
                printf("Erro ao dar join na thread heartbeat\n");
                exit(1);
            }
            if (pthread_join(invitation_t, NULL)) {
                printf("Erro ao dar join na thread invitation\n");
                exit(1);
            }
            if (pthread_join(entrada_t, NULL)) {
                printf("Erro ao dar join na thread entrada\n");
                exit(1);
            }

            exit(0);

        } else
            /**** PROCESSO PAI ****/
            close(connfd);
    }

    exit(0);
}
