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
std::string player_name;
int player_uid;
int player_state;
char ch;
Logical logical;

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
    if (conn->connected())
    {
        logger.log(DETAIL,"connect [fd = %d]\n",  conn->get_fd());

    }
    else
    {
        logger.log(DETAIL, "disconnect [fd = %d]\n", conn->get_fd());
    }
}

void onMessage(const TcpConnection* conn, const char* msg, int len)
{
    auto ptr = msg;
    Head head{ntohl(*(uint32_t *) ptr), ntohl(*(uint32_t *) (ptr+4)), ntohl(*(uint32_t *) (ptr+8))};
    logger.log(DETAIL,"[head = %s]\n", head.to_string());
    callbacks[head.msg_id](conn, head, msg, len);
}


void init_input(){
    printf("\n\n\t\t\t\t\t type your name > ");
    fflush(stdout);
    while(player_name.length() == 0){
        std::cin >> player_name;
        if(player_name.length() == 0 || player_name.length() > MAX_PLAYER_NAME_LENGTH){
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
        case CLIENT_PLAYER_STATE_MATCH_GAME:
            break;
        case CLIENT_PLAYER_STATE_MATCHING:
            break;
    }
}



int main(int argc, char* argv[])
{
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

