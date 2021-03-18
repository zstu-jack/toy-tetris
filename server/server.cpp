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

Logger logger(DETAIL, "server_log");

struct Player{
    int uid;
    int state;
    std::string name;
};

std::map<TcpConnection*, Player*> conn_to_player;
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
        logger.log(DETAIL,"[fd = %d]\n",  conn->get_fd());
    }else{
        logger.log(DETAIL,"[fd = %d]\n",  conn->get_fd());
    }
}
void onMessage(const TcpConnection* conn, const char* msg, int len){
    auto ptr = msg;
    Head head{ntohl(*(uint32_t *) ptr), ntohl(*(uint32_t *) (ptr+4)), ntohl(*(uint32_t *) (ptr+8))};
    logger.log(DETAIL,"[head = %s]\n", head.to_string());
    callbacks[head.msg_id](conn, head, msg, len);
}

void onReqLogin(const TcpConnection* conn, Head& head, const char* msg, int len){

}
void onGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len){
    // req.ParseFromArray(ptr+head.size(), len-head.size());
}
void onReqPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len){

}

int main(){

    {
        using namespace std::placeholders;
        callbacks[(int) MessageID::REQ_LOGIN] = std::bind(onReqLogin, _1, _2, _3, _4);
        callbacks[(int) MessageID::REQ_PLAYER_MATCH] = std::bind(onReqPlayerMatch, _1, _2, _3, _4);
        callbacks[(int) MessageID::GAME_PLAYER_OP] = std::bind(onGamePlayerOp, _1,_2,_3,_4);
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
        loop.update(SERVER_LOOP_TIMEOUT);
    }while(!quit);
    return 0;
}