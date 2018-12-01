#ifndef _DISPLAY_H_ //Header guard
#define _DISPLAY_H_

#include "sprites.h"
#include <stdint.h>
#include <stdlib.h>


void display_init();
void draw_pixel(int x, int y, uint8_t* retArray);
void display_image_hori(const uint8_t data[]);

#endif //_DISPLAY_H_