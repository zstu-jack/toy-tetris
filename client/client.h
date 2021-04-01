//
// Created by jackyan on 2021/3/22.
//

#ifndef CLIENT_H
#define CLIENT_H

struct LocalPlayer {
    // net.
    TcpConnection* conn;
    int login;

    // player attr.
    std::string player_name;
    int player_uid;
    int player_state;

    // game
    int gameid;
    std::vector<int> game_uids;
    Logical logical;

    void send(const std::string& msg);
};

#endif //CLIENT_H
