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

    ustring write_fixed_header();
};

class HeaderTemplate {
  public:
    FixedHeader fixed_header;

    HeaderTemplate();

    virtual void string_to_header(ustring recvline, size_t len) = 0;
    virtual ustring header_to_string(size_t &len) = 0;
};

class CreateUserPackage : public HeaderTemplate {
  public:
    ustring header_to_string(size_t &len);
    void string_to_header(ustring recvline, size_t len);
    std::string username;
    std::string password;
};

class LoginPackage : public HeaderTemplate {
   public:
    std::string user_login;
    std::string user_password;
};

class LoginAckPackage : public HeaderTemplate {
   public:
    byte connection_accepted;
};

#endif /* ifndef PACKAGES_HPP */
