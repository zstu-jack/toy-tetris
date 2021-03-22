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
#include "client.h"
#include "handlers.h"

long long current_timestamp_ms = 0;
std::map<int, std::function<void(const TcpConnection* , Head& , const char*, int) > > callbacks;
Logger logger(DETAIL, "client_log");

//////
char ch;
LocalPlayer player;

void onConnection(const TcpConnection* conn)
{
    player.conn = const_cast<TcpConnection*>(conn);
    if (conn->connected())
    {
        player.player_state = CLIENT_PLAYER_STATE_IDLE;
        logger.LOG(DETAIL,"connect [fd = %d]",  conn->get_fd());
        ReqPlayerLogin req;
        req.set_name(player.player_name);
        player.conn->send(packMessage(MessageID::REQ_LOGIN,-1, req));
    }
    else
    {
        logger.LOG(DETAIL, "disconnect [fd = %d]", conn->get_fd());
        player.login = 0;
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
    player.player_state = CLIENT_PLAYER_STATE_IDLE;

    initscr();
    nodelay(stdscr, true);   // make getch() non-block call.
    noecho();
    curs_set(0);
    clear();
}

void print_screen(){
    clear();
    ch = getch();
    switch (player.player_state) {
        case CLIENT_PLAYER_STATE_IDLE:
            printw(" \n\n single mode, input `s`\n\n match mode, input `m` \n\n> ");
            if (toupper(ch) == 'S'){
                player.logical.init(1, get_1970_ms(), GAME_MODE_SINGLE);
                player.logical.start();
                player.player_state = CLIENT_PLAYER_STATE_SINGLE_GAME;
            }else if(toupper(ch) == 'M'){
                if(!player.login){
                    break;
                }
                ReqPlayerMatch req;
                req.set_player_number(MATCH_PLAYER_NUM);
                player.conn->send(packMessage(MessageID::REQ_PLAYER_MATCH, player.player_uid, req));
            }
            break;
        case CLIENT_PLAYER_STATE_SINGLE_GAME:
            if(key_to_op_type.count((int)ch)){
                player.logical.input(0, key_to_op_type.find(ch)->second);
            }
            player.logical.update(current_timestamp_ms);
            player.logical.print();
            if(player.logical.alive_player() == 0){
                printw("\n\n dead, input `q` to quit\n\n > ");
                do{ ch = getch(); } while (toupper(ch) != 'Q');
                player.player_state = CLIENT_PLAYER_STATE_IDLE;
            }
            break;
        case CLIENT_PLAYER_STATE_MATCHING:
            printw("\n\n matching...... input `q` to quit\n\n > ");
            if(toupper(ch) == 'Q'){
                player.conn->send(packMessage(MessageID::REQ_PLAYER_CANCEL_MATCH, player.player_uid));
            }
            break;
        case CLIENT_PLAYER_STATE_MATCH_GAME:
            if(key_to_op_type.count((int)ch)){
                ReqGamePlayerOp req;
                req.set_op((int)key_to_op_type.find(ch)->second);
                player.conn->send(packMessage(MessageID::REQ_GAME_PLAYER_OP, player.player_uid, req));
            }
            player.logical.print();
            if(!player.logical.is_alive(index_of(player.game_uids.begin(), player.game_uids.end(), player.player_uid))){
                player.player_state = CLIENT_PLAYER_STATE_MATCH_GAME_DEAD;
            }
            break;
        case CLIENT_PLAYER_STATE_MATCH_GAME_DEAD:
            player.logical.print();
            printw("\n\n dead, input `q` to quit\n\n > ");
            if(toupper(ch) == 'Q') {
                player.conn->send(packMessage(MessageID::REQ_GAME_PLAYER_LEAVE, player.player_uid));
            }
            break;
    }
    refresh();
}


int main(int argc, char* argv[])
{
    {
        using namespace std::placeholders;
        callbacks[(int) MessageID::RSP_LOGIN] = std::bind(onRspLogin, _1, _2, _3, _4);
        callbacks[(int) MessageID::RSP_PLAYER_MATCH] = std::bind(onRspPlayerMatch, _1, _2, _3, _4);
        callbacks[(int) MessageID::INF_PLAYER_MATCH_SUCCESS] = std::bind(onInfPlayerMatchSuccess, _1, _2, _3, _4);
        callbacks[(int) MessageID::RSP_GAME_PLAYER_LEAVE] = std::bind(RspGamePlayerLeave, _1, _2, _3, _4);
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

