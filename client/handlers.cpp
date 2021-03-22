//
// Created by jackyan on 2021/3/22.
//
#include "../easy-muduo/common/EventLoop.h"
#include "../easy-muduo/common/TcpClient.h"
#include "../easy-muduo/common/TcpConnection.h"

#include "../common/head.h"
#include "../common/util.h"
#include "../common/common_define.h"

#include "protocal/tetris.pb.h"

#include "../logical.h"
#include "client.h"

extern Logger logger;
extern LocalPlayer player;

void onRspLogin(const TcpConnection* conn, Head& head, const char* msg, int len){
    RspPlayerLogin rsp;
    rsp.ParseFromArray(msg+head.size(), len-head.size());
    player.login = 1;
    player.player_uid = rsp.uid();
}
void onRspPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len){
    player.player_state = CLIENT_PLAYER_STATE_MATCHING;
}
void onInfPlayerMatchSuccess(const TcpConnection* conn, Head& head, const char* msg, int len){
    InfPlayerMatchSuccess inf;
    inf.ParseFromArray(msg+head.size(), len-head.size());
    std::vector<int> uids;
    for(int i = 0; i < inf.uids_size(); i ++){
        uids.push_back(inf.uids(i));
    }
    player.player_state = CLIENT_PLAYER_STATE_MATCH_GAME;
    player.game_uids = uids;
    player.gameid = inf.gameid();
    player.logical.init(uids.size(), 0, GAME_MODE_MATCH);
    player.logical.start();
}
void RspGamePlayerLeave(const TcpConnection* conn, Head& head, const char* msg, int len){
    player.player_state = CLIENT_PLAYER_STATE_IDLE;
}
void onInfGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len){
    InfGamePlayerOp inf;
    inf.ParseFromArray(msg+head.size(), len-head.size());
    if(inf.gameid() != player.gameid){
        logger.LOG(WARNING, "local gameid = %d, remote gameid = %d", player.gameid, inf.gameid());
        return ;
    }
    logger.LOG(DETAIL, "InfGamePlayerOp gameid = %d, remote gameid = %d, op size=%d", player.gameid, inf.gameid(),inf.ops_size());
    for(int j = 0; j < inf.ops_size(); j ++) {
        auto& op = inf.ops(j);
        int index = index_of(player.game_uids.begin(), player.game_uids.end(), op.uid());
        if(index == -1){
            logger.LOG(WARNING, "no such uid, local gameid = %d, remote gameid = %d", player.gameid, inf.gameid());
            continue;
        }
        player.logical.input(index, op.op());
    }
}