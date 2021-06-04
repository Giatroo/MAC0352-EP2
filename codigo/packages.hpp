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
   byte header_type;
   byte remaning_length[2];

   FixedHeader();
   FixedHeader(ustring recvline, size_t len);
};

class HeaderTemplate {
   FixedHeader fixed_header;

   HeaderTemplate();
   HeaderTemplate(ustring recvline, size_t len);

   virtual ustring header_to_string(size_t &len) = 0;
};

class LoginPackage : public HeaderTemplate {
   std::string user_login;
   std::string user_password;
};

class LoginAckPackage : public HeaderTemplate {
   byte connection_accepted;
};



#endif /* ifndef PACKAGES_HPP */
