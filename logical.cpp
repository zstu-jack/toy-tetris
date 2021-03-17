#include <curses.h>
#include <ncurses.h>
#include <unistd.h>
#include <cctype>
#include <ctime>
#include "common/util.h"
#include "logical.h"

Logical::Logical(int number_player, int time_ms){
    assert(number_player <= MAX_PLAYER);
    for(int i = 0; i < RANDOM_BLOCK; i ++){
        block_down_types.push_back(get_random(0, BLOCK_TYPE));
    }
    for(int i = 0; i < number_player; i ++){
        block_down_index[i] = 0;
        block_down_state[i].x = BLOCK_BORN_N;
        block_down_state[i].y = BLOCK_BORN_M;
        block_down_state[i].type = block_down_types[block_down_index[i] ++];
        block_down_state[i].shape = get_random(0,INT_MAX) % BLOCK_SHAPE_NUM[block_down_state[i].type];
        block_down_last_time_ms[i] = time_ms;
        player_state[i] = ALIVE;
    }
}

int Logical::alive_player(){
    int count = 0;
    for(int i = 0; i < number_player; i ++){
        if(player_state[i] == ALIVE){
            ++ count;
        }
    }
    return count;
}
bool Logical::check(int player_uid, Block block){
    for(int i = 0; i < 16; i ++){
        int offx = i / 4;
        int offy = i % 4;
        int has = BLOCK_SHAPE[block.type][block.shape][i];
        if(has){
            if(offx + block.x > N) return false;
            if(offx + block.x < 0) return false;
            if(offy + block.y < 0) return false;
            if(offy + block.y > M) return false;
            if(map[player_uid].map[offx + block.x][offy + block.y] != MAP_EMPTY) return false;
        }
    }
    return true;
}

void Logical::block_born(int player_uid){
    Block born_block;
    born_block.x = BLOCK_BORN_N;
    born_block.y = BLOCK_BORN_M;
    if(block_down_index[player_uid] >= RANDOM_BLOCK){
        block_down_index[player_uid] = 0;
    }
    born_block.type = block_down_types[block_down_index[player_uid] ++];
    born_block.shape = get_random(0,INT_MAX) % BLOCK_SHAPE_NUM[born_block.type];
    if(!check(player_uid, born_block)){
        player_state[player_uid] = DEATH;
    }else{
        for(int i = 0; i < 16; i ++){
            int offx = i / 4;
            int offy = i % 4;
            auto& block = block_down_state[player_uid];
            int has = BLOCK_SHAPE[block.type][block.shape][i];
            if(has){
                map[player_uid].map[offx + block.x][offy + block.y] = MAP_BLOCK;
            }
        }
        block_down_state[player_uid] = born_block;
    }
}

void Logical::input(int player_uid, int type){
    Block nxt_block = block_down_state[player_uid];
    switch (type) {
        case LEFT:
            nxt_block.y -= 1;
            if(check(player_uid, nxt_block)){
                block_down_state[player_uid] = nxt_block;
            }
            break;
        case RIGHT:
            nxt_block.y += 1;
            if(check(player_uid, nxt_block)){
                block_down_state[player_uid] = nxt_block;
            }
            break;
        case UP:
            nxt_block.shape = (nxt_block.shape + 1) % BLOCK_SHAPE_NUM[nxt_block.type];
            if(check(player_uid, nxt_block)){
                block_down_state[player_uid] = nxt_block;
            }
            break;
        case STRONG_DOWN:
            for(int i = 1; ; i ++){
                nxt_block.x ++;
                if(check(player_uid, nxt_block)){
                    block_down_state[player_uid] = nxt_block;
                }else{
                    break;
                }
            }
            block_born(player_uid);
            break;
        case DOWN:
            nxt_block.x += 1;
            if(check(player_uid, nxt_block)){
                block_down_state[player_uid] = nxt_block;
            }
            break;
        case SYS_DOWN:
            nxt_block.x += 1;
            if(check(player_uid, nxt_block)){
                block_down_state[player_uid] = nxt_block;
            }else{
                block_born(player_uid);
            }
            break;
    }
}
void Logical::print(){

}
void Logical::update(int time_ms){
    for(int i = 0; i < number_player; i ++){
        if(player_state[i] != ALIVE){
            continue;
        }
        if(time_ms - block_down_last_time_ms[i] > BLOCK_DOWN_INTERVAL_MS){
            input(i, SYS_DOWN);
            block_down_last_time_ms[i] = time_ms;
        }
    }
}



//void update(int size){
//    clear();
//    for(int i = 0; i < size; i ++){
//        for(int j = 0; j < size; j ++){
//            printw("[]");
//        }
//        printw("\n");
//    }
//    refresh();
//}
//
//
//int main()
//{
//    initscr();
//    nodelay(stdscr, true);
//    noecho();
//
//    std::time_t last_t = std::time(0);
//    int count = 0;
//    char ch;
//    for(;;){
//        std::time_t t = std::time(0);
//        if(t - last_t > 1){
//            update(++ count);
//            last_t = t;
//        }
//        ch = getch();
//        if (ch >= 0x20 && ch <= 0x7F){
//            printw("|%c,%x|\n", ch,(int)ch);
//            if (toupper(ch) == 'Q') {
//                break;
//                printw("break.......\n");
//            }
//        }
//        refresh();
//    }
//    endwin();
//    return 0;
//}