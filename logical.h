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

std::vector<std::pair<int, int>> dots_in_axis(int type);

// map: [0, N-1], [0, M-1]
template<int N, int M, char c>
class Map{
public:
    int map[N][M];
public:
    Map();

};

class Logical{
public:
    Map map[MAX_PLAYER];
public:
    void update(){

    }
    void input(){

    }
};

#endif //LOGICAL_H
