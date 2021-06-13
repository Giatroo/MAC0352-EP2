#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include "packages.hpp"

#include "client-functionality.hpp"

int main(int argc, char **argv){
	int sockfd, n;
   	unsigned char recvline[MAXLINE + 1], sndline[MAXLINE + 1];
    struct sockaddr_in servaddr;
  	if (argc != 4)
      	fprintf(stderr,"usage: %s <IPaddress> <Port> <Port>\n",argv[0]);
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        fprintf(stderr,"socket error :( \n");

   	bzero(&servaddr, sizeof(servaddr));
   	servaddr.sin_family = AF_INET;
   	servaddr.sin_port = htons(atoi(argv[2]));

   	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
      	fprintf(stderr,"inet_pton error for %s :(\n", argv[1]);

   	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
      	fprintf(stderr,"connect error :(\n");

   	fprintf(stdout, "connected\n");
   	

   	int saidafds[2], uifds[2];
   	if(pipe(saidafds)){
   		fprintf(stderr,"Erro ao criar pipe\n");
   		exit (1);
   	}
   	if(pipe(uifds)){
   		fprintf(stderr,"Erro ao criar pipe\n");
   		exit (1);
   	}

   	pid_t childpid_pai = getpid(), childpid;
    pid_t * childpid_saida = (pid_t *)global_malloc(sizeof(pid_t));
    pid_t * childpid_ui = (pid_t *)global_malloc(sizeof(pid_t));
    int * wait_invitation = (int *)global_malloc(sizeof(int));
    *wait_invitation = 0;
    /*
		-1 se está esperando resposta do convidado e -2 é o convidado e está esperando
		o usuário
    */

   	if((childpid = fork()) == 0){
   		// Saída
   		*childpid_saida = getpid();
   		while ((n = read(saidafds[0], recvline, MAXLINE)) > 0){
   			
   			if(write(sockfd, recvline, n) < 0){
	            printf("Erro ao enviar pacote :(\n");
	            exit (11);
   			}
   		}
   	}
   	else if((childpid = fork()) == 0){
   		*childpid_ui = getpid();
   		// UI
   		while(scanf("%s", recvline)){
   			if(*wait_invitation == -1) continue;
   			if(*wait_invitation == -2){
				int resp = answer_opponent(recvline);

				InviteOpponentAckPackage p(resp);
				p.port = atoi(argv[3]);
				n = p.header_to_string(sndline);
				if(write(saidafds[1], sndline, n) < 0){
		            printf("Erro ao direcionar à saída :(\n");
		            exit (11);
		        }

				if(resp > 0){
					int pont = start_match(true, false, true, atoi(argv[3]), NULL);
					end_match(pont, saidafds[1]);
				}
				
				*wait_invitation = 0;
   			}
   			else if(strcmp((char *)recvline, "invite") == 0){
   				int cliente;
   				scanf("%d", &cliente);
   				InviteOpponentPackage p(cliente);
   				n = p.header_to_string(sndline);
   				*wait_invitation = -1;

	   			if(write(saidafds[1], sndline, n) < 0){
		            printf("Erro ao direcionar à saída :(\n");
		            exit (11);
		        }

		        if((n = read(uifds[0], recvline, MAXLINE)) > 0){
		        	if((int)recvline[3] == 1){
		      			printf("Usuário aceitou o jogo!\n");

		      			InviteOpponentAckPackage p(0);
		      			p.string_to_header(recvline);
						
						int pont = start_match(false, true, false, p.port, p.ip);
						printf("Pontuacao %d\n", pont);
						end_match(pont, saidafds[1]);
		      		}
		      		else
						printf("Usuário recusou o jogo!\n");
				}
				else
					printf("Erro de conexão :(\n");

		      	*wait_invitation = 0;
   			}
		}
   	}
   	else{
   		// Entrada
	   	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
	      	recvline[n] = 0;
	      			
	      	if(recvline[0] == PINGREQ_PACKAGE)
	      		pingback(saidafds[1]);
	      	else if(recvline[0] == INVITE_OPPONENT_PACKAGE){
	      		*wait_invitation = -2;
	      		printf("Usuário %d está te convidando para jogar um jogo!\n", (int)recvline[3]);
	      		printf("Aceita o convite?(Digite 1 se sim e 0 se não)\n");
	      	}
	      	else if(recvline[0] == INVITE_OPPONENT_ACK_PACKAGE){
	      		if(write(uifds[1], recvline, n) < 0){
		            printf("Erro ao direcionar à saída :(\n");
		            exit (11);
		        }
	      	}
	   	} 
	   		
	   	close(saidafds[0]), close(saidafds[1]), close(uifds[0]), close(uifds[1]);
	   	kill(*childpid_saida, SIGTERM), kill(*childpid_ui, SIGTERM);
   	}

   exit(0);
}