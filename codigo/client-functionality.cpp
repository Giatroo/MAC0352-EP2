#ifndef CLIENT_FUNCTIONALITY_CPP
#define CLIENT_FUNCTIONALITY_CPP

#include "client-functionality.hpp"
#include <string>

std::string opponent;

void create_user(std::string name, std::string password) { }

bool user_login(std::string name, std::string password) { }

bool change_password(std::string cur_password, std::string new_password) { }

void user_logout() { }

void show_all_connected_users() { }

void show_classifications(int n) { }

InviteOpponentAckPackage invite_opponent(int sockfd, int uifd, \
	int * wait_invitation) {
	int cliente;
	unsigned char sndline[MAXLINE + 1], recvline[MAXLINE + 1];
   	scanf("%d", &cliente);
   	InviteOpponentPackage p(cliente);
   	int n = p.header_to_string(sndline);
   	*wait_invitation = -1;

   	if(write(sockfd, sndline, n) < 0){
		printf("Erro ao direcionar à saída :(\n");
		exit (11);
	}

	InviteOpponentAckPackage pa(0);
	if((n = read(uifd, recvline, MAXLINE)) > 0){
		if((int)recvline[3] == 1){
		    printf("Usuário aceitou o jogo!\n");
		    pa.string_to_header(recvline);
						
			int pont = start_match(false, true, false, pa.port, pa.ip);
			end_match(pont, sockfd);
		}
		else
			printf("Usuário recusou o jogo!\n");
	}
	return pa;
}

int answer_opponent(ustring recvline){
	if(recvline[0] == '1') {
		// Faz outras perguntas
		return 1;
	}
	else return 0;
}

int start_match(bool tipo, bool moving_first, bool x, int port, char * ip) { 
	unsigned char recvline[MAXLINE + 1];
    int connfd, n;
	struct sockaddr_in servaddr, client_addr;
    socklen_t clen;
	
	fprintf(stdout, "Game started!\n");
	if(tipo){
		int listenfd;
		if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	        perror("socket :(\n");
	        exit(2);
	    }

	    bzero(&servaddr, sizeof(servaddr));
	    servaddr.sin_family = AF_INET;
	    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	    servaddr.sin_port = htons(port);

	    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
	    	printf("%d\n", port);
	        perror("bind :(\n");
	        exit(3);
	    }

	    if (listen(listenfd, LISTENQ) == -1) {
	        perror("listen :(\n");
	        exit(4);
	    }

	    if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &clen)) == -1) {
            perror("accept :(\n");
            exit(5);
        }
        close(listenfd);
	}
	else{
		bzero(&servaddr, sizeof(servaddr));
	   	servaddr.sin_family = AF_INET;
	   	servaddr.sin_port = htons(port);

	   	if ( (connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        	fprintf(stderr,"socket error :( \n");

	   	if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
	      	fprintf(stderr,"inet_pton error for %s :(\n", ip);

	    if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	      	fprintf(stderr,"connect error :(\n");
	}

	fprintf(stdout, "JogoDaVelha> ");
	fprintf(stdout, "connected!\n");
	
	pid_t childpid_pai = getpid(), childpid;
    pid_t * childpid_heartbeat = (pid_t *)global_malloc(sizeof(pid_t));
    pid_t * childpid_ui = (pid_t *)global_malloc(sizeof(pid_t));

    int * trava_shell = (int *)global_malloc(sizeof(int));

	int delay_fds[2];
   	if(pipe(delay_fds)){
   		fprintf(stderr,"Erro ao criar pipe\n");
   		exit (1);
   	}

   	Table * t = (Table *)global_malloc(sizeof(Table));
   	(*t).build();
	(*t).print();
		
    if((childpid = fork()) == 0){
        *childpid_heartbeat = getpid();
                
        // Heartbeats
        while(true){
        	double val;
            if((val = get_ping(delay_fds[0], connfd)) == 0){
                fprintf(stderr, "Cliente Morreu :(\n");
                close(connfd);
                kill(childpid_pai, SIGTERM);
                kill(*childpid_ui, SIGTERM);
            	break;
            }
            
            sleep(5);
        }
        return 0;
    }
	else if((childpid = fork()) == 0){
   		*childpid_ui = getpid();
   		// UI
   		if(moving_first){
    		*trava_shell = 0;
			printf("JogoDaVelha> ");
    	}
    	else{
    		*trava_shell = 1;
    	}

		while(scanf("%s", recvline)){
			if(*trava_shell) continue;
			if(strcmp((char *)recvline, "send") == 0){
				int r, c;
				scanf("%d %d", &r, &c);
				(*t).print();
				if((*t).update(r, c, x) == 0){
					printf("Posição ocupada! Faça um outro movimento\n");
						fprintf(stdout, "JogoDaVelha> ");
					continue;
				}
				send_move(r, c, connfd);
				(*t).print();
				if((*t).winner() == 1){
			    	printf("Você ganhou!!\n");
				   	close(connfd);
					kill(childpid_pai, SIGTERM);
					kill(*childpid_heartbeat, SIGTERM);
				   	return 2;
				}
				else if((*t).winner() == 2){
				   	printf("Empatou!!\n");
				   	close(connfd);
					kill(childpid_pai, SIGTERM);
					kill(*childpid_heartbeat, SIGTERM);
				   	return 1;
				}
				*trava_shell = 1;
				continue;
			}
			else if(strcmp((char *)recvline, "end") == 0){
				surrender(connfd);
				kill(childpid_pai, SIGTERM);
				kill(*childpid_heartbeat, SIGTERM);
				return 0;
			}
			else
				printf("Comando inválido!\n");
			fprintf(stdout, "JogoDaVelha> ");
		}
		kill(childpid_pai, SIGTERM);
		kill(*childpid_heartbeat, SIGTERM);
		return 0;
    }
    else{
       	// Entrada 	
		while ( (n = read(connfd, recvline, MAXLINE)) > 0) {
			if((int)recvline[0] == PINGREQ_PACKAGE)
				pingback(connfd);
			else if((int)recvline[0] == PINGBACK_PACKAGE){
				if(write(delay_fds[1], recvline, n) < 0){
					printf("Deu ruim em write delay_fds :(\n");
					exit (11);
				}
			}
			else if((int)recvline[0] == SEND_MOVE_PACKAGE){
				if(get_move(t, x, recvline) == 0){
					printf("O outro jogador desistiu. Você ganhou!!\n");
					kill(*childpid_ui, SIGTERM);
					kill(*childpid_heartbeat, SIGTERM);
					close(connfd);
					return 2;
				}
				(*t).print();
				if((*t).winner() == 1){
					printf("Outro jogador ganhou!!\n");
					close(connfd);
					kill(*childpid_ui, SIGTERM);
					kill(*childpid_heartbeat, SIGTERM);
					return 0;
				}
				else if((*t).winner() == 2){
					printf("Empatou!!\n");
					close(connfd);
					kill(*childpid_ui, SIGTERM);
					kill(*childpid_heartbeat, SIGTERM);
					return 1;
				}
				*trava_shell = 0;
				printf("JogoDaVelha> ");
			}
		}
		return 0;
    }
}

void send_move(int r, int c, int connfd) {	
	SendMovePackage p(r, c);
	unsigned char sndline[MAXLINE + 1];
	int n = p.header_to_string(sndline);

	if(write(connfd, sndline, n) < 0){
		printf("Erro ao enviar pacote :(\n");
		exit (11);
	}
	printf("Você jogou na casa (%d, %d)\n", r, c);
}

int get_move(Table * t, bool x, ustring recvline){
	SendMovePackage p;
	p.string_to_header(recvline);

	if(p.r == 0) return 0;

	printf("Outro jogador jogou na casa (%d, %d).\n", p.r, p.c);
	if((*t).update(p.r, p.c, !x) == 0){
		printf("Erro no jogo da velha!\n");
		exit (12);
	}
	return 1;
}

void surrender(int connfd) {
	send_move(0, 0, connfd);
	printf("Você desistiu!!\n");
	close(connfd);
}

void end_match(int score1, int pipe) {
	/*
		TODO: implementar a tolerância a erro de queda do servidor
	*/
	EndMatchPackage p(score1);
	unsigned char sndline[MAXLINE + 1];
	ssize_t n = p.header_to_string(sndline);
	if(write(pipe, sndline, n) < 0){
		printf("Erro ao direcionar à saída :(\n");
		exit (11);
	}
}

void pingback(int pipe) {
	PingBackPackage p;
	unsigned char sndline[MAXLINE + 1];

	ssize_t n = p.header_to_string(sndline);
	if(write(pipe, sndline, n) < 0){
	    printf("Erro ao direcionar à saída :(\n");
	    exit (11);
	}
}

double get_ping(int pipe_to_read, int connfd) {
	PingReqPackage p;
	unsigned char sndline[MAXLINE + 1], recvline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);
   	if(write(connfd, sndline, n) < 0)
	    return 0;
	
	struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC, &start);
    if((n = read(pipe_to_read, recvline, MAXLINE)) > 0){
    	clock_gettime(CLOCK_MONOTONIC, &finish);

		elapsed = (finish.tv_sec - start.tv_sec);
		elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    	return elapsed;
    }
    else
    	return 0;
}

void quit() { }

#endif /* ifndef CLIENT_FUNCTIONALITY_CPP */
