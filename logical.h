//
// Created by jackyan on 2021/3/12.
//

#ifndef LOGICAL_H
#define LOGICAL_H

#include <map>
#include "common/common_define.h"

// map: [0, N-1], [0, M-1]
template<int MAP_SIZE_N, int MAP_SIZE_M>
struct Map{
    int map[MAP_SIZE_N][MAP_SIZE_M];
    Map(){}
    void init(){
        for(int i = 0; i < MAP_SIZE_N; ++ i){
            for(int j = 0; j < MAP_SIZE_M; ++ j){
                map[i][j] = MAP_BLOCK_EMPTY;
            }
        }
    }
};

struct Block{
    int x, y, type,shape;
};

class Logical{
public:
    Map<MAP_SIZE_N, MAP_SIZE_M> map[MAX_PLAYER];
    std::vector<int> block_down_types;
    int block_down_index[MAX_PLAYER];   // next down block type
    int block_down_last_time_ms[MAX_PLAYER];
    Block block_down_state[MAX_PLAYER];

    int alive_player();
    int player_state[MAX_PLAYER];
    int number_player;
public:
    int game_mode;
    int run;

public:
    Logical();

    void eliminate_lines(int player_index);
    void set_map_block(int player_index, int map_block);
    void set_map_block_for_all_player(int map_block);
    void block_born_gen(int player_index);
    void block_born(int player_index);
    bool check(int player_index, Block block);

    void init(int number_player, int time_ms, int game_mode);
    void input(int player_index, int type);
    void update(int time_ms);
    void print_mode_single(int);
    void print_mode_match();
    void print(); // client side.

    void start();
    void stop();

};

#endif //LOGICAL_H
