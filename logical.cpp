#include <curses.h>
#include <ncurses.h>
#include <unistd.h>
#include <cctype>
#include <ctime>

void update(int size){
    clear();
    for(int i = 0; i < size; i ++){
        for(int j = 0; j < size; j ++){
            printw("[]");
        }
        printw("\n");
    }
    refresh();
}


int main()
{
    initscr();
    nodelay(stdscr, true);
    noecho();

    std::time_t last_t = std::time(0);
    int count = 0;
    char ch;
    for(;;){
        std::time_t t = std::time(0);
        if(t - last_t > 1){
            update(++ count);
            last_t = t;
        }
        ch = getch();
        if (ch >= 0x20 && ch <= 0x7F){
            printw("|%c,%x|\n", ch,(int)ch);
            if (toupper(ch) == 'Q') {
                break;
                printw("break.......\n");
            }
        }
        refresh();
    }
    endwin();
    return 0;
}