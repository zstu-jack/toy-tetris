//
// Created by jackyan on 2021/3/12.
//
#include <functional>

#include "../easy-muduo/common/EventLoop.h"
#include "../easy-muduo/common/TcpServer.h"
#include "../common/head.h"
#include "../common/util.h"
#include "../common/common_define.h"
#include "../protocal/tetris.pb.h"
#include "../logical.h"

#include "server.h"

Logger logger(DETAIL, "server_log");
long long current_timestamp_ms = 0;

Server::Server(){ uid_gen = gameid_gen = 1;}
Player* Server::new_player(const TcpConnection* conn, const std::string& name){
    auto iter = conn_to_player.find(const_cast<TcpConnection*>(conn));
    if(iter != conn_to_player.end()){
        delete iter->second;
        iter->second = nullptr;
    }
    Player* player = new Player();
    player->name = name;
    player->uid = uid_gen ++;
    conn_to_player[const_cast<TcpConnection*>(conn)] = player;
    uid_to_conn[player->uid] = const_cast<TcpConnection*>(conn);

    return player;
}
void Server::leave_player(const TcpConnection* conn){
    auto* local_conn = const_cast<TcpConnection*>(conn);
    auto cp = conn_to_player.find(local_conn);
    if(cp != conn_to_player.end()) {
        int uid = cp->second->uid;
        // conn.
        uid_to_conn.erase(uid);
        delete cp->second;
        cp->second = nullptr;

        conn_to_player.erase(local_conn);
        // match.
        match_uids.erase(uid);
        // game
        player_leave_game(uid);
    }else{
        logger.log(DETAIL, " leave without uid, fd = %d", conn->get_fd());
    }
}
void Server::player_leave_game(int uid){
    auto iter = uid_to_gameid.find(uid);
    if(iter != uid_to_gameid.end()){
        Game* game = gameid_to_game[iter->second];
        game->leave(uid);
        if(game->stills.size() == 0){
            delete game->logical;
            delete game;
            gameid_to_game.erase(uid);
        }
        uid_to_gameid.erase(uid);
    }
}
Server server;


void Game::input(int player_index, int op){
    if(player_index >= uids.size()){
        logger.log(ERROR, "index too large, player_index = %d, op = %d\n", player_index, op);
        return ;
    }
    logger.log(DETAIL, "player_index = %d, op = %d\n", player_index, op);
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


std::map<int, std::function<void(const TcpConnection* , Head&, const char*, int)> > callbacks;
// std::map<int, shared_ptr<logical>>

// == -1  error, which would lead to close the connection
// >  0   package size.
// == 0   wait for head.
int decodeMessage(const TcpConnection* conn, const char* msg, int len){
    if(len < 4) return 0;
    auto pkgSize = ntohl(*(int32_t *) msg);
    if(pkgSize >= SOCKET_APP_MAX_BUFFER_SIZE || pkgSize < 0){
        logger.log(WARNING, "[size = %d]\n", pkgSize);
        return -1;
    }
    return pkgSize;
}
void onConnection(const TcpConnection* conn){
    // you can echo peer's ip and port here.
    if(conn->connected()){
        logger.log(DETAIL,"connected [fd = %d]\n",  conn->get_fd());
    }else{
        logger.log(DETAIL,"disconnected [fd = %d]\n",  conn->get_fd());
        server.leave_player(conn);
    }
}
void onMessage(const TcpConnection* conn, const char* msg, int len){
    auto ptr = msg;
    Head head{ntohl(*(uint32_t *) ptr), ntohl(*(uint32_t *) (ptr+4)), ntohl(*(uint32_t *) (ptr+8))};
    logger.log(DETAIL,"[fd = %d, head = %s]\n", conn->get_fd(), head.to_string().c_str());
    callbacks[head.msg_id](conn, head, msg, len);
}

void onReqLogin(const TcpConnection* conn, Head& head, const char* msg, int len){
    ReqPlayerLogin req;
    req.ParseFromArray(msg+head.size(), len-head.size());
    auto* player = server.new_player(conn, req.name());
    logger.log(DETAIL, "login, uid = %d, fd = %d", head.uid, conn->get_fd());
    RspPlayerLogin rsp;
    rsp.set_uid(player->uid);

    server.uid_to_conn[player->uid]->send(packMessage(MessageID::RSP_LOGIN, player->uid, rsp));
}
void onGamePlayerLeave(const TcpConnection* conn, Head& head, const char* msg, int len){
    if(head.uid < 0){
        logger.log(ERROR, "leave, uid = %d, fd = %d\n", head.uid, conn->get_fd());
        return ;
    }
    logger.log(DETAIL, "on leave, uid = %d, fd = %d", head.uid, conn->get_fd());
    server.player_leave_game(head.uid);
}
void onGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len){
    if(head.uid < 0){
        logger.log(ERROR, "leave, uid = %d, fd = %d\n", head.uid, conn->get_fd());
        return ;
    }
    logger.log(DETAIL, "op, uid = %d, fd = %d\n", head.uid, conn->get_fd());
    ReqGamePlayerOp req;
    req.ParseFromArray(msg+head.size(), len-head.size());
    int op = req.op();

    auto iter = server.uid_to_gameid.find(head.uid);
    if(iter != server.uid_to_gameid.end()){
        int gameid = iter->second;
        auto* game = server.gameid_to_game[gameid];
        int index = -1;
        for(int i = 0; i < game->uids.size(); i ++){
            if(game->uids[i] == head.uid){
                index = i;
                break;
            }
        }
        if(index == -1){
            logger.log(ERROR, "index == -1, uid= %d, fd = %d\n", head.uid, conn->get_fd());
            return ;
        }
        game->logical->input(index, op);
    }else{
        logger.log(WARNING, "no such uid in uin_to_gameid, uid=%d\n", head.uid);
    }
}
void onReqPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len){
    ReqPlayerMatch req;
    req.ParseFromArray(msg+head.size(), len-head.size());
    if(head.uid < 0){
        logger.log(WARNING, "match, uid = %d, fd = %d", head.uid, conn->get_fd());
        return ;
    }
    logger.log(DETAIL, "enter match, uid = %d, fd = %d", head.uid, conn->get_fd());
    RspPlayerMatch rsp;
    server.uid_to_conn[head.uid]->send(packMessage(MessageID::RSP_PLAYER_MATCH, head.uid, rsp));
    server.match_uids.insert(head.uid);
    if(server.match_uids.size() >= MATCH_PLAYER_NUM){
        InfPlayerMatchSuccess inf;
        inf.set_gameid(server.gameid_gen);
        std::vector<int> vuids;
        for(int i = 0; i < MATCH_PLAYER_NUM; i ++) {
            int uid = *server.match_uids.begin();
            server.match_uids.erase(uid);
            inf.add_uids(uid);
            vuids.push_back(uid);
        }
        for(int uid : vuids) {
            server.uid_to_conn[uid]->send(packMessage(MessageID::INF_PLAYER_MATCH_SUCCESS, uid, inf));
        }

        Game* game = new Game();
        game->uids = vuids;
        game->stills.insert(game->uids.begin(), game->uids.end());
        game->logical = new Logical();
        game->logical->init(MATCH_PLAYER_NUM, current_timestamp_ms, GAME_MODE_MATCH);
        game->logical->start();
        game->logical->set_server(game);

        for(int uid : vuids){
            server.uid_to_gameid[uid] = server.gameid_gen;
        }
        server.gameid_to_game[server.gameid_gen] = game;
        server.gameid_gen ++;
    }
}


void tick(){
    for(auto pi : server.gameid_to_game){
        pi.second->logical->update(current_timestamp_ms);
    }
}

int main(){

    {
        using namespace std::placeholders;
        callbacks[(int) MessageID::REQ_LOGIN] = std::bind(onReqLogin, _1, _2, _3, _4);
        callbacks[(int) MessageID::REQ_PLAYER_MATCH] = std::bind(onReqPlayerMatch, _1, _2, _3, _4);
        callbacks[(int) MessageID::REQ_GAME_PLAYER_OP] = std::bind(onGamePlayerOp, _1,_2,_3,_4);
        callbacks[(int) MessageID::REQ_GAME_PLAYER_LEAVE] = std::bind(onGamePlayerLeave, _1,_2,_3,_4);

    }

    EventLoop loop;
    loop.set_logger(&logger);

    TcpServer server(&loop, SOCKET_PORT);
    server.setConnectionCallback(std::bind(&onConnection, _1));
    server.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    server.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2, _3));
    server.start(SOCKET_BACKLOG_SIZE);   // enable reading event

    bool quit = false;
    do{
        current_timestamp_ms = get_1970_ms();
        tick();
        loop.update(SERVER_LOOP_TIMEOUT);
    }while(!quit);
    return 0;
}