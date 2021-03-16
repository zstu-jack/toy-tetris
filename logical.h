//
// Created by jackyan on 2021/3/12.
//

#ifndef LOGICAL_H
#define LOGICAL_H

const int N = 21;
const int M = 12;
const int MAX_PLAYER = 2;
const int BLOCK_BORN_N = 0;  // [0, 5]
const int BLOCK_BORN_M = 5;  // [0, 5]
const int BLOCK_DOWN_INTERVAL_MS = 500;
const int BLOCK_TYPE = 7;
const int MAP_EMPTY = 0;
const int MAP_BLOCK = 1;
const int RANDOM_BLOCK = 4000;

std::vector<std::pair<int, int>> dots_in_axis(int type);

// map: [0, N-1], [0, M-1]
template<int N, int M, char c>
class Map{
public:
    int map[N][M];
public:
    Map(){
        for(int i = 0; i < N; ++ i){
            for(int j = 0; j < M; ++ j){
                map[i][j] = MAP_EMPTY;
            }
        }
    }

};

class Logical{
public:
    Map<N, M, '*'> map[MAX_PLAYER];
    std::vector<int> block_down_types;
    int block_down_index[MAX_PLAYER];
    int alive_player;
public:
    Logical();
    // serve side.
    void input();
    // client side.
    void print();
    void player_op();
};

#endif //LOGICAL_H
