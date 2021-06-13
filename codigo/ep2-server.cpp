#define _GNU_SOURCE
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

#include<sys/types.h>
#include<signal.h>

#include "util.hpp"
#include "server-functionality.hpp"

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pid_t childpid;
    unsigned char recvline[MAXLINE + 1], sndline[MAXLINE + 1];
    ssize_t n;

    /* TODO: Enviar uma mensagem caso os argumentos estejam errados <03-06-21, Paiolla> */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <Porta>\n", argv[0]);
        fprintf(stderr, "Vai rodar um servidor MQTT na porta <Porta> TCP\n");
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

    /*
        HARADA - VARIÁVEIS
    */
    const int MAX_CLIENTS = 10;
    int * pid_clients = (int *)global_malloc( MAX_CLIENTS * sizeof(int));
    int * port_clients = (int *)global_malloc( MAX_CLIENTS * sizeof(int));
    char ** ip_clients = (char **)global_malloc( MAX_CLIENTS * sizeof(char *));
    int * clients_num = (int *)global_malloc(sizeof(int));
    int * clients_invitation = (int *)global_malloc(100 * sizeof(int));
    /*
        FIM - HARADA - VARIÁVEIS
    */
    *clients_num = 1    ;


    printf("[Servidor no ar. Aguardando conexões na porta %s]\n", argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t clen;

        if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &clen)) == -1) {
            perror("accept :(\n");
            exit(5);
        }

        if ((childpid = fork()) == 0) {
            /**** PROCESSO FILHO ****/
            fprintf(stdout, "[Uma conexão aberta (PID = %d)]\n", getpid());
            close(listenfd);
            printf("IP address is: %s\n", inet_ntoa(client_addr.sin_addr));
            printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
            
            /*
                TODO : semaforizar isso
            */
            int ind = *clients_num;
            pid_clients[ind] = getpid();
            ip_clients[ind] = inet_ntoa(client_addr.sin_addr);
            port_clients[ind] = (int) ntohs(client_addr.sin_port);
            clients_invitation[ind] = 0;
            *clients_num += 1;

            int pipefds[2];
            if(pipe(pipefds)){
                fprintf(stderr,"Erro ao criar pipe\n");
                exit (1);
            }

            pid_t childpid_pai = getpid();
            pid_t * childpid_saida = (pid_t *)global_malloc(sizeof(pid_t));
            pid_t * childpid_heartbeat = (pid_t *)global_malloc(sizeof(pid_t));
            pid_t * childpid_invitation = (pid_t *)global_malloc(sizeof(pid_t));
            int * heartbeat_resp = (int *)global_malloc(sizeof(int));

            if ((childpid = fork()) == 0){
                *childpid_saida = getpid();
                // Saída
                while ((n = read(pipefds[0], recvline, MAXLINE)) > 0){
                	if(write(connfd, recvline, n) < 0){
                        printf("ERRO AO ENVIAR PACOTE :(\n");
                        exit (11);
                    }
                }
            }
            else if ((childpid = fork()) == 0){
                *childpid_heartbeat = getpid();
                
                // Heartbeats
                while(true){
                    if(pingreq(pipefds[1], heartbeat_resp) == 0){
                        fprintf(stderr, "Cliente Morreu :(\n");
                        close(connfd);
                        kill(*childpid_saida, SIGTERM);
                        kill(*childpid_invitation, SIGTERM);
                        kill(childpid_pai, SIGTERM);
                        break;
                    }
                }
            }
            else if ((childpid = fork()) == 0){
                *childpid_invitation = getpid();
                
                // Invitation
                while(true){
                    /*
                        TODO: Semaforizar?
                    */
                    if(new_update_client_invitation(clients_invitation[ind])){
                    	if(is_invited(clients_invitation[ind])){
                    		int invitor = clients_invitation[ind]/(1 << 5);
                    		send_invitation_package(invitor, pipefds[1]);
                    		clients_invitation[ind] ^= (1 << 3);
                    	}
                    	else{
                    		int invitor = ind, invited = clients_invitation[ind]/(1 << 5);
                    		int resp = clients_invitation[invitor]%(1 << 3);
                    		send_invitation_ack_package(resp, ip_clients[invited],\
                    			port_clients[invited], pipefds[1]);

                    		if(resp == 0) 
					            clients_invitation[ind] = 0;
					        else 
					        	clients_invitation[ind] ^= (1 << 3);
                    	}
                    }
                    sleep(1);
                }
            }
            else{
                // Entrada
                while ((n = read(connfd, recvline, MAXLINE)) > 0) {
                    if(recvline[0] == PINGBACK_PACKAGE)
                        *heartbeat_resp = 1;
                    else if(recvline[0] == INVITE_OPPONENT_PACKAGE){
                        /*
                            TODO: Semaforizar?
                        */
                        invite_opponent(recvline, clients_invitation, ind,\
                        	pipefds[1]);
                    }
                    else if(recvline[0] == INVITE_OPPONENT_ACK_PACKAGE){
                        process_invitation_ack(recvline, clients_invitation,\
                        	port_clients, ind);
                    }
                    else if(recvline[0] == END_MATCH_PACKAGE){
                    	printf("Jogador %d com pontuação %d\n", ind, (int)recvline[1]);
                    }
                }
                kill(*childpid_saida, SIGTERM);
                kill(*childpid_heartbeat, SIGTERM);
                kill(*childpid_invitation, SIGTERM);
            }       

            close(pipefds[0]);
            close(pipefds[1]);

            printf("[Uma conexão fechada (PID = %d)] %ld\n", getpid(), n);
            exit(0);

        } else
            /**** PROCESSO PAI ****/
            close(connfd);
    }

    global_free(pid_clients, MAX_CLIENTS);
    global_free(port_clients, MAX_CLIENTS);

    exit(0);
}
