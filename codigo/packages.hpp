#ifndef PACKAGES_HPP
#define PACKAGES_HPP

#include <cstddef>
#include <string>

#include "util.hpp"

typedef enum {
    CONNECT_PACKAGE,
    LOGIN_PACKAGE,
    LOGIN_ACK_PACKAGE,
    LOGOUT_PACKAGE,
    CREATE_USER_PACKAGE,
    CREATE_USER_ACK_PACKAGE,
    CHANGE_PASSWORD_PACKAGE,
    CHANGE_PASSWORD_ACK_PACKAGE,
    REQUEST_ALL_CONNECTED_USERS_PACKAGE,
    RESPONSE_ALL_CONNECTED_USERS_PACKAGE,
    REQUEST_CLASSIFICATIONS_PACKAGE,
    RESPONSE_CLASSIFICATIONS_PACKAGE,
    INVITE_OPPONENT_PACKAGE,
    INVITE_OPPONENT_ACK_PACKAGE,
    START_MATCH_PACKAGE,
    SEND_MOVE_PACKAGE,
    END_MATCH_PACKAGE,
    PINGREQ_PACKAGE,
    PINGBACK_PACKAGE,
} package_t;

class FixedHeader {
  public:
    byte header_type;
    int remaning_length;

    FixedHeader();
    FixedHeader(ustring recvline);

    ustring header_to_string();
    void write(ustring line, int &pos);
};

class HeaderTemplate {
  public:
    HeaderTemplate();

    FixedHeader fixed_header;

    virtual ssize_t header_to_string(ustring line) = 0;
};

class CreateUserPackage : public HeaderTemplate {
  public:
    CreateUserPackage(std::string, std::string);
    CreateUserPackage(ustring recvline);

    std::string username;
    std::string password;

    ssize_t header_to_string(ustring line);
};

class CreateUserAckPackage : public HeaderTemplate {
  public:
    byte return_code;

    CreateUserAckPackage(byte);
    CreateUserAckPackage(ustring line);

    ssize_t header_to_string(ustring line);
};

class LoginPackage : public HeaderTemplate {
  public:
    LoginPackage(std::string, std::string);
    LoginPackage(ustring recvline);

    std::string user_login;
    std::string user_password;

    ssize_t header_to_string(ustring line);
};

class LoginAckPackage : public HeaderTemplate {
  public:
    byte return_code;

    LoginAckPackage(byte);
    LoginAckPackage(ustring recvline);

    ssize_t header_to_string(ustring line);
};

class LogoutPackage : public HeaderTemplate {
  public:
    LogoutPackage();
    LogoutPackage(ustring recvline);

    ssize_t header_to_string(ustring line);
};

class ChangePasswordPackage : public HeaderTemplate {
  public:
    std::string cur_password;
    std::string new_password;

    ChangePasswordPackage(std::string, std::string);
    ChangePasswordPackage(ustring recvline);

    ssize_t header_to_string(ustring line);
};

class ChangePasswordAckPackage : public HeaderTemplate {
  public:
    byte return_code;

    ChangePasswordAckPackage(byte);
    ChangePasswordAckPackage(ustring recvline);

    ssize_t header_to_string(ustring line);
};

class ReqConnectedUsersPackage : public HeaderTemplate {
  public:
    ReqConnectedUsersPackage();
    ReqConnectedUsersPackage(ustring recvline);

    ssize_t header_to_string(ustring line);
};

// RESPONSE ALL CONNECTED USERS PACKAGE
class ResConnectedUsersPackage : public HeaderTemplate {
  public:
    ResConnectedUsersPackage();
    ResConnectedUsersPackage(ustring recvline);

    ssize_t header_to_string(ustring line);
};

#endif /* ifndef PACKAGES_HPP */
