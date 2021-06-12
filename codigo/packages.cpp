#ifndef PACKAGES_CPP
#define PACKAGES_CPP

#include "packages.hpp"

#include <stdio.h>
#include <stdlib.h>

#include "util.hpp"

// FIXED HEADER

FixedHeader::FixedHeader() { }

FixedHeader::FixedHeader(ustring recvline) {
    this->header_type = recvline[0];
    this->remaning_length = byte_str_to_int(recvline, 1);
}

ustring FixedHeader::header_to_string() {
    ustring fixed_header_str;
    fixed_header_str = (ustring) malloc(3 * sizeof(uchar));
    fixed_header_str[0] = this->header_type;
    ustring temp = int_to_2byte_str(this->remaning_length);
    fixed_header_str[1] = temp[0];
    fixed_header_str[2] = temp[1];
    free(temp);

    return fixed_header_str;
}

void FixedHeader::write(ustring line, int &pos) {
    ustring temp = header_to_string();
    for (pos = 0; pos < 3; ++pos) line[pos] = temp[pos];
    free(temp);
}

// HEADER TEMPLATE
HeaderTemplate::HeaderTemplate() { }

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

ssize_t CreateUserPackage::header_to_string(ustring line) {
    int name_len = this->username.size();
    int pwd_len = this->password.size();
    int total_len = name_len + pwd_len + 4;

    this->fixed_header.header_type = CREATE_USER_PACKAGE;
    this->fixed_header.remaning_length = total_len;

    int pos;
    this->fixed_header.write(line, pos);
    write_string(line, pos, this->username);
    write_string(line, pos, this->password);

    return pos;
}

// CREATE USER ACK PACKAGE
CreateUserAckPackage::CreateUserAckPackage(byte return_code) {
    this->fixed_header.header_type = CREATE_USER_ACK_PACKAGE;
    this->fixed_header.remaning_length = 1;
    this->return_code = return_code;
}

CreateUserAckPackage::CreateUserAckPackage(ustring recvline) {
    int pos = 3;
    this->fixed_header = FixedHeader(recvline);
    this->return_code = recvline[pos];
}

ssize_t CreateUserAckPackage::header_to_string(ustring line) {
    this->fixed_header.header_type = CREATE_USER_ACK_PACKAGE;
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

ssize_t LoginPackage::header_to_string(ustring line) {
    int login_len = this->user_login.size();
    int password_len = this->user_password.size();
    int total_len = login_len + password_len + 4;

    this->fixed_header.header_type = LOGIN_PACKAGE;
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

ssize_t LoginAckPackage::header_to_string(ustring line) {
    this->fixed_header.header_type = LOGIN_ACK_PACKAGE;
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

ssize_t LogoutPackage::header_to_string(ustring line) {
    this->fixed_header.header_type = LOGOUT_PACKAGE;
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

ssize_t ChangePasswordPackage::header_to_string(ustring line) {
    int cur_password_len = this->cur_password.size();
    int new_password_len = this->new_password.size();
    int total_len = cur_password_len + new_password_len + 4;

    this->fixed_header.header_type = CHANGE_PASSWORD_PACKAGE;
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

ssize_t ChangePasswordAckPackage::header_to_string(ustring line) {
    this->fixed_header.header_type = CHANGE_PASSWORD_ACK_PACKAGE;
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

ssize_t ReqConnectedUsersPackage::header_to_string(ustring line) {
    this->fixed_header.header_type = REQUEST_ALL_CONNECTED_USERS_PACKAGE;
    this->fixed_header.remaning_length = 0;

    int pos = 0;
    this->fixed_header.write(line, pos);

    return pos;
}

// RESPONSE ALL CONNECTED USERS PACKAGE
ResConnectedUsersPackage::ResConnectedUsersPackage() { }
ResConnectedUsersPackage::ResConnectedUsersPackage(ustring recvline) { }

ssize_t ResConnectedUsersPackage::header_to_string(ustring line) { return 0; }

#endif /* ifndef PACKAGES_CPP */
