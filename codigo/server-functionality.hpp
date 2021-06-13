#ifndef SERVER_FUNCTIONALITY_HPP
#define SERVER_FUNCTIONALITY_HPP

#include "server-io.hpp"
#include <string>
#include "packages.hpp"
#include <unistd.h>

// Cria um novo usuário com nome name e senha password
user *create_user(std::string name, std::string password);

// Caso o usuário name exista e sua senha seja password, loja o usuário no
// servidor e retorna true. Caso contrário, retorna false.
bool user_login(std::string name, std::string password);

// Caso cur_password seja diferente da senha atual, retorna false. Caso seja
// igual, atualiza a senha para new_password.
bool change_password(std::string cur_password, std::string new_password);

// Desloga o usuário do servidor
void user_logout();

// Mostra todos os usuários conectados ao servidor
void show_all_connected_users();

// Mostra os n primeiros classificados ou todos se -1
void show_classifications(int n);

// Convida client2 para jogar. Se ele não estiver online, não existir ou
// estiver em partida, retorna false automaticamente. Caso contrário, envia um
// prompt para o client2 perguntando se ele quer jogar ou não.
void invite_opponent(ustring recvline, int * ci, int invitor, int pipe);

// Pega a resposta do cliente convidado, processa ela e comunica o outro processo
// do servidor dessa resposta
void process_invitation_ack(ustring recvline, int * clients_invitation,\
	int * port, int invited);

// Começa a partida entre client1 (quem convidou) e client2. Deve ser chamada
// depois que invite_opponent(client2) retornou true.
void start_match(std::string client1, std::string client2);

// Termina a partida entre client1 e client2, cujo score optido pelo client1 é
// score1. Essa função deve ser chamada pelo client1 (que foi quem fez o
// convite para a partida) e atualiza os scores de ambos os jogadores.
void end_match(std::string client1, std::string client2, int score1);

// Pinga o usuário solicitando um pigback para checar se o usuário ainda está
// conectado ou não. Se sim, retorna true, se não, retorna false.
int pingreq(int pipe, int * heartbeat_resp);

// Checa se houve alguma atualização em relação a convites
bool new_update_client_invitation(int client_invitation);

// Se houve atualização nos convites, processa esse convite
void process_client_invitation(int * clients_invitation, int ind, int pipe);

// Checa se ele é o convidador ou o convidado
bool is_invited(int client_invitation);

// Envia o pacote de convite para o convidado
void send_invitation_package(int invitor, int pipe);

// Processa o convite se é o convidado, ou seja, envia
void send_invitation_ack_package(int resp, char * ip, int port, int pipe);

#endif /* ifndef SERVER_FUNCTIONALITY_HPP */
