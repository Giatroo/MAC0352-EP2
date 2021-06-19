#ifndef CLIENT_FUNCTIONALITY_HPP
#define CLIENT_FUNCTIONALITY_HPP

#include <string>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include "util.hpp"
#include "packages.hpp"
#include "ttt-engine.hpp"

#include<sys/types.h>
#include<signal.h>

#define MAXLINE 4096
#define LISTENQ 1
// true se o jogador está em uma partida.
extern bool user_playing;

// true se o client está logado.
extern bool logged_in;

// pids de processos no client_functionality
extern pid_t *pid_jogo_latencia;
extern pid_t *pid_jogo_ui;
extern pid_t *pid_jogo_pai;

// Contém o nome do jogador com quem se está jogando contra caso se esteja
// jogando contra alguém ou nullptr caso contrário.
extern std::string opponent;

// Cria um novo usuário com nome name e senha password.
void create_user(std::string name, std::string password);

// Caso o usuário name exista e sua senha seja password, loja o usuário no
// servidor e retorna true. Caso contrário, retorna false.
bool user_login(std::string name, std::string password);

// Caso cur_password seja diferente da senha atual, retorna false. Caso seja
// igual, atualiza a senha para new_password.
bool change_password(std::string cur_password, std::string new_password);

// Desloga o usuário do servidor.
void user_logout();

// Mostra todos os usuários conectados ao servidor.
void show_all_connected_users();

// Mostra os n primeiros classificados ou todos se -1.
void show_classifications(int n);

// Retorna uma porta que está livre
int get_free_port();

// Convida user para jogar. Se ele não estiver online, não existir ou estiver em
// partida, retorna false automaticamente. Caso contrário, o servidor irá
// automaticamente lançar os dois jogadores em um lobby para decidir quem será o
// primeiro a jogar e quem será o X e começa a partida.
// O jogador que convidou se torna o host da sessão e deve chamar end_match()
// para enviar os resultados da partida para o servidor.
InviteOpponentAckPackage invite_opponent(int sockfd, int uifd);

// Prepara a resposta a um convite e retorna essa resposta
int answer_opponent(std::string recvline);

// Começa a partida após definir quem é o primeiro a jogar e quem será o X.
// Retorna a pontuação desse jogador no jogo ou -1 se houve um erro.
int start_match(bool tipo, bool moving_first, bool x, int port, char * ip);

// Envia um movimento na linha row e coluna col.
// row e col devem ser números de 0 a 2.
int send_move(bool x, int connfd);

// Pega o movimento recebido em recvline, atualiza na tabela e checa se
// houve um vencedor. Retorna 0 se o outro jogador desistiu e 1 caso contrário
int get_move(bool x, ustring recvline);

// O jogador desiste da partida (perdendo automaticamente).
void surrender(int connfd);

// Envia os resultados da partida para o servidor.
void end_match(int score1, int pipe);

// Ao receber um pingreq, essa função é chamada para enviar um pingback.
void pingback(int pipe);

// Envia um ping para o outro cliente.
void get_ping(int connfd);

// Encerra as operações do cliente (pode ser chamada apenas depois de logout).
void quit(int ui);

#endif /* ifndef CLIENT_FUNCTIONALITY_HPP */
