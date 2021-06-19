#ifndef SERVER_IO_HPP
#define SERVER_IO_HPP

#include <ostream>
#include <string>

#define MAX_STR_LEN 128
#define USER_PATH "users.db"
#define LOG_PATH "ttt.log"

class user_t {
  public:
    char *name;
    char *password;
    int score;
    bool connected;
    bool in_match;
    char *ip;
    int port;
    int client_invitation;

    std::string get_user_str();

    friend std::ostream &operator<<(std::ostream &, const user_t &);
};

extern user_t **users;
#define MAX_USERS 1000
extern int *total_users;

void serialize_users(bool server_shutdown);
void deserialize_users();

typedef enum {
    SERVER_STARTED,
    CLIENT_CONNECTED,
    SUCCESS_USER_CREATED,
    UNSUCCESS_USER_CREATED,
    SUCCESS_LOGIN,
    UNSUCCESS_LOGIN,
    SUCCESS_LOGOUT,
    SUCCESS_CHANGE_PASS,
    UNSUCCESS_CHANGE_PASS,
    CLIENT_DISCONNECT,
    MATCH_STARTED,
    MATCH_FINISHED,
    UNEXPECTED_DISCONNECT,
    SERVER_FINISHED,
} log_t;

class log_struct_t {
  public:
    char *client_ip;
    std::string username;
    std::string player1_name;
    char *player1_ip;
    std::string player2_name;
    char *player2_ip;
    std::string winner_name;
};

void write_log_line(log_t, log_struct_t);

#endif /* ifndef SERVER_IO_HPP */
