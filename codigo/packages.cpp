#ifndef PACKAGES_CPP
#define PACKAGES_CPP

#include "packages.hpp"
#include <string.h>

ssize_t PingReqPackage::header_to_string(ustring line){
	line[0] = this->fixed_header.header_type;
    line[1] = line[2] = 0;
    return 3;
}
PingReqPackage::PingReqPackage(){
	fixed_header.header_type = (byte)PINGREQ_PACKAGE;
}

ssize_t PingBackPackage::header_to_string(ustring line){
	line[0] = this->fixed_header.header_type;
    line[1] = line[2] = 0;
    return 3;
}
PingBackPackage::PingBackPackage(){
	fixed_header.header_type = (byte)PINGBACK_PACKAGE;
}

ssize_t InviteOpponentPackage::header_to_string(ustring line){
	line[0] = this->fixed_header.header_type;
    line[1] = 0;
	line[2] = 1;
	line[3] = (byte)cliente;
    return 4;
}
InviteOpponentPackage::InviteOpponentPackage(int c){
	cliente = c;
	fixed_header.header_type = (byte)INVITE_OPPONENT_PACKAGE;
}

ssize_t InviteOpponentAckPackage::header_to_string(ustring line){
	line[0] = this->fixed_header.header_type;
	line[3] = (byte)resp;
	if(resp == 0){		
	    line[1] = 0;
		line[2] = 1;
		return 4;
	}
	else{
		int tam = 4;
		line[tam++] = port/(1 << 8);
		line[tam++] = port%(1 << 8);
 		line[tam++] = (byte)strlen(ip);
		for(int j = 0; j < (int)strlen(ip); j++){
			line[tam++] = (byte)ip[j];
		}
		line[1] = tam/(1 << 8);
		line[2] = tam%(1 << 8);
		return tam;
	}
}

void InviteOpponentAckPackage::string_to_header(ustring recvline){
	resp = (int)recvline[3];
	port = (int)recvline[4]*(1 << 8) + (int)recvline[5];
	int tam = (int)recvline[6];
	ip = (char *)malloc((tam) * sizeof(char));
	for(ssize_t j = 7; j < 7 + tam; j++){
		ip[j - 7] = (char)recvline[j];
	}
}

InviteOpponentAckPackage::InviteOpponentAckPackage(int r){
	resp = r;
	port = 0;
	ip = (char *)malloc(sizeof(char));
	fixed_header.header_type = (byte)INVITE_OPPONENT_ACK_PACKAGE;
}


ssize_t SendMovePackage::header_to_string(ustring line){
	line[0] = this->fixed_header.header_type;
	line[1] = (byte)r;
	line[2] = (byte)c;
	return 3;
}

void SendMovePackage::string_to_header(ustring recvline){
	r = (int)recvline[1];
	c = (int)recvline[2];
}

SendMovePackage::SendMovePackage(int r, int c){
	this->r = r, this->c = c;
	fixed_header.header_type = (byte)SEND_MOVE_PACKAGE;
}
SendMovePackage::SendMovePackage(){
	this->r = 0, this->c = 0;
	fixed_header.header_type = (byte)SEND_MOVE_PACKAGE;
}

ssize_t EndMatchPackage::header_to_string(ustring line){
	line[0] = this->fixed_header.header_type;
    line[1] = pont;
    return 2;
}
EndMatchPackage::EndMatchPackage(int p){
	pont = p;
	fixed_header.header_type = (byte)END_MATCH_PACKAGE;
}

HeaderTemplate::HeaderTemplate(){
}

FixedHeader::FixedHeader(){
}

#endif