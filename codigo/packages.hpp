#ifndef PACKAGES_HPP
#define PACKAGES_HPP

#include <cstddef>
#include <string>

#include "server-io.hpp"
#include "util.hpp"

typedef enum {
    CONNECT_PACKAGE,
    RECONNECT_PACKAGE,
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
    byte package_type;
    int remaning_length;

    FixedHeader();
    FixedHeader(ustring recvline);

    ustring package_to_string();
    void write(ustring line, int &pos);
};

class PackageTemplate {
  public:
    PackageTemplate();

    FixedHeader fixed_header;

    virtual ssize_t package_to_string(ustring line) = 0;
};

class CreateUserPackage : public PackageTemplate {
  public:
    CreateUserPackage(std::string, std::string);
    CreateUserPackage(ustring recvline);

    std::string username;
    std::string password;

    ssize_t package_to_string(ustring line);
};

class CreateUserAckPackage : public PackageTemplate {
  public:
    byte return_code;

    CreateUserAckPackage(byte);
    CreateUserAckPackage(ustring line);

    ssize_t package_to_string(ustring line);
};

class LoginPackage : public PackageTemplate {
  public:
    LoginPackage(std::string, std::string);
    LoginPackage(ustring recvline);

    std::string user_login;
    std::string user_password;

    ssize_t package_to_string(ustring line);
};

class LoginAckPackage : public PackageTemplate {
  public:
    byte return_code;

    LoginAckPackage(byte);
    LoginAckPackage(ustring recvline);

    ssize_t package_to_string(ustring line);
};

class LogoutPackage : public PackageTemplate {
  public:
    LogoutPackage();
    LogoutPackage(ustring recvline);

    ssize_t package_to_string(ustring line);
};

class ChangePasswordPackage : public PackageTemplate {
  public:
    std::string cur_password;
    std::string new_password;

    ChangePasswordPackage(std::string, std::string);
    ChangePasswordPackage(ustring recvline);

    ssize_t package_to_string(ustring line);
};

class ChangePasswordAckPackage : public PackageTemplate {
  public:
    byte return_code;

    ChangePasswordAckPackage(byte);
    ChangePasswordAckPackage(ustring recvline);

    ssize_t package_to_string(ustring line);
};

class ReqConnectedUsersPackage : public PackageTemplate {
  public:
    ReqConnectedUsersPackage();
    ReqConnectedUsersPackage(ustring recvline);

    ssize_t package_to_string(ustring line);
};

class ResConnectedUsersPackage : public PackageTemplate {
  public:
    ResConnectedUsersPackage();
    ResConnectedUsersPackage(ustring recvline);

    int num_users;
    user_t pkg_users[MAX_USERS];

    ssize_t package_to_string(ustring line);
    void show_users();
};

class ReqClassificationsPackage : public PackageTemplate {
  public:
    ReqClassificationsPackage();
    ReqClassificationsPackage(ustring recvline);

    ssize_t package_to_string(ustring line);
};

class ResClassificationsPackage : public PackageTemplate {
  public:
    ResClassificationsPackage();
    ResClassificationsPackage(ustring recvline);

    int num_users;
    user_t pkg_users[MAX_USERS];

    ssize_t package_to_string(ustring line);
    void show_users();
};

class PingReqPackage : public PackageTemplate {
  public:
    PingReqPackage();
    ssize_t package_to_string(ustring line);
};

class PingBackPackage : public PackageTemplate {
  public:
    PingBackPackage();
    ssize_t package_to_string(ustring line);
};

class InviteOpponentPackage : public PackageTemplate {
  public:
    InviteOpponentPackage(std::string c);
    InviteOpponentPackage(ustring recvline);

    std::string cliente;

    ssize_t package_to_string(ustring line);
};

class InviteOpponentAckPackage : public PackageTemplate {
  public:
    int resp;
    char *ip;
    int port;
    InviteOpponentAckPackage(int r);
    ssize_t package_to_string(ustring line);
    void string_to_header(ustring recvline);
};

class SendMovePackage : public PackageTemplate {
  public:
    int r, c;
    SendMovePackage();
    SendMovePackage(int r, int c);
    ssize_t package_to_string(ustring line);
    void string_to_header(ustring recvline);
};

class EndMatchPackage : public PackageTemplate {
  public:
    int pont;
    EndMatchPackage(int p);
    ssize_t package_to_string(ustring line);
};

class ReconnectPackage : public PackageTemplate {
  public:
    ReconnectPackage(std::string username);
    ReconnectPackage(ustring recvline);

    std::string username;

    ssize_t package_to_string(ustring line);
};

#endif /* ifndef PACKAGES_HPP */
