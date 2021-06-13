#ifndef SERVER_FUNCTIONALITY_CPP
#define SERVER_FUNCTIONALITY_CPP

#include "server-functionality.hpp"

user *create_user(std::string name, std::string password) { }

bool user_login(std::string name, std::string password) { }

bool change_password(std::string cur_password, std::string new_password) { }

void user_logout() { }

void show_all_connected_users() { }

void show_classifications(int n) { }

void invite_opponent(ustring recvline, int * ci, int invitor, int pipe) {
	int invited = (int)recvline[3];
    if(ci[invited] == 0){
        ci[invitor] = (1 << 5)*invited;
        ci[invited] = (1 << 5)*invitor + (1 << 4) + (1 << 3);
    }
    else{
    	// Jogador está ocupado e o servidor recusa
    	unsigned char sndline[MAXLINE + 1];
		InviteOpponentAckPackage p(0);
        ssize_t n = p.header_to_string(sndline);
        if(write(pipe, sndline, n) < 0){
            printf("Reject :(\n");
            exit (11);
       	}
    }
}

void process_invitation_ack(ustring recvline, int * clients_invitation,\
	int * port, int invited
	){
	int invitor = clients_invitation[invited]/32;
	InviteOpponentAckPackage p(0);
	p.string_to_header(recvline);

    if(p.resp == 0)
        clients_invitation[invited] = 0;
    else 
    	port[invited] = p.port;

    clients_invitation[invitor] += p.resp;
   	clients_invitation[invitor] |= (1 << 3);
 	// Comunica o processo do invitor de que teve resposta 
}

void start_match(std::string client1, std::string client2) { }

void end_match(std::string client1, std::string client2, int score1) { }

int pingreq(int pipe, int * heartbeat_resp) {
	PingReqPackage p;
	unsigned char sndline[MAXLINE + 1];
    ssize_t n = p.header_to_string(sndline);
   	if(write(pipe, sndline, n) < 0){
        printf("Reject :(\n");
        exit (11);
    }
    * heartbeat_resp = 0;
    sleep(10);

    return * heartbeat_resp;
}

bool new_update_client_invitation(int client_invitation){
	return (((1 << 3) & client_invitation) != 0);
}

void send_invitation_package(int invitor, int pipe){
	unsigned char sndline[MAXLINE + 1];
	InviteOpponentPackage p(invitor);
    ssize_t n = p.header_to_string(sndline);
    if(write(pipe, sndline, n) < 0){
        printf("Reject :(\n");
        exit (11);
    }
}

void send_invitation_ack_package(int resp, char * ip, int port, int pipe){
	unsigned char sndline[MAXLINE + 1];
	InviteOpponentAckPackage p(resp);
    if(resp != 0){
        p.ip = ip;
        p.port = port;
    }

    ssize_t n = p.header_to_string(sndline);
    if(write(pipe, sndline, n) < 0){
        printf("Reject :(\n");
        exit (11);
    }
}

bool is_invited(int client_invitation){
	return (((1 << 4) & client_invitation) != 0);
}

#endif /* ifndef SERVER_FUNCTIONALITY_CPP */
