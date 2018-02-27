#ifndef SPRITES
#define SPRITES

#define 	ball_width() 12;
#define 	ball_height() 9;
const uint8_t ball_sprite[2][12] = { 0x38, 0x7C, 0xFE, 0xC7, 0x83, 0x83, 0x83, 0x83, 0xC7, 0xFE, 0x7C, 0x38, 0x00 };
typedef struct Ball {
	int x;
	int y;
	
}Ball;

typedef struct Paddle {
	int x;
	int y;
}Paddle;

#endif