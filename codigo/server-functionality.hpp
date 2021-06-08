#ifndef SERVER_FUNCTIONALITY_HPP
#define SERVER_FUNCTIONALITY_HPP

#include <string>

#include "server-io.hpp"

extern user_t *current_user;

// Procura o usuário nos usuários existentes.
// Se ele existir, o retorna. Caso contrário, retorna nullptr.
user_t *find_user(std::string name);

// Cria um novo usuário com nome name e senha password
user_t *create_user(std::string name, std::string password);

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
bool invite_opponent(std::string client2);

// Começa a partida entre client1 (quem convidou) e client2. Deve ser chamada
// depois que invite_opponent(client2) retornou true.
void start_match(std::string client1, std::string client2);

// Termina a partida entre client1 e client2, cujo score optido pelo client1 é
// score1. Essa função deve ser chamada pelo client1 (que foi quem fez o
// convite para a partida) e atualiza os scores de ambos os jogadores.
void end_match(std::string client1, std::string client2, int score1);

// Pinga o usuário solicitando um pigback para checar se o usuário ainda está
// conectado ou não. Se sim, retorna true, se não, retorna false.
bool pingreq();

#endif /* ifndef SERVER_FUNCTIONALITY_HPP */
