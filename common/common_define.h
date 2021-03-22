//
// Created by jackyan on 2021/3/18.
//

#ifndef COMMON_DEFINE_H
#define COMMON_DEFINE_H

#include <map>

enum COMMON_DEFINE{
    MAX_PLAYER_NAME_LENGTH = 10,
};

////////// Net Related ////////
enum COMMON_SOCKET_DEFINE{
    SOCKET_APP_MAX_BUFFER_SIZE = 10240,
    SOCKET_BACKLOG_SIZE = 1024,
};

enum{
    CLIENT_LOOP_TIMEOUT = 4,
    SERVER_LOOP_TIMEOUT = 1,
};

extern const char* SOCKET_IP;
extern int SOCKET_PORT;

////////// PLAYER Related //////
enum CLIENT_PLAYER_STATE{
    CLIENT_PLAYER_STATE_IDLE = 0,
    CLIENT_PLAYER_STATE_MATCHING = 1,
    CLIENT_PLAYER_STATE_SINGLE_GAME = 2,
    CLIENT_PLAYER_STATE_SINGLE_DEATH = 3,
    CLIENT_PLAYER_STATE_MATCH_GAME = 4,
    CLIENT_PLAYER_STATE_MATCH_GAME_DEAD = 5,
};
enum PLAYER_GAME_STATE{
    PLAYER_GAME_ALIVE = 0,
    PLAYER_GAME_DEATH = 1,
};

///////// Game Related ///////

enum MAP_STATE{
    MAP_SIZE_N = 20,   // [0,20]
    MAP_SIZE_M = 11,   // [0,11]
    MAP_BLOCK_EMPTY = 0,
    MAP_BLOCK_NOT_EMPTY = 1,
    MAP_BLOCK_BORN_N = 0,  // [0, 5]
    MAP_BLOCK_BORN_M = 5,  // [0, 5]
};
enum GAME_BASIC_SETTING{
    BLOCK_TOTAL_TYPE = 7,
    MAX_PLAYER = 5,
    MATCH_PLAYER_NUM = 2,
    BLOCK_DOWN_INTERVAL_MS = 500,
    BLOCK_DOWN_RANDOM_MAX_SIZE = 4000,
};
enum GAME_MODE{
    GAME_MODE_SINGLE = 0,
    GAME_MODE_MATCH = 1,
};
enum GAME_OP_TYPE{
    GAME_OP_STRONG_DOWN = 0,
    GAME_OP_DOWN = 1,
    GAME_OP_UP = 2,
    GAME_OP_LEFT = 3,
    GAME_OP_RIGHT = 4,
    GAME_OP_SYS_DOWN = 5,
};
extern const int BLOCK_SHAPE_NUM[BLOCK_TOTAL_TYPE];
extern const int BLOCK_SHAPE[BLOCK_TOTAL_TYPE][4][16];

extern const std::map<int, int> key_to_op_type;

///////// Graphics //////////
extern const std::pair<int, int> SHELL_BLOCK_POSITION[MAX_PLAYER-1];
extern const std::pair<int, int> SHELL_BLOCK_SELF_POSITION;


#endif //COMMON_DEFINE_H
