#ifndef DISPLAY_H_ //Header guard
#define DISPLAY_H_

#include <stdint.h>
#include <stdlib.h>

void game_init();
void reset_score();
int game_run(short int tennis);

void main_menu_init();
int main_menu_run();

#endif //_DISPLAY_H_