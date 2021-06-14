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

InviteOpponentAckPackage invite_opponent(int sockfd, int uifd) {
	int cliente;
	unsigned char sndline[MAXLINE + 1], recvline[MAXLINE + 1];
   	scanf("%d", &cliente);
   	InviteOpponentPackage p(cliente);
   	int n = p.header_to_string(sndline);

   	if(write(sockfd, sndline, n) < 0){
		printf("Erro ao direcionar à saída :(\n");
		exit (11);
	}

	InviteOpponentAckPackage pa(0);
	if((n = read(uifd, recvline, MAXLINE)) > 0){
		if((int)recvline[3]%2){
		    printf("Usuário aceitou o jogo!\n");
		    pa.string_to_header(recvline);
		}
		else
			printf("Usuário recusou o jogo!\n");
	}
	return pa;
}

int answer_opponent(ustring recvline){
	if(recvline[0] == '1') {
		int ret = 1, resp;
		printf("Deseja começar?(Digite 1 se sim e 0 se não)\n");
		scanf("%d", &resp);
		ret |= (resp << 1);
		printf("Deseja ser o X?(Digite 1 se sim e 0 se não)\n");
		scanf("%d", &resp);
		ret |= (resp << 2);
		// Faz outras perguntas
		return ret;
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
	
	pid_t childpid;
    pid_t * pid_latencia = (pid_t *)global_malloc(sizeof(pid_t));
    pid_t * pid_ui = (pid_t *)global_malloc(sizeof(pid_t));
    pid_t * pid_pai = (pid_t *)global_malloc(sizeof(pid_t));

    *pid_pai = getpid();

    int * trava_shell = (int *)global_malloc(sizeof(int));

    /*
		TODO SEMAFORIZAR?
    */
    double * delay = (double *)global_malloc(3*sizeof(double));
	int * delay_ind = (int *) global_malloc(sizeof(int));
	*delay_ind = 0;    

	int delay_fds[2];
   	if(pipe(delay_fds)){
   		fprintf(stderr,"Erro ao criar pipe\n");
   		exit (1);
   	}

   	Table * t = (Table *)global_malloc(sizeof(Table));
   	(*t).build();
	(*t).print();
		
    if((childpid = fork()) == 0){
        *pid_latencia = getpid();
                
        // Latencia
        while(true){
        	double val;
            if((val = get_ping(delay_fds[0], connfd)) == 0){
                fprintf(stderr, "Cliente Morreu :(\n");
                break;
            }
            else{
            	delay[*delay_ind] = 1000*val;
            	*delay_ind = (*delay_ind + 1)%3;
            }
            sleep(1);
        }

		quit(pid_pai, pid_ui, pid_latencia,\
			trava_shell, t, connfd);
        return 0;
    }
	else if((childpid = fork()) == 0){
   		*pid_ui = getpid();
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
				int acabou;
				if((acabou = send_move(t, x, connfd)) > -1){
					quit(pid_pai, pid_latencia, pid_ui,\
					trava_shell, t, connfd);
					return acabou;
				}
				else if(acabou == -1){
					*trava_shell = 1;
					continue;
				}
			}
			else if(strcmp((char *)recvline, "end") == 0){
				surrender(connfd);
				quit(pid_pai, pid_latencia, pid_ui,\
					trava_shell, t, connfd);
				return 0;
			}
			else if(strcmp((char *)recvline, "delay") == 0){
				int i = *delay_ind;
				printf("%lf ms\n%lf ms\n%lf ms\n", delay[i], delay[(i + 2)%3],\
					delay[(i + 1)%3]);
			}
			else{
				printf("Comando inválido!\n");
			}

			fprintf(stdout, "JogoDaVelha> ");
		}
		quit(pid_pai, pid_latencia, pid_ui, trava_shell, t, connfd);
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
				int acabou;
				if((acabou = get_move(t, x, recvline)) != -1){
					quit(pid_ui, pid_latencia, pid_pai, trava_shell,\
						t, connfd);
					return acabou;
				}
				*trava_shell = 0;
			}
		}

		quit(pid_ui, pid_latencia, pid_pai, trava_shell, t, connfd);
		return 0;
    }
}

int send_move(Table * t, bool x, int connfd) {	
	int r, c;
	scanf("%d %d", &r, &c);
	if((*t).update(r, c, x) == 0){
		fprintf(stdout, "Posição ocupada! Faça um outro movimento\n");
		return -2;
	}
	
	SendMovePackage p(r, c);
	unsigned char sndline[MAXLINE + 1];
	int n = p.header_to_string(sndline);

	if(write(connfd, sndline, n) < 0){
		printf("Erro ao enviar pacote :(\n");
		exit (11);
	}
	printf("Você jogou na casa (%d, %d)\n", r, c);
	(*t).print();
	fflush(stdout);

	if((*t).winner() == 1){
		printf("Você ganhou!!\n");
		return 2;
	}
	else if((*t).winner() == 2){
		printf("Empatou!!\n");
		return 1;
	}
	return -1;
}

int get_move(Table * t, bool x, ustring recvline){
	SendMovePackage p;
	p.string_to_header(recvline);

	if(p.r == 0) {
		printf("O outro jogador desistiu. Você ganhou!!\n");
		return 2;
	}

	printf("Outro jogador jogou na casa (%d, %d).\n", p.r, p.c);
	if((*t).update(p.r, p.c, !x) == 0){
		printf("Erro no jogo da velha!\n");
		exit (12);
	}
	(*t).print();

	if((*t).winner() == 1){
		printf("Outro jogador ganhou!!\n");
		return 0;
	}
	else if((*t).winner() == 2){
		printf("Empatou!!\n");
		return 2;
	}

	fprintf(stdout, "JogoDaVelha> ");
	fflush(stdout);
	return -1;
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

void pingback(int fd) {
	PingBackPackage p;
	unsigned char sndline[MAXLINE + 1];

	ssize_t n = p.header_to_string(sndline);
	if(write(fd, sndline, n) < 0){
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

void quit(pid_t * p1, pid_t * p2, pid_t * p3, int * trava, Table * t, int connfd) {
	kill(*p1, SIGTERM);
	kill(*p2, SIGTERM);
	global_free(p1, sizeof(pid_t)), global_free(p2,sizeof(pid_t));
	global_free(p3,sizeof(pid_t)), global_free(trava, sizeof(int));
	global_free(t, sizeof(Table));	
	close(connfd);
}

#endif /* ifndef CLIENT_FUNCTIONALITY_CPP */
