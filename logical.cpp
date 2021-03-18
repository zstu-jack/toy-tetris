#include <curses.h>
#include <ncurses.h>
#include <unistd.h>
#include <cctype>
#include <ctime>
#include "common/util.h"
#include "logical.h"

Logical::Logical(){

};

void Logical::init(int number_player, int time_ms, int game_mode, int you){
    assert(number_player <= MAX_PLAYER);
    block_down_types.clear();
    this->number_player = number_player;
    this->game_mode = game_mode;
    this->you = you;
    for(int i = 0; i < number_player; i ++){
        map[i].init();
    }
    block_down_types.push_back(get_random(0, 1000007) % BLOCK_TYPE);
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
bool Logical::check(int player_index, Block block){

    for(int i = 0; i < 16; i ++){
        int offx = i / 4;
        int offy = i % 4;
        int has = BLOCK_SHAPE[block.type][block.shape][i];
        if(has){
            if(offx + block.x >= N) return false;
            if(offx + block.x < 0) return false;
            if(offy + block.y < 0) return false;
            if(offy + block.y >= M) return false;
            if(map[player_index].map[offx + block.x][offy + block.y] != MAP_EMPTY) return false;
        }
    }
    return true;
}

void Logical::eliminate_lines(int player_index){
    std::vector<int> uneliminates_lines = {};
    for(int i = N - 1; i >= 0; i --){
        bool all = 1;
        for(int j = 0; j < M; j ++){
            if(map[player_index].map[i][j] == MAP_EMPTY) all = 0;
        }
        if(!all) {
            uneliminates_lines.push_back(i);
        }
    }
    int put_row = N;
    for(int move_row : uneliminates_lines){
        memmove(map[player_index].map[-- put_row],map[player_index].map[move_row], sizeof(map[player_index].map[move_row]));
    }
    for(int i = 0; i < put_row; i ++){
        for(int j = 0; j < M; ++ j){
            map[player_index].map[i][j] = MAP_EMPTY;
        }
    }

}

void Logical::block_born(int player_index){

    for(int i = 0; i < 16; i ++){
        int offx = i / 4;
        int offy = i % 4;
        auto& block = block_down_state[player_index];
        int has = BLOCK_SHAPE[block.type][block.shape][i];
        if(has){
            map[player_index].map[offx + block.x][offy + block.y] = MAP_BLOCK;
        }
    }

    eliminate_lines(player_index);

    Block born_block;
    born_block.x = BLOCK_BORN_N;
    born_block.y = BLOCK_BORN_M;
    if(block_down_index[player_index] >= RANDOM_BLOCK){
        block_down_index[player_index] = 0;
    }
    if(block_down_types.size() <= block_down_index[player_index]){
        block_down_types.push_back(get_random(0, 1000007) % BLOCK_TYPE);
    }
    born_block.type = block_down_types[block_down_index[player_index] ++];
    born_block.shape = get_random(0, 100007) % BLOCK_SHAPE_NUM[born_block.type];
    if(!check(player_index, born_block)){
        player_state[player_index] = DEATH;
    }else{
        block_down_state[player_index] = born_block;
    }
}

void Logical::input(int player_index, int op){
    if(run == 0){
        return ;
    }
    if(player_state[player_index] == DEATH){
        return ;
    }

    Block nxt_block = block_down_state[player_index];
    switch (op) {
        case LEFT:
            nxt_block.y -= 1;
            if(check(player_index, nxt_block)){
                block_down_state[player_index] = nxt_block;
            }
            break;
        case RIGHT:
            nxt_block.y += 1;
            if(check(player_index, nxt_block)){
                block_down_state[player_index] = nxt_block;
            }
            break;
        case UP:
            nxt_block.shape = (nxt_block.shape + 1) % BLOCK_SHAPE_NUM[nxt_block.type];
            if(check(player_index, nxt_block)){
                block_down_state[player_index] = nxt_block;
            }
            break;
        case STRONG_DOWN:
            for(int i = 1; ; i ++){
                nxt_block.x += 1;
                if(check(player_index, nxt_block)){
                    block_down_state[player_index] = nxt_block;
                }else{
                    break;
                }
            }
            block_born(player_index);
            break;
        case DOWN:
            nxt_block.x += 1;
            if(check(player_index, nxt_block)){
                block_down_state[player_index] = nxt_block;
            }
            break;
        case SYS_DOWN:
            nxt_block.x += 1;
            if(check(player_index, nxt_block)){
                block_down_state[player_index] = nxt_block;
            }else{
                block_born(player_index);
            }
            break;
    }
}

void Logical::print_mode_single(int player_index){
    if (game_mode != MODE_SINGLE){
        return;
    }

    auto n_char = [](int n, char c){
        for(int i = 0; i < n; ++ i) printw("%c", c);
    };

    n_char(8, '\n');
    // n_char(5, '\t');  n_char(2 * M +2, '-'); n_char(1, '\n');
    n_char(5, '\t');  n_char(2 * M +2, '-'); n_char(1, '\n');
    for(int i = 0; i < N; i ++){
        n_char(5, '\t'); n_char(1, '|');
        for(int j = 0; j < M; j ++){
            printw("%s", map[player_index].map[i][j] ? "[]" : "  ");
        }
        n_char(1, '|');
        n_char(1, '\n');
    }
    n_char(5, '\t');  n_char(2 * M +2, '-');
}
void Logical::print_mode_match(){
    if(game_mode != MODE_MATCH){
        return ;
    }
}

void Logical::set_block_print(int grid_shape){
    for(int player_index = 0; player_index < number_player; player_index ++) {
        for (int i = 0; i < 16; i++) {
            int offx = i / 4;
            int offy = i % 4;
            auto &block = block_down_state[player_index];
            int has = BLOCK_SHAPE[block.type][block.shape][i];
            if (has) {
                map[player_index].map[offx + block.x][offy + block.y] = grid_shape;
            }
        }
    }
}

void Logical::print(){
    if(run == 0){
        return ;
    }
    if(alive_player() == 0){
        return ;
    }
    clear();
    set_block_print(MAP_BLOCK);
    print_mode_single(0);
    print_mode_match();
    set_block_print(MAP_EMPTY);
    refresh();
}
void Logical::update(int time_ms){
    if(run == 0){
        return ;
    }
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

void Logical::start() {
    run = 1;
}
void Logical::stop() {
    run = 0;
}

