#ifndef PACKAGES_CPP
#define PACKAGES_CPP

#include "packages.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iostream>

#include "server-io.hpp"
#include "util.hpp"

// FIXED HEADER
FixedHeader::FixedHeader() { }

FixedHeader::FixedHeader(ustring recvline) {
    this->package_type = recvline[0];
    this->remaning_length = byte_str_to_int(recvline, 1);
}

ustring FixedHeader::package_to_string() {
    ustring fixed_header_str;
    fixed_header_str = (ustring) malloc(3 * sizeof(uchar));
    fixed_header_str[0] = this->package_type;
    ustring temp = int_to_2byte_str(this->remaning_length);
    fixed_header_str[1] = temp[0];
    fixed_header_str[2] = temp[1];
    free(temp);

    return fixed_header_str;
}

void FixedHeader::write(ustring line, int &pos) {
    ustring temp = package_to_string();
    for (pos = 0; pos < 3; ++pos) line[pos] = temp[pos];
    free(temp);
}

// PACKAGE TEMPLATE
PackageTemplate::PackageTemplate() { }

// CREATE USER PACKAGE
CreateUserPackage::CreateUserPackage(std::string username,
                                     std::string password) {
    this->username = username;
    this->password = password;
}

CreateUserPackage::CreateUserPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    read_string(recvline, pos, this->username);
    read_string(recvline, pos, this->password);
}

ssize_t CreateUserPackage::package_to_string(ustring line) {
    int name_len = this->username.size();
    int pwd_len = this->password.size();
    int total_len = name_len + pwd_len + 4;

    this->fixed_header.package_type = CREATE_USER_PACKAGE;
    this->fixed_header.remaning_length = total_len;

    int pos;
    this->fixed_header.write(line, pos);
    write_string(line, pos, this->username);
    write_string(line, pos, this->password);

    return pos;
}

// CREATE USER ACK PACKAGE
CreateUserAckPackage::CreateUserAckPackage(byte return_code) {
    this->fixed_header.package_type = CREATE_USER_ACK_PACKAGE;
    this->fixed_header.remaning_length = 1;
    this->return_code = return_code;
}

CreateUserAckPackage::CreateUserAckPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    this->return_code = recvline[pos];
}

ssize_t CreateUserAckPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = CREATE_USER_ACK_PACKAGE;
    this->fixed_header.remaning_length = 1;

    int pos;
    this->fixed_header.write(line, pos);
    line[pos++] = this->return_code;

    return pos;
}

// LOGIN PACKAGE
LoginPackage::LoginPackage(std::string user_login, std::string user_password) {
    this->user_login = user_login;
    this->user_password = user_password;
}

LoginPackage::LoginPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    read_string(recvline, pos, this->user_login);
    read_string(recvline, pos, this->user_password);
}

ssize_t LoginPackage::package_to_string(ustring line) {
    int login_len = this->user_login.size();
    int password_len = this->user_password.size();
    int total_len = login_len + password_len + 4;

    this->fixed_header.package_type = LOGIN_PACKAGE;
    this->fixed_header.remaning_length = total_len;

    int pos;
    this->fixed_header.write(line, pos);
    write_string(line, pos, this->user_login);
    write_string(line, pos, this->user_password);

    return pos;
}

//  LOGIN ACK PACKAGE
LoginAckPackage::LoginAckPackage(byte return_code) {
    this->return_code = return_code;
}

LoginAckPackage::LoginAckPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    this->return_code = recvline[pos++];
}

ssize_t LoginAckPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = LOGIN_ACK_PACKAGE;
    this->fixed_header.remaning_length = 1;

    int pos;
    this->fixed_header.write(line, pos);
    line[pos++] = this->return_code;
    return pos;
}

// LOGOUT PACKAGE
LogoutPackage::LogoutPackage() { }

LogoutPackage::LogoutPackage(ustring recvline) {
    this->fixed_header = FixedHeader(recvline);
}

ssize_t LogoutPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = LOGOUT_PACKAGE;
    this->fixed_header.remaning_length = 0;

    int pos = 0;
    this->fixed_header.write(line, pos);

    return pos;
}

// CHANGE PASSWORD PACKAGE
ChangePasswordPackage::ChangePasswordPackage(std::string cur_password,
                                             std::string new_password) {
    this->cur_password = cur_password;
    this->new_password = new_password;
}

ChangePasswordPackage::ChangePasswordPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    read_string(recvline, pos, this->cur_password);
    read_string(recvline, pos, this->new_password);
}

ssize_t ChangePasswordPackage::package_to_string(ustring line) {
    int cur_password_len = this->cur_password.size();
    int new_password_len = this->new_password.size();
    int total_len = cur_password_len + new_password_len + 4;

    this->fixed_header.package_type = CHANGE_PASSWORD_PACKAGE;
    this->fixed_header.remaning_length = total_len;

    int pos = 0;
    this->fixed_header.write(line, pos);
    write_string(line, pos, this->cur_password);
    write_string(line, pos, this->new_password);

    return pos;
}

// CHANGE PASSWORD ACK PACKAGE
ChangePasswordAckPackage::ChangePasswordAckPackage(byte return_code) {
    this->return_code = return_code;
}

ChangePasswordAckPackage::ChangePasswordAckPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    this->return_code = recvline[pos++];
}

ssize_t ChangePasswordAckPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = CHANGE_PASSWORD_ACK_PACKAGE;
    this->fixed_header.remaning_length = 1;

    int pos = 0;
    this->fixed_header.write(line, pos);
    line[pos++] = this->return_code;

    return pos;
}

// REQUEST ALL CONNECTED USERS PACKAGE
ReqConnectedUsersPackage::ReqConnectedUsersPackage() { }
ReqConnectedUsersPackage::ReqConnectedUsersPackage(ustring recvline) {
    this->fixed_header = FixedHeader(recvline);
}

ssize_t ReqConnectedUsersPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = REQUEST_ALL_CONNECTED_USERS_PACKAGE;
    this->fixed_header.remaning_length = 0;

    int pos = 0;
    this->fixed_header.write(line, pos);

    return pos;
}

// RESPONSE ALL CONNECTED USERS PACKAGE
ResConnectedUsersPackage::ResConnectedUsersPackage() { }
ResConnectedUsersPackage::ResConnectedUsersPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);

    this->num_users = recvline[pos++];

    for (int i = 0; i < this->num_users; i++) {
        int len = byte_str_to_int(recvline, pos);
        pos += 2;
        this->pkg_users[i].name = (char *) malloc(len * sizeof(char));
        for (int j = 0; j < len; j++)
            this->pkg_users[i].name[j] = (char) recvline[pos++];
        this->pkg_users[i].name[len] = 0;
        this->pkg_users[i].score = recvline[pos++];
        this->pkg_users[i].connected = recvline[pos++];
        this->pkg_users[i].in_match = recvline[pos++];
    }
}
// ResConnectedUsersPackage::~ResConnectedUsersPackage() { }

ssize_t ResConnectedUsersPackage::package_to_string(ustring line) {
    int total_len = 1;
    for (int i = 0; i < *total_users; i++) {
        total_len += strlen(users[i]->name) + 2 + 3;
    }

    this->fixed_header.package_type = RESPONSE_ALL_CONNECTED_USERS_PACKAGE;
    this->fixed_header.remaning_length = total_len;

    int pos = 0;
    this->fixed_header.write(line, pos);
    line[pos++] = *total_users;

    for (int i = 0; i < *total_users; i++) {
        int len = strlen(users[i]->name);
        ustring strlen = int_to_2byte_str(len);
        line[pos++] = strlen[0];
        line[pos++] = strlen[1];
        free(strlen);
        for (int j = 0; j < len; ++j) { line[pos++] = users[i]->name[j]; }
        line[pos++] = users[i]->score;
        line[pos++] = users[i]->connected;
        line[pos++] = users[i]->in_match;
    }

    return pos;
}

void ResConnectedUsersPackage::show_users() {
    for (int i = 0; i < this->num_users; i++) {
        std::cout << "User [" << this->pkg_users[i].name << "]:" << std::endl
                  << "\tScore: " << this->pkg_users[i].score << std::endl
                  << "\tConnected: " << this->pkg_users[i].connected
                  << std::endl
                  << "\tPlaying: " << this->pkg_users[i].in_match << std::endl
                  << std::endl;
    }
}

// REQUEST CLASSIFICATIONS PACKAGE
ReqClassificationsPackage::ReqClassificationsPackage() { }
ReqClassificationsPackage::ReqClassificationsPackage(ustring recvline) {
    this->fixed_header = FixedHeader(recvline);
}

ssize_t ReqClassificationsPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = REQUEST_CLASSIFICATIONS_PACKAGE;
    this->fixed_header.remaning_length = 0;

    int pos = 0;
    this->fixed_header.write(line, pos);

    return pos;
}

// RESPONSE CLASSIFICATIONS PACKAGE
ResClassificationsPackage::ResClassificationsPackage() { }
ResClassificationsPackage::ResClassificationsPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);

    this->num_users = recvline[pos++];

    for (int i = 0; i < this->num_users; i++) {
        int len = byte_str_to_int(recvline, pos);
        pos += 2;
        this->pkg_users[i].name = (char *) malloc(len * sizeof(char));
        for (int j = 0; j < len; j++)
            this->pkg_users[i].name[j] = (char) recvline[pos++];
        this->pkg_users[i].name[len] = 0;
        this->pkg_users[i].score = recvline[pos++];
    }
}

ssize_t ResClassificationsPackage::package_to_string(ustring line) {
    int total_len = 1;
    for (int i = 0; i < *total_users; i++) {
        total_len += strlen(users[i]->name) + 2 + 1;
    }

    this->fixed_header.package_type = RESPONSE_CLASSIFICATIONS_PACKAGE;
    this->fixed_header.remaning_length = total_len;

    int pos = 0;
    this->fixed_header.write(line, pos);
    line[pos++] = *total_users;

    for (int i = 0; i < *total_users; i++) {
        int len = strlen(users[i]->name);
        ustring strlen = int_to_2byte_str(len);
        line[pos++] = strlen[0];
        line[pos++] = strlen[1];
        free(strlen);
        for (int j = 0; j < len; ++j) { line[pos++] = users[i]->name[j]; }
        line[pos++] = users[i]->score;
    }

    return pos;
}
void ResClassificationsPackage::show_users() {
    auto great = [](user_t a, user_t b) { return a.score > b.score; };
    std::sort(this->pkg_users, this->pkg_users + this->num_users, great);

    for (int i = 0; i < this->num_users; i++) {
        std::cout << "User [" << this->pkg_users[i].name << "]:" << std::endl
                  << "\tScore: " << this->pkg_users[i].score << std::endl
                  << "\tClassification: " << i + 1 << std::endl
                  << std::endl;
    }
}

ssize_t PingReqPackage::package_to_string(ustring line) {
    line[0] = this->fixed_header.package_type;
    line[1] = line[2] = 0;
    return 3;
}
PingReqPackage::PingReqPackage() { fixed_header.package_type = PINGREQ_PACKAGE; }

ssize_t PingBackPackage::package_to_string(ustring line) {
    line[0] = this->fixed_header.package_type;
    line[1] = line[2] = 0;
    return 3;
}
PingBackPackage::PingBackPackage() {
    fixed_header.package_type = PINGBACK_PACKAGE;
}

ssize_t InviteOpponentPackage::package_to_string(ustring line) {
    int pos = 0;
    this->fixed_header.write(line, pos);
    write_string(line, pos, this->cliente);
    return pos;
}
InviteOpponentPackage::InviteOpponentPackage(std::string c) {
    this->cliente = c;
    this->fixed_header.package_type = (byte) INVITE_OPPONENT_PACKAGE;
    this->fixed_header.remaning_length = this->cliente.size() + 2;
}
InviteOpponentPackage::InviteOpponentPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    read_string(recvline, pos, this->cliente);
}

ssize_t InviteOpponentAckPackage::package_to_string(ustring line) {
    line[0] = this->fixed_header.package_type;
    line[3] = (byte) resp;
    if (resp == 0) {
        line[1] = 0;
        line[2] = 1;
        return 4;
    } else {
        int tam = 4;
        line[tam++] = port / (1 << 8);
        line[tam++] = port % (1 << 8);
        line[tam++] = (byte) strlen(ip);
        for (int j = 0; j < (int) strlen(ip); j++) {
            line[tam++] = (byte) ip[j];
        }
        line[1] = tam / (1 << 8);
        line[2] = tam % (1 << 8);
        return tam;
    }
}

void InviteOpponentAckPackage::string_to_header(ustring recvline) {
    resp = (int) recvline[3];
    port = (int) recvline[4] * (1 << 8) + (int) recvline[5];
    int tam = (int) recvline[6];
    ip = (char *) malloc((tam) * sizeof(char));
    for (ssize_t j = 7; j < 7 + tam; j++) { ip[j - 7] = (char) recvline[j]; }
}

InviteOpponentAckPackage::InviteOpponentAckPackage(int r) {
    resp = r;
    port = 0;
    ip = (char *) malloc(sizeof(char));
    fixed_header.package_type = (byte) INVITE_OPPONENT_ACK_PACKAGE;
}

ssize_t SendMovePackage::package_to_string(ustring line) {
    line[0] = this->fixed_header.package_type;
    line[1] = (byte) r;
    line[2] = (byte) c;
    return 3;
}

void SendMovePackage::string_to_header(ustring recvline) {
    r = (int) recvline[1];
    c = (int) recvline[2];
}

SendMovePackage::SendMovePackage(int r, int c) {
    this->r = r, this->c = c;
    fixed_header.package_type = (byte) SEND_MOVE_PACKAGE;
}
SendMovePackage::SendMovePackage() {
    this->r = 0, this->c = 0;
    fixed_header.package_type = (byte) SEND_MOVE_PACKAGE;
}

ssize_t EndMatchPackage::package_to_string(ustring line) {
    line[0] = this->fixed_header.package_type;
    line[1] = pont;
    return 2;
}
EndMatchPackage::EndMatchPackage(int p) {
    pont = p;
    fixed_header.package_type = (byte) END_MATCH_PACKAGE;
}

ReconnectPackage::ReconnectPackage(std::string username) {
    this->username = username;
}

ReconnectPackage::ReconnectPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    read_string(recvline, pos, this->username);
}


ssize_t ReconnectPackage::package_to_string(ustring line) {
    this->fixed_header.package_type = RECONNECT_PACKAGE;
    this->fixed_header.remaning_length = 2 + this->username.size();

    int pos = 0;
    this->fixed_header.write(line, pos);
    write_string(line, pos, this->username);

    return pos;
}

#endif /* ifndef PACKAGES_CPP */
