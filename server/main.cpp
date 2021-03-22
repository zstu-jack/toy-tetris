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
#include "handlers.h"

Logger logger(DETAIL, "server_log");
long long current_timestamp_ms = 0;
Server server;

std::map<int, std::function<void(const TcpConnection* , Head&, const char*, int)> > callbacks;

void onConnection(const TcpConnection* conn){
    // you can echo peer's ip and port here.
    if(conn->connected()){
        logger.LOG(DETAIL,"connected [fd = %d]",  conn->get_fd());
    }else{
        logger.LOG(DETAIL,"disconnected [fd = %d]",  conn->get_fd());
        server.leave_player(conn);

    }
}
void onMessage(const TcpConnection* conn, const char* msg, int len){
    auto ptr = msg;
    Head head{ntohl(*(uint32_t *) ptr), ntohl(*(uint32_t *) (ptr+4)), ntohl(*(uint32_t *) (ptr+8))};
    logger.LOG(DETAIL,"[fd = %d, head = %s]", conn->get_fd(), head.to_string().c_str());
    callbacks[head.msg_id](conn, head, msg, len);
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