//
// Created by jackyan on 2021/3/18.
//

#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <string>

#include "../../easy-muduo/common/Log.h"

extern Logger logger;

class TcpConnection;
class Logical;

struct Player{
    Player(const int uid, const std::string& name);
    int uid;
    std::string name;
    int state;
};

struct Game{
    std::vector<int> uids;
    std::set<int> stills;
    Logical* logical;
    void input(int player_index, int op);
    void leave(int uid);
    Game(std::vector<int> uids);
    ~Game();
};

struct Server{
    Server();
    // uids;
    int uid_gen;
    std::map<TcpConnection*, Player*> conn_to_player;
    std::map<int, TcpConnection*> uid_to_conn;
    // matchs
    std::set<int> match_uids;
    // games;
    int gameid_gen;
    std::map<int, int> uid_to_gameid;
    std::map<int, Game*> gameid_to_game;

    // player <---> connection
    int delete_player_conn(TcpConnection* conn);
    Player* new_player(const TcpConnection* conn, const std::string& name);
    void leave_player(const TcpConnection* conn);
    // player <---> game
    void player_leave_game(int uid);
};

// TODO: store player state in the server  in hall/in game/game state.

#endif //SERVER_H
