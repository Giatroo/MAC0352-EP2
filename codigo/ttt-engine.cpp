#ifndef TTT_ENGINE_CPP
#define TTT_ENGINE_CPP

#include "ttt-engine.hpp"

Table::Table(){
	pecas = 0;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			t[i][j] = ' ';
		}
	}
}
void Table::build(){
	pecas = 0;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			t[i][j] = ' ';
		}
	}
}

int Table::update(int r, int c, bool p){
	r--, c--;
	if(t[r][c] != ' ' || r > 2 || c > 2 || r < 0 || c < 0)
		return 0;

	char ox;

	if(p) ox = 'X';
	else ox = 'O';

	t[r][c] = ox;
	pecas++;
	return 1;
}

int Table::winner(){
	for(int r = 0; r < 3; r++){	
		if(t[r][0] == t[r][1] && t[r][1] == t[r][2] && t[r][0] != ' ')
			return 1;
	}
	for(int c = 0; c < 3; c++){	
		if(t[0][c] == t[1][c] && t[1][c] == t[2][c] && t[0][c] != ' ')
			return 1;
	}
	if(t[0][0] == t[1][1] && t[1][1] == t[2][2] && t[1][1] != ' ')
		return 1;
	if(t[0][2] == t[1][1] && t[1][1] == t[2][0] && t[1][1] != ' ')
		return 1;

	if(pecas == 9)
		return 2;

	return 0;
}

void Table::print(){
	fprintf(stdout, "-------------\n");
	for(int i = 0; i < 3; i++){
		fprintf(stdout, "| %c | %c | %c |\n", t[i][0], t[i][1], t[i][2]);
		fprintf(stdout, "-------------\n");
	}
}


#endif /* ifndef TTT_ENGINE_CPP */