#ifndef TTT_ENGINE_HPP
#define TTT_ENGINE_HPP

#include <stdio.h>

class Table{
private:
	char t[3][3];
	int pecas;
public:

	Table();
	// Retorna 0 se não foi possível colocar a peça nesse local
	// Retorna 1 caso contrário
	int update(int r, int c, bool p);

	// Inicalizar as variaveis da classe
	void build();
	// Retorna 1 se houve um vencedor
	// Retorna 2 se houve empate
	int winner();
	// Imprime o tabuleiro
	void print();
};

#endif /* ifndef TTT_ENGINE_HPP */
