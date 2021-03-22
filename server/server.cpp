//
// Created by jackyan on 2021/3/22.
//

#include "../easy-muduo/common/EventLoop.h"
#include "../easy-muduo/common/TcpServer.h"
#include "../common/head.h"
#include "../common/util.h"
#include "../common/common_define.h"
#include "../protocal/tetris.pb.h"
#include "../logical.h"

#include "server.h"
#include "handlers.h"

extern Server server;

Player::Player(const int uid, const std::string& name){
    this->uid = uid;
    this->name = name;
}

Server::Server(){ uid_gen = gameid_gen = 1;}

int Server::delete_player_conn(TcpConnection* conn){
    auto connection_iter = conn_to_player.find(conn);
    if(connection_iter == conn_to_player.end()){
        return -1;
    }
    assert(uid_to_conn.count(connection_iter->second->uid));

    int uid = connection_iter->second->uid;
    uid_to_conn.erase(uid);

    delete connection_iter->second;
    connection_iter->second = nullptr;
    conn_to_player.erase(connection_iter);
    return uid;
}

Player* Server::new_player(const TcpConnection* conn, const std::string& name){
    auto* non_const_connection = const_cast<TcpConnection*>(conn);
    delete_player_conn(non_const_connection);
    auto* player = new Player(uid_gen ++, name);
    conn_to_player[non_const_connection] = player;
    uid_to_conn[player->uid] = non_const_connection;
    return player;
}
void Server::leave_player(const TcpConnection* conn){
    auto* local_conn = const_cast<TcpConnection*>(conn);
    // connection
    int uid = delete_player_conn(local_conn);
    // match.
    match_uids.erase(uid);
    // game
    player_leave_game(uid);

}
void Server::player_leave_game(int uid){
    auto iter = uid_to_gameid.find(uid);
    if(iter == uid_to_gameid.end()){
        return ;
    }
    Game* game = gameid_to_game[iter->second];
    game->leave(uid);
    if(game->stills.size() == 0){
        delete game;
        gameid_to_game.erase(iter->second);
    }
    uid_to_gameid.erase(uid);
}


Game::Game(std::vector<int> uids) {
    this->uids = uids;
    this->stills.insert(uids.begin(), uids.end());
    this->logical = new Logical();
}
void Game::input(int player_index, int op){
    if(player_index >= uids.size()){
        logger.LOG(ERROR, "index too large, player_index = %d, op = %d", player_index, op);
        return ;
    }
    logger.LOG(DETAIL, "player_index = %d, op = %d", player_index, op);
    InfGamePlayerOp ops;
    ops.set_gameid(server.uid_to_gameid[uids[player_index]]);
    auto* opptr = ops.add_ops();
    opptr->set_op(op);
    opptr->set_uid(uids[player_index]);
    for(int uid : stills){
        server.uid_to_conn[uid]->send(packMessage(MessageID::GAME_INF_PLAYER_OP, uid, ops));
    }
}
void Game::leave(int uid){
    stills.erase(uid);
}
Game::~Game(){
    delete logical;
}