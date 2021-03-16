#include <utility>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>

#include "../easy-muduo/common/EventLoop.h"
#include "../easy-muduo/common/TcpClient.h"
#include "../easy-muduo/common/TcpConnection.h"

#include "protocal/tetris.pb.h"

#include "../common/head.h"
#include "../common/util.h"

extern int MAX_BUFFER_SIZE;
const char* ip = "127.0.0.1";
const int listen_port = 9090;
const int loop_time_out_ms = 15;
bool quit = false;
std::map<int, std::function<void(const TcpConnection* , Head& , const char*, int) > > callbacks;
Logger logger(DETAIL, "client_log");

int decodeMessage(const TcpConnection* conn, const char* msg, int len){
    if(len < 4) return 0;
    auto pkgSize = ntohl(*(int32_t *) msg);
    if(pkgSize >= MAX_BUFFER_SIZE || pkgSize < 0){
        return -1;
    }
    return pkgSize;
}

void onConnection(const TcpConnection* conn)
{
    if (conn->connected())
    {
        printf("[fd = %d]\n",  conn->get_fd());
        // connected. do something here
    }
    else
    {
        printf("[fd = %d]\n", conn->get_fd());
        // disconnected. do something here
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len)
{
    auto ptr = msg;
    Head head{ntohl(*(uint32_t *) ptr), ntohl(*(uint32_t *) (ptr+4)), ntohl(*(uint32_t *) (ptr+8))};
    logger.log(DETAIL,"[head = %s]\n", head.to_string());
    callbacks[head.msg_id](conn, head, msg, len);
}

int main(int argc, char* argv[])
{
    EventLoop loop;
    loop.set_logger(&logger);

    //1. connecting
    TcpClient client(&loop, ip, listen_port);
    client.setConnectionCallback(std::bind(&onConnection,  _1));
    client.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    client.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2, _3));
    //client.setWriteCompleteCallback(std::bind(&onWriteComplete, this, _1));
    client.connect();
    do{
        loop.update(loop_time_out_ms);
    }while(!quit);
}

