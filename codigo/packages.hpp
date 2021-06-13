#ifndef PACKAGES_HPP
#define PACKAGES_HPP

#include "util.hpp"
#include <string>

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
   byte remaning_length[2];

   FixedHeader();
   FixedHeader(ustring recvline, ssize_t len);
};

class HeaderTemplate {
public:
   FixedHeader fixed_header;

   HeaderTemplate();
   HeaderTemplate(ustring recvline, ssize_t len);

   virtual ssize_t header_to_string(ustring line) = 0;
};

class LoginPackage : public HeaderTemplate {
   std::string user_login;
   std::string user_password;
};

class LoginAckPackage : public HeaderTemplate {
   byte connection_accepted;
};

class PingReqPackage : public HeaderTemplate {
public:
   PingReqPackage();
   ssize_t header_to_string(ustring line);
};

class PingBackPackage : public HeaderTemplate {
public:
   PingBackPackage();
   ssize_t header_to_string(ustring line);
};

class InviteOpponentPackage : public HeaderTemplate {
public:
   int cliente;
   InviteOpponentPackage(int c);
   ssize_t header_to_string(ustring line);
};

class InviteOpponentAckPackage : public HeaderTemplate {
public:
   int resp;
   char * ip;
   int port;
   InviteOpponentAckPackage(int r);
   ssize_t header_to_string(ustring line);
   void string_to_header(ustring recvline);
};

class SendMovePackage : public HeaderTemplate {
public:
   int r, c;
   SendMovePackage();
   SendMovePackage(int r, int c);
   ssize_t header_to_string(ustring line);
   void string_to_header(ustring recvline);
};

class EndMatchPackage : public HeaderTemplate {
public:
   int pont;
   EndMatchPackage(int p);
   ssize_t header_to_string(ustring line);
};

#endif /* ifndef PACKAGES_HPP */
