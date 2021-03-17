//
// Created by jackyan on 2021/3/12.
//

#ifndef LOGICAL_H
#define LOGICAL_H

#include <map>

const int N = 20;   // [0,20]
const int M = 11;   // [0,11]
const int MAX_PLAYER = 5;
const int BLOCK_BORN_N = 0;  // [0, 5]
const int BLOCK_BORN_M = 5;  // [0, 5]
const int BLOCK_DOWN_INTERVAL_MS = 500;
const int BLOCK_TYPE = 7;
const int RANDOM_BLOCK = 4000;

enum MAP_STATE{
    MAP_EMPTY = 0,
    MAP_BLOCK = 1,
};

enum OP_TYPE{
    STRONG_DOWN = 0,
    DOWN = 1,
    UP = 2,
    LEFT = 3,
    RIGHT = 4,
    SYS_DOWN = 5,
};
const std::map<int, int> key_to_op_type = {
        {65,UP},
        {66,DOWN},
        {68,LEFT},
        {67,RIGHT},
        {32,STRONG_DOWN},
};

enum PLAYER_GAME_STATE{
    ALIVE = 0,
    DEATH = 1,
};

enum GAME_MODE{
    MODE_SINGLE = 0,
    MODE_MATCH = 1,
};

const int BLOCK_SHAPE_NUM[BLOCK_TYPE] = {
        2, 1, 2, 2, 4, 4, 4
};
const int BLOCK_SHAPE[BLOCK_TYPE][4][16] = {
        {{0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0},{0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0},{},{}},
        {{0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0},{},{},{}},
        {{1,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0},{0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0},{},{}},
        {{0,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0},{1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0},{},{}},
        {{0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0},{0,1,0,0,0,1,1,0,1,0,0,0,0,0,0,0},{0,0,0,0,1,1,1,0,0,1,0,0,0,0,0,0},{0,1,0,0,1,1,0,0,0,1,0,0,0,0,0,0}},
        {{0,1,0,0,0,1,0,0,1,1,0,0,0,0,0,0},{1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0},{0,1,1,0,0,1,0,0,0,1,0,0,0,0,0,0},{1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,0}},
        {{1,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0},{1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0},{0,1,1,0,0,0,1,0,0,0,1,0,0,0,0,0},{0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0}}
};


const std::pair<int, int> BLOCK_POSITION[MAX_PLAYER-1] = {
        {10, 10},
        {10, 50},
        {50, 10},
        {50, 50},
};
const std::pair<int, int> BLOCK_SELF_POSITION = {10, 110};


// map: [0, N-1], [0, M-1]
template<int N, int M, char c>
class Map{
public:
    int map[N][M];
public:
    Map(){}

    void init(){
        for(int i = 0; i < N; ++ i){
            for(int j = 0; j < M; ++ j){
                map[i][j] = MAP_EMPTY;
            }
        }
    }
};

struct Block{
    int x, y, type,shape;
};

class Logical{
public:
    Map<N, M, '*'> map[MAX_PLAYER];
    std::vector<int> block_down_types;
    int block_down_index[MAX_PLAYER];   // next down block type
    int block_down_last_time_ms[MAX_PLAYER];
    Block block_down_state[MAX_PLAYER];

    int player_state[MAX_PLAYER];
    int alive_player();
    int number_player;
    std::map<int, int> uin_to_index;
public:
    int game_mode;
    int run;

public:
    Logical();
    void block_born(int player_index);
    bool check(int player_index, Block block);

    void init(int number_player, int time_ms, int game_mode, int you = 0);
    void input(int player_index, int type);
    void update(int time_ms);
    void print(); // client side.

    void start();
    void stop();
    int you;

};

#endif //LOGICAL_H
