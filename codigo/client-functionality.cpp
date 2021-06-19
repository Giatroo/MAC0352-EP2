#ifndef CLIENT_FUNCTIONALITY_CPP
#define CLIENT_FUNCTIONALITY_CPP

#include "client-functionality.hpp"

#include <iostream>
#include <string>

std::string opponent;
pid_t *pid_jogo_latencia;
pid_t *pid_jogo_ui;
pid_t *pid_jogo_pai;

void create_user(std::string name, std::string password) { }

bool user_login(std::string name, std::string password) { return false; }

bool change_password(std::string cur_password, std::string new_password) {
    return false;
}

void user_logout() { }

void show_all_connected_users() { }

void show_classifications(int n) { }

int get_free_port() {
    struct sockaddr_in sin;
    int sockefds, port = 8000;

    sockefds = socket(AF_INET, SOCK_STREAM, 0);
    if (sockefds == -1) return -1;

    sin.sin_addr.s_addr = 0;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;

    while (true) {
        port = rand() % 60000 + 5535;
        sin.sin_port = htons(port);
        if (bind(sockefds, (struct sockaddr *) &sin,
                 sizeof(struct sockaddr_in)) == -1) {
            if (errno == EADDRINUSE) printf("Port in use");
        } else {
            close(sockefds);
            return port;
        }
    }
}

InviteOpponentAckPackage invite_opponent(int sockfd, int uifd) {
    std::string client_name;
    unsigned char sndline[MAXLINE + 1], recvline[MAXLINE + 1];
    std::cin >> client_name;
    InviteOpponentPackage p(client_name);
    int n = p.header_to_string(sndline);

    print_in_hex(sndline, n);
    if (write(sockfd, sndline, n) < 0) {
        std::cout << "Erro ao direcionar à saída :(" << std::endl;
        exit(11);
    }

    InviteOpponentAckPackage pa(0);
    if ((n = read(uifd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if ((int) recvline[3] % 2) {
            std::cout << "Usuário aceitou o jogo!" << std::endl;
            pa.string_to_header(recvline);
        } else {
            std::cout << "Usuário recusou o jogo!" << std::endl;
        }
    }
    return pa;
}

int answer_opponent(std::string recvline) {
    if (recvline == "yes") {
        int ret = 1;
        std::string resp;
        std::cout << "Deseja comecar?" << std::endl;
        std::cin >> resp;
        if (resp == "yes") ret |= (1 << 1);
        std::cout << "Deseja ser o X?" << std::endl;
        std::cin >> resp;
        if (resp == "yes") ret |= (1 << 2);
        return ret;
    } else
        return 0;
}

int matchfd;
int trava_shell;
struct timespec start_delay;
double delay[3];
int delay_ind;
int delay_fds[2];

int * acabou;

Table t;
pthread_t ui_match_thread, entrada_match_thread, latency_match_thread;
    
void * match_latency(void * args){
    // Latencia
    while (true) {
        clock_gettime(CLOCK_MONOTONIC, &start_delay);
        get_ping(matchfd);
        sleep(3);
    }
    std::cout << "Algo deu ruim" << std::endl;
    quit(0);
}

void * match_ui(void * args){
    bool * aux_args = (bool *)args;
    bool moving_first = aux_args[0];
    bool x = aux_args[1];
    /*
        args é moving_first e x
    */
    if (moving_first) {
        trava_shell = 0;
        std::cout << "JogoDaVelha> " << std::flush;
    } else {
        trava_shell = 1;
    }

    std::string comando;
    while (std::cin >> comando) {
        if (trava_shell) continue;
        if (comando == "send") {
            int pont;
            if ((pont = send_move(x, matchfd)) > -1) {
                *acabou = pont;
                quit(1);
                return NULL;
            } else if (pont == -1) {
                trava_shell = 1;
                continue;
            }
        } else if (comando == "end") {
            surrender(matchfd);
            *acabou = 0;
            quit(1);
            return NULL;
        } else if (comando == "delay") {
            int i = delay_ind;
            std::cout << delay[(i + 2) % 3] << "ms" << std::endl;
            std::cout << delay[(i + 1) % 3] << "ms" << std::endl;
            std::cout << delay[i] << "ms" << std::endl;
        } else {
            std::cout << "Comando inválido" << std::endl;
        }
        std::cout << "JogoDaVelha> " << std::flush;
    }

    std::cout << "Algo deu ruim" << std::endl;
    *acabou = 0;
    quit(1);
}

void * match_entrada(void * args){
    bool * aux_args = (bool *)args;
    bool moving_first = aux_args[0];
    bool x = aux_args[1];
    ssize_t n;
    unsigned char recvline[MAXLINE + 1];
    
    while ((n = read(matchfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        // fprintf(stdout, "Recebido: ");
        // print_in_hex(recvline, n);

        if ((int) recvline[0] == PINGREQ_PACKAGE)
            pingback(matchfd);
        else if ((int) recvline[0] == PINGBACK_PACKAGE) {
            struct timespec finish;
            clock_gettime(CLOCK_MONOTONIC, &finish);
            double elapsed = (finish.tv_sec - (start_delay).tv_sec);
            elapsed += (finish.tv_nsec - (start_delay).tv_nsec) / 1000000000.0;

            delay[delay_ind] = 1000 * elapsed;
            delay_ind = (delay_ind + 1) % 3;
        } else if ((int) recvline[0] == SEND_MOVE_PACKAGE) {
            int pont;
            if ((pont = get_move(x, recvline)) != -1) {
                *acabou = pont;
                quit(0);
                return NULL;
            }
            trava_shell = 0;
        }
    }

    std::cout << "Algo deu ruim" << std::endl;
    quit(0);
}

int start_match(bool tipo, bool moving_first, bool x, int port, char *ip) {
    unsigned char recvline[MAXLINE + 1];
    int n;
    struct sockaddr_in servaddr, client_addr;
    socklen_t clen;
    acabou = (int *)global_malloc(sizeof(int));
    *acabou = -1;
    trava_shell = 0;
    delay_ind = 0;

    std::cout << "O jogo comecou!!" << std::endl;
    if (tipo) {
        int listenfd;
        if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket :(\n");
            exit(2);
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port = htons(port);

        if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) ==
            -1) {
            perror("bind :(\n");
            exit(3);
        }

        if (listen(listenfd, LISTENQ) == -1) {
            perror("listen :(\n");
            exit(4);
        }

        if ((matchfd = accept(listenfd, (struct sockaddr *) &client_addr,
                             &clen)) == -1) {
            perror("accept :(\n");
            exit(5);
        }
        close(listenfd);
    } else {
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);

        if ((matchfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr, "socket error :( \n");
            exit(5);
        }

        if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
            fprintf(stderr, "inet_pton error for %s :(\n", ip);
            exit(5);
        }

        if (connect(matchfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) <
            0) {
            fprintf(stderr, "connect error :(\n");
            exit(5);
        }
    }

    pid_t childpid;
    if((childpid = fork()) == 0){
        std::cout << "JogoDaVelha> connected" << std::endl;

        /*
            TODO SEMAFORIZAR?
        */
        if (pipe(delay_fds)) {
            fprintf(stderr, "Erro ao criar pipe\n");
            exit(1);
        }


        t.build();
        t.print();

        bool args[2] = {moving_first, x};

        if (pthread_create(&ui_match_thread, NULL, match_ui, &args)) {
            printf("Erro ao criar thread UI\n");
            exit(1);
        }
        if (pthread_create(&entrada_match_thread, NULL, match_entrada, &args)) {
            printf("Erro ao criar thread UI\n");
            exit(1);
        }
        if (pthread_create(&latency_match_thread, NULL, match_latency, NULL)) {
            printf("Erro ao criar thread UI\n");
            exit(1);
        }


        if (pthread_join(ui_match_thread, NULL)) {
            printf("Erro ao dar join na thread ui\n");
            exit(1);
        }
        if (pthread_join(entrada_match_thread, NULL)) {
            printf("Erro ao dar join na thread entrada\n");
            exit(1);
        }
        if (pthread_join(latency_match_thread, NULL)) {
            printf("Erro ao dar join na thread latency\n");
            exit(1);
        }
        close(matchfd);
    }
    else{
        while(*acabou == -1){
            sleep(1);
        }
    }
    int pont = *acabou;
    global_free(acabou, sizeof(int));

    return pont;
}

int send_move(bool x, int connfd) {
    int r, c;
    std::cin >> r >> c;
    if ((t).update(r, c, x) == 0) {
        std::cout << "Posição ocupada! Faça um outro movimento" << std::endl;
        return -2;
    }

    SendMovePackage p(r, c);
    unsigned char sndline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);

    if (write(connfd, sndline, n) < 0) {
        std::cout << "Erro ao enviar pacote" << std::endl;
        exit(11);
    }
    std::cout << "Você jogou na casa (" << r << ", " << c << ")" << std::endl;
    (t).print();

    if ((t).winner() == 1) {
        std::cout << "Você ganhou!!" << std::endl;
        return 2;
    } else if ((t).winner() == 2) {
        std::cout << "Empatou!!" << std::endl;
        return 1;
    }
    return -1;
}

int get_move(bool x, ustring recvline) {
    SendMovePackage p;
    p.string_to_header(recvline);

    debug(p.r);
    debug(p.c);

    if (p.r == 0) {
        std::cout << "O outro jogador desistiu. Você ganhou!!" << std::endl;
        return 2;
    }
    std::cout << "Outro jogador jogou na casa (" << p.r << ", " << p.c << ")"
              << std::endl;

    if (t.update(p.r, p.c, !x) == 0) {
        std::cout << "Erro no jogo da velha!" << std::endl;
        exit(12);
    }
    t.print();

    if (t.winner() == 1) {
        std::cout << "Outro jogador ganhou!!" << std::endl;
        return 0;
    } else if (t.winner() == 2) {
        std::cout << "Empatou!!" << std::endl;
        return 2;
    }

    std::cout << "JogoDaVelha> " << std::flush;
    return -1;
}

void surrender(int connfd) {
    SendMovePackage p(0, 0);
    unsigned char sndline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);

    if (write(connfd, sndline, n) < 0) {
        std::cout << "Erro ao enviar pacote :(" << std::endl;
        exit(11);
    }
    std::cout << "Você desistiu!" << std::endl;
    close(connfd);
}

void end_match(int score1, int pipe) {
    /*
        TODO: implementar a tolerância a erro de queda do servidor
    */
    EndMatchPackage p(score1);
    unsigned char sndline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);
    if (write(pipe, sndline, n) < 0) {
        std::cout << "Erro ao direcionar à saída :(" << std::endl;
        exit(11);
    }
}

void pingback(int fd) {
    PingBackPackage p;
    unsigned char sndline[MAXLINE + 1];

    ssize_t n = p.header_to_string(sndline);
    if (write(fd, sndline, n) < 0) {
        std::cout << "Erro ao direcionar à saída :(" << std::endl;
        exit(11);
    }
}

void get_ping(int connfd) {
    PingReqPackage p;
    unsigned char sndline[MAXLINE + 1], recvline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);
    if (write(connfd, sndline, n) < 0) {
        std::cout << "Erro no get_ping" << std::endl;
        return;
    }
}

void quit(int ui) {
    exit(0);
}

#endif /* ifndef CLIENT_FUNCTIONALITY_CPP */
