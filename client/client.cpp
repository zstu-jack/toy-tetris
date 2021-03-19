#include <utility>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <string>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <curses.h>
#include <ncurses.h>
#include <iostream>

#include "../easy-muduo/common/EventLoop.h"
#include "../easy-muduo/common/TcpClient.h"
#include "../easy-muduo/common/TcpConnection.h"

#include "protocal/tetris.pb.h"

#include "../common/head.h"
#include "../common/util.h"
#include "../common/common_define.h"

#include "../logical.h"

long long current_timestamp_ms = 0;
std::map<int, std::function<void(const TcpConnection* , Head& , const char*, int) > > callbacks;
Logger logger(DETAIL, "client_log");

//////
char ch;
Logical logical;
int player_state;
struct LocalPlayer {
    TcpConnection* conn;
    int connected;
    std::string player_name;
    int player_uid;
};
LocalPlayer player;
int gameid;
std::vector<int> game_uins;

int decodeMessage(const TcpConnection* conn, const char* msg, int len){
    if(len < 4) return 0;
    auto pkgSize = ntohl(*(int32_t *) msg);
    if(pkgSize >= SOCKET_APP_MAX_BUFFER_SIZE || pkgSize < 0){
        return -1;
    }
    return pkgSize;
}

void onConnection(const TcpConnection* conn)
{
    player.conn = const_cast<TcpConnection*>(conn);
    if (conn->connected())
    {
        player_state = CLIENT_PLAYER_STATE_IDLE;
        logger.LOG(DETAIL,"connect [fd = %d]\n",  conn->get_fd());
        ReqPlayerLogin req;
        req.set_name(player.player_name);
        player.conn->send(packMessage(MessageID::REQ_LOGIN,-1, req));
    }
    else
    {
        logger.LOG(DETAIL, "disconnect [fd = %d]\n", conn->get_fd());
        player.connected = 0;
        player.player_uid = -1;
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len)
{
    auto ptr = msg;
    Head head{ntohl(*(uint32_t *) ptr), ntohl(*(uint32_t *) (ptr+4)), ntohl(*(uint32_t *) (ptr+8))};
    logger.log(DETAIL,"[head = %s]\n", head.to_string().c_str());
    callbacks[head.msg_id](conn, head, msg, len);
}


void init_input(){
    printf("\n\n\t\t\t\t\t type your name > ");
    fflush(stdout);
    while(player.player_name.length() == 0){
        std::cin >> player.player_name;
        if(player.player_name.length() == 0 || player.player_name.length() > MAX_PLAYER_NAME_LENGTH){
            printf("\n\n\t\t\t\t\tname can't be empty\n\n\t\t\t\t\ttype your name > ");
            fflush(stdout);
        }
    }
    player_state = CLIENT_PLAYER_STATE_IDLE;

    initscr();
    nodelay(stdscr, true);   // make getch() non-block call.
    noecho();
    clear();
}

void print_screen(){
    switch (player_state) {
        case CLIENT_PLAYER_STATE_IDLE:
            clear();
            printw(" \n\n single mode, input `s`\n\n match mode, input `m` \n\n> ");
            ch = getch();
            if (toupper(ch) == 'S'){
                logical.init(1, get_1970_ms(), GAME_MODE_SINGLE);
                logical.start();
                player_state = CLIENT_PLAYER_STATE_SINGLE_GAME;
            }else if(toupper(ch) == 'M'){
                if(player.connected){
                    ReqPlayerMatch req;
                    req.set_player_number(MATCH_PLAYER_NUM);
                    player.conn->send(packMessage(MessageID::REQ_PLAYER_MATCH, player.player_uid, req));
                }
            }
            break;
        case CLIENT_PLAYER_STATE_SINGLE_GAME:
            ch = getch();
            if(key_to_op_type.count((int)ch)){
                logical.input(0, key_to_op_type.find(ch)->second);
            }
            logical.update(current_timestamp_ms);
            logical.print();
            if(logical.alive_player() == 0){
                printw("\n\n dead, input `q` to quit\n\n > ");
                do{ ch = getch(); } while (toupper(ch) != 'Q');
                player_state = CLIENT_PLAYER_STATE_IDLE;
            }
            break;
        case CLIENT_PLAYER_STATE_MATCHING:
            clear();
            printw("\n\n matching...... input `q` to quit\n\n > ");
            ch = getch();
            if(toupper(ch) == 'Q'){
                // TODO: quit match
            }
            refresh();
        case CLIENT_PLAYER_STATE_MATCH_GAME:
            ch = getch();
            if(key_to_op_type.count((int)ch)){
                ReqGamePlayerOp req;
                req.set_op((int)key_to_op_type.find(ch)->second);
                player.conn->send(packMessage(MessageID::REQ_GAME_PLAYER_OP, player.player_uid, req));
            }
            logical.print();
            break;
        case CLIENT_PLAYER_STATE_MATCH_GAME_DEAD:
            logical.print();
            printw("\n\n dead, input `q` to quit\n\n > ");
            ch = getch();
            if(toupper(ch) != 'Q') {
                // TODO: notify server.
                player_state = CLIENT_PLAYER_STATE_IDLE;
            }
            break;
    }
}

//////////// net /////////////
void onRspLogin(const TcpConnection* conn, Head& head, const char* msg, int len){
    RspPlayerLogin rsp;
    rsp.ParseFromArray(msg+head.size(), len-head.size());
    player.connected = 1;
    player.player_uid = rsp.uid();
}
void onRspPlayerMatch(const TcpConnection* conn, Head& head, const char* msg, int len){
    player_state = CLIENT_PLAYER_STATE_MATCHING;
}
void onInfPlayerMatchSuccess(const TcpConnection* conn, Head& head, const char* msg, int len){
//    repeated int32 uids = 1;
//    required int32 gameid = 2;
    InfPlayerMatchSuccess inf;
    inf.ParseFromArray(msg+head.size(), len-head.size());
    std::vector<int> uids;
    for(int i = 0; i < inf.uids_size(); i ++){
        uids.push_back(inf.uids(i));
    }
    player_state = CLIENT_PLAYER_STATE_MATCH_GAME;
    game_uins = uids;
    gameid = inf.gameid();
    logical.init(uids.size(), 0, GAME_MODE_MATCH);
    logical.start();
}
void onInfGamePlayerOp(const TcpConnection* conn, Head& head, const char* msg, int len){
    InfGamePlayerOp inf;
    inf.ParseFromArray(msg+head.size(), len-head.size());
    if(inf.gameid() != gameid){
        logger.LOG(WARNING, "local gameid = %d, remote gameid = %d\n", gameid, inf.gameid());
        return ;
    }
    logger.LOG(DETAIL, "InfGamePlayerOp gameid = %d, remote gameid = %d, op size=%d\n", gameid, inf.gameid(),inf.ops_size());
    for(int j = 0; j < inf.ops_size(); j ++) {
        auto& op = inf.ops(j);
        int index = -1;
        for (int i = 0; i < game_uins.size(); i++) {
            if (game_uins[i] == op.uid()){
                index = i;
            }
        }
        if(index == -1){
            logger.LOG(WARNING, "no such uid, local gameid = %d, remote gameid = %d\n", gameid, inf.gameid());
            continue;
        }
        logical.input(index, op.op());
    }

}

int main(int argc, char* argv[])
{
    {
        using namespace std::placeholders;
        callbacks[(int) MessageID::RSP_LOGIN] = std::bind(onRspLogin, _1, _2, _3, _4);
        callbacks[(int) MessageID::RSP_PLAYER_MATCH] = std::bind(onRspPlayerMatch, _1, _2, _3, _4);
        callbacks[(int) MessageID::INF_PLAYER_MATCH_SUCCESS] = std::bind(onInfPlayerMatchSuccess, _1, _2, _3, _4);
        callbacks[(int) MessageID::GAME_INF_PLAYER_OP] = std::bind(onInfGamePlayerOp, _1,_2,_3,_4);
    }

    init_input();
    bool quit = false;

    EventLoop loop;
    loop.set_logger(&logger);

    //1. connecting
    TcpClient client(&loop, SOCKET_IP, SOCKET_PORT);
    client.setConnectionCallback(std::bind(&onConnection,  _1));
    client.setMessageCallback(std::bind(&onMessage, _1, _2, _3));
    client.setPkgDecodeCallback(std::bind(&decodeMessage, _1, _2, _3));
    //client.setWriteCompleteCallback(std::bind(&onWriteComplete, this, _1));
    client.connect();
    do{
        current_timestamp_ms = get_1970_ms();
        print_screen();
        loop.update(CLIENT_LOOP_TIMEOUT);
    }while(!quit);
}

