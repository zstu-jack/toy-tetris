#include <curses.h>
#include <ncurses.h>
#include <unistd.h>
#include <cctype>
#include <ctime>
#include <numeric>
#include "common/util.h"
#include "common/common_define.h"
#include "logical.h"
#include "server/server.h"

Logical::Logical(){

};

void Logical::init(int number_player, int time_ms, int game_mode){
    assert(number_player <= MAX_PLAYER);
    server_side = false;
    for(int i = 0; i < MAX_PLAYER; i ++) {
        player_state[i] = PLAYER_GAME_DEATH;
    }
    block_down_types.clear();
    this->number_player = number_player;
    this->game_mode = game_mode;
    for(int i = 0; i < number_player; i ++){
        map[i].init();
    }
    block_down_types.push_back(get_random(0, 1000007) % BLOCK_TOTAL_TYPE);
    for(int i = 0; i < number_player; i ++){
        block_down_index[i] = 0;
        block_down_state[i] = {MAP_BLOCK_BORN_N, MAP_BLOCK_BORN_M, block_down_types[block_down_index[i] ++], 0};
        block_down_state[i].shape = get_random(0,1000007) % BLOCK_SHAPE_NUM[block_down_state[i].type];

        block_down_last_time_ms[i] = time_ms;
        player_state[i] = PLAYER_GAME_ALIVE;
    }
}

int Logical::alive_player(){
    return std::count_if(std::begin(player_state), std::end(player_state), [](int state){ return state == PLAYER_GAME_ALIVE;});
}
bool Logical::check(int player_index, Block block){
    for(int i = 0; i < 16; i ++){
        int offx = i / 4, offy = i % 4;
        int has = BLOCK_SHAPE[block.type][block.shape][i];
        if(has){
            if(offx + block.x < 0 || offx + block.x >= MAP_SIZE_N) return false;
            if(offy + block.y < 0 || offy + block.y >= MAP_SIZE_M) return false;
            if(map[player_index].map[offx + block.x][offy + block.y] != MAP_BLOCK_EMPTY) return false;
        }
    }
    return true;
}

int Logical::is_alive(int player_index){
    if(player_index < 0 || player_index > MAX_PLAYER){
        return 0;
    }
    return player_state[player_index] == PLAYER_GAME_ALIVE;
}

void Logical::eliminate_lines(int player_index){
    std::vector<int> uneliminates_lines = {};
    for(int i = MAP_SIZE_N - 1; i >= 0; i --){
        if(std::count_if(std::begin(map[player_index].map[i]), std::end(map[player_index].map[i]),
                         [](int state){ return state == MAP_BLOCK_EMPTY;})){
            uneliminates_lines.push_back(i);
        }
    }
    int put_row = MAP_SIZE_N;
    for(int move_row : uneliminates_lines){
        memmove(map[player_index].map[-- put_row],map[player_index].map[move_row], sizeof(map[player_index].map[move_row]));
    }
    for(int i = 0; i < put_row; i ++){
        std::fill_n(map[player_index].map[i], MAP_SIZE_M, MAP_BLOCK_EMPTY);
    }
}

void Logical::set_map_block(int player_index, int map_block){
    for (int i = 0; i < 16; i++) {
        int offx = i / 4, offy = i % 4;
        auto &block = block_down_state[player_index];
        int has = BLOCK_SHAPE[block.type][block.shape][i];
        if (has) {
            map[player_index].map[offx + block.x][offy + block.y] = map_block;
        }
    }
}
void Logical::set_map_block_for_all_player(int map_block) {
    for(int player_index = 0; player_index < number_player; player_index ++) {
        set_map_block(player_index, map_block);
    }
}

void Logical::block_born_gen(int player_index){
    if(block_down_index[player_index] >= BLOCK_DOWN_RANDOM_MAX_SIZE){
        block_down_index[player_index] = 0;
    }
    if(block_down_types.size() <= block_down_index[player_index]){
        block_down_types.push_back(get_random(0, 1000007) % BLOCK_TOTAL_TYPE);
    }
}

void Logical::block_born(int player_index){

    set_map_block(player_index, MAP_BLOCK_NOT_EMPTY);
    eliminate_lines(player_index);
    block_born_gen(player_index);
    Block born_block{MAP_BLOCK_BORN_N, MAP_BLOCK_BORN_M, block_down_types[block_down_index[player_index] ++],
                     get_random(0, 100007) % BLOCK_SHAPE_NUM[born_block.type]};
    if(!check(player_index, born_block)){
        player_state[player_index] = PLAYER_GAME_DEATH;
        return ;
    }
    block_down_state[player_index] = born_block;
}

void Logical::print_mode_single(int player_index){
    if (game_mode != GAME_MODE_SINGLE){
        return;
    }

    auto n_char = [](int n, char c){
        for(int i = 0; i < n; ++ i) printw("%c", c);
    };

    n_char(8, '\n');
    // n_char(5, '\t');  n_char(2 * M +2, '-'); n_char(1, '\n');
    n_char(5, '\t');  n_char(2 * MAP_SIZE_M +2, '-'); n_char(1, '\n');
    for(int i = 0; i < MAP_SIZE_N; i ++){
        n_char(5, '\t'); n_char(1, '|');
        for(int j = 0; j < MAP_SIZE_M; j ++){
            printw("%s", map[player_index].map[i][j] ? "[]" : "  ");
        }
        n_char(1, '|');
        n_char(1, '\n');
    }
    n_char(5, '\t');  n_char(2 * MAP_SIZE_M +2, '-');
}
void Logical::print_mode_match(){
    if(game_mode != GAME_MODE_MATCH){
        return ;
    }

    auto n_char = [](int n, char c){
        for(int i = 0; i < n; ++ i) printw("%c", c);
    };

    n_char(8, '\n');
    n_char(5, '\t');
    n_char(2 * MAP_SIZE_M + 2, '-');
    for(int i = 1; i < number_player; i ++) {
        n_char(10, ' ');
        n_char(2 * MAP_SIZE_M + 2, '-');
    }
    n_char(1, '\n');
    for(int i = 0; i < MAP_SIZE_N; i ++){
        n_char(5, '\t');
        n_char(1, '|');
        for (int j = 0; j < MAP_SIZE_M; j++) {
            printw("%s", map[0].map[i][j] ? "[]" : "  ");
        }
        n_char(1, '|');
        for(int pi = 1; pi < number_player; pi ++) {
            n_char(10, ' ');
            n_char(1, '|');
            for (int j = 0; j < MAP_SIZE_M; j++) {
                printw("%s", map[pi].map[i][j] ? "[]" : "  ");
            }
            n_char(1, '|');
        }
        n_char(1, '\n');
    }
    n_char(5, '\t');  n_char(2 * MAP_SIZE_M +2, '-');
    for(int i = 1; i < number_player; i ++) {
        n_char(10, ' ');
        n_char(2 * MAP_SIZE_M + 2, '-');
    }
}

void Logical::print(){
    if(run == 0){
        return ;
    }
    set_map_block_for_all_player(MAP_BLOCK_NOT_EMPTY);
    print_mode_single(0);
    print_mode_match();
    set_map_block_for_all_player(MAP_BLOCK_EMPTY);
}


void Logical::input(int player_index, int op){
    if(run == 0){
        logger.LOG(DETAIL, "just return, not run now");
        return ;
    }
    if(player_state[player_index] == PLAYER_GAME_DEATH){
        logger.LOG(DETAIL, "just return, player death");
        return ;
    }
    logger.LOG(DETAIL, "index(%d) input op(%d)", player_index, op);

#ifdef SERVER_SIDE
    if(server_side){
        server_game->input(player_index, op);
    }
#endif

    Block nxt_block = block_down_state[player_index];
    auto block_state_trans = [&](int player_index, Block& next_block)->bool {
        if(check(player_index, next_block)){
            block_down_state[player_index] = next_block;
            return true;
        }
        return false;
    };
    switch (op) {
        case GAME_OP_LEFT:
            nxt_block.y -= 1;
            block_state_trans(player_index, nxt_block);
            break;
        case GAME_OP_RIGHT:
            nxt_block.y += 1;
            block_state_trans(player_index, nxt_block);
            break;
        case GAME_OP_UP:
            nxt_block.shape = (nxt_block.shape + 1) % BLOCK_SHAPE_NUM[nxt_block.type];
            block_state_trans(player_index, nxt_block);
            break;
        case GAME_OP_DOWN:
            nxt_block.x += 1;
            block_state_trans(player_index, nxt_block);
            break;
        case GAME_OP_STRONG_DOWN:
            for(;block_state_trans(player_index, nxt_block);nxt_block.x += 1);
            block_born(player_index);
            break;
        case GAME_OP_SYS_DOWN:
            nxt_block.x += 1;
            if(!block_state_trans(player_index, nxt_block)){
                block_born(player_index);
            }
            break;
    }
}

void Logical::update(int time_ms){
    if(run == 0){
        return ;
    }
    for(int i = 0; i < number_player; i ++){
        if(player_state[i] != PLAYER_GAME_ALIVE){
            continue;
        }
        if(time_ms - block_down_last_time_ms[i] > BLOCK_DOWN_INTERVAL_MS){
            input(i, GAME_OP_SYS_DOWN);
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
void Logical::set_server(Game* game) {
    server_side = 1;
    this->server_game = game;
}
