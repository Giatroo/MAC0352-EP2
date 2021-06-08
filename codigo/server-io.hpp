#ifndef SERVER_IO_HPP
#define SERVER_IO_HPP

#include <ostream>
#include <string>

#define MAX_STR_LEN 128

class user_t {
  public:
    char *name;
    char *password;
    int score;
    bool connected;
    bool in_match;

    void set_name(std::string);
    void set_password(std::string);

    friend std::ostream &operator<<(std::ostream &, const user_t &);
};

extern user_t **users;
#define MAX_USERS 1000
extern int *total_users;

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
