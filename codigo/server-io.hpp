#ifndef SERVER_IO_HPP
#define SERVER_IO_HPP

#include <string>

class user {
  private:
     std::string name;
     std::string password;
     int score;
     bool connected;
     bool in_match;
};

extern user* users;

void serialize_users();
void deserialize_users();

typedef enum {
   SERVER_STARTED,
   CLIENT_CONNECTED,
   SUCCESS_LOGIN,
   UNSUCCESS_LOGIN,
   SUCCESS_LOGOUT,
   MATCH_STARTED,
   MATCH_FINISHED,
   UNEXPECTED_DISCONNECT,
   SERVER_FINISHED,
} log_t;

void write_log_line(log_t);

#endif /* ifndef SERVER_IO_HPP */
