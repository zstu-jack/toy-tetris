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
extern Logger logger;
extern long long current_timestamp_ms;

void onReqLogin(const TcpConnection* conn, Head& head, const char* msg, int len){
    ReqPlayerLogin req;
    req.ParseFromArray(msg+head.size(), len-head.size());
    auto* player = server.new_player(conn, req.name());
    logger.LOG(DETAIL, "login, uid = %d, fd = %d", head.uid, conn->get_fd());
    RspPlayerLogin rsp;
    rsp.set_uid(player->uid);
    server.uid_to_conn[player->uid]->send(packMessage(MessageID::RSP_LOGIN, player->uid, rsp));
}
void onGamePlayerLeave(const TcpConnection* conn, Head& head, const char* msg, int len){
    if(head.uid < 0){
        logger.LOG(ERROR, "leave game uid < 0, uid = %d, fd = %d", head.uid, conn->get_fd());
        return ;
    }
    logger.LOG(DETAIL, "leave game, uid = %d, fd = %d", head.uid, conn->get_fd());
    server.player_leave_game(head.uid);
    server.uid_to_conn[head.uid]->send(packMessage(MessageID::RSP_GAME_PLAYER_LEAVE, head.uid));
}
void onGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len){
    if(head.uid < 0){
        logger.LOG(ERROR, "game operate uid < 0, uid = %d, fd = %d", head.uid, conn->get_fd());
        return ;
    }
    logger.LOG(DETAIL, "game operate, uid = %d, fd = %d", head.uid, conn->get_fd());
    ReqGamePlayerOp req;
    req.ParseFromArray(msg+head.size(), len-head.size());
    int op = req.op();

    auto iter = server.uid_to_gameid.find(head.uid);
    if(iter == server.uid_to_gameid.end()) {
        logger.LOG(WARNING, "no such uid in uid_to_gameid, uid=%d", head.uid);
        return ;
    }
    int gameid = iter->second;
    auto* game = server.gameid_to_game[gameid];
    int index = index_of(game->uids.begin(), game->uids.end(), head.uid);
    if(index == -1){
        logger.LOG(ERROR, "index == -1, uid= %d, fd = %d", head.uid, conn->get_fd());
        return ;
    }
    game->logical->input(index, op);
}
void onReqPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len){
    ReqPlayerMatch req;
    req.ParseFromArray(msg+head.size(), len-head.size());
    if(head.uid < 0){
        logger.LOG(WARNING, "uin < 0, uid = %d, fd = %d", head.uid, conn->get_fd());
        return ;
    }
    logger.LOG(DETAIL, "uid = %d, fd = %d", head.uid, conn->get_fd());
    server.uid_to_conn[head.uid]->send(packMessage(MessageID::RSP_PLAYER_MATCH, head.uid));


    server.match_uids.insert(head.uid);
    if(server.match_uids.size() >= MATCH_PLAYER_NUM){
        InfPlayerMatchSuccess inf;
        inf.set_gameid(server.gameid_gen);

        // TODO: clear old player state
        // uid_to_gameid

        // all matched players.
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

        // gen a table.
        Game* game = new Game(vuids);
        game->logical->init(MATCH_PLAYER_NUM, current_timestamp_ms, GAME_MODE_MATCH);
        game->logical->start();
        game->logical->set_server(game);

        // modify server state.
        for(int uid : vuids){
            server.uid_to_gameid[uid] = server.gameid_gen;
        }
        server.gameid_to_game[server.gameid_gen] = game;
        server.gameid_gen ++;
    }
}

void onReqPlayerCancelMatch(const TcpConnection* conn, Head& head, const char* msg, int len){
    // TODO: clear old player state
}