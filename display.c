#include <stdint.h>
#include <stdlib.h>
#include <pic32mx.h>
#include "itos.h"
#include "sprites.h"

#define DISPLAY_VDD PORTFbits.RF6
#define DISPLAY_VBATT PORTFbits.RF5
#define DISPLAY_COMMAND_DATA PORTFbits.RF4
#define DISPLAY_RESET PORTGbits.RG9


#define DISPLAY_VDD_PORT PORTF // Uno32 pin 38
#define DISPLAY_VDD_MASK 0x40
#define DISPLAY_VBATT_PORT PORTF // Uno32 pin 40
#define DISPLAY_VBATT_MASK 0x20
#define DISPLAY_COMMAND_DATA_PORT PORTF // Uno32 pin 39
#define DISPLAY_COMMAND_DATA_MASK 0x10
#define DISPLAY_RESET_PORT PORTG // Uno32 pin 10 JP4
#define DISPLAY_RESET_MASK 0x200

#define PADDLE_HEIGHT   8

uint8_t display_array[4 * 128] = {0};
int ball_speed, paddle_speed;

void delay(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 0x01));
	return SPI2BUF;
}


void display_init() {
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK; // Clear the command port
	delay(10);
	DISPLAY_VDD_PORT &= ~DISPLAY_VDD_MASK; // Put the power on for the display and wait for it to power on
	delay(1000000);
	
	spi_send_recv(0xAE);	// Display off command
	DISPLAY_RESET_PORT &= ~DISPLAY_RESET_MASK;	// Bring reset low 
	delay(10);
	DISPLAY_RESET_PORT |= DISPLAY_RESET_MASK;	// Bring rest high
	delay(10);
	
	spi_send_recv(0x8D);	// Send the Set Charge Pump from P 3/6 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);	// Set Pre-Charge Period command from P 32/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf 
	spi_send_recv(0xF1);
	
	DISPLAY_VBATT_PORT &= ~DISPLAY_VBATT_MASK;  // Turn on VCC and wait 100ms
	delay(10000000);
	// Send the commands to invert the display. This puts the display origin in the upper left corner.
	spi_send_recv(0xA1);	//remap columns
	spi_send_recv(0xC8);	//remap rows
	// Send the commands to select sequential COM configuration. This makes the display memory non-interleaved.
	spi_send_recv(0xDA); //set COM configuration command
	spi_send_recv(0x20); //sequential COM, left/right remap enabled
	
	spi_send_recv(0x81); // Set contrast command
	spi_send_recv(0xFF); // Hex for 255
	
	spi_send_recv(0xAF); // Send Display On command
}
void draw_char(int x_offset, int y_offset, char text, uint8_t* draw_to) {
	int i;
	for (i = 0; i < 8; i++) {
		draw_to[x_offset + i + (y_offset-1) * 128] = font[text * 8 + i];
	}
}
void draw_string(int x_offset, int y_offset, char* text, uint8_t* draw_to) {
	int i = 0;
	while(1) {
		if( text[i] == '\0' ) { 
			break;
		}
		else {
			draw_char( x_offset + (i * 8), y_offset, text[i], draw_to);
			i++;
		}
	}
}
void display_fill() {
	int i, j;
	uint8_t clear_dat = 0x00;
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	
	spi_send_recv(0x20); // Set adressing mode  P 30/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x00); // Horizontal adressing mode
	
	spi_send_recv(0x22); // Set page adress P 31/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x00); // Page start = 0
	spi_send_recv(0x04); // Page end = 3
	
	DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
	
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 128; j++) {
			spi_send_recv(0xAA);
		}
	}
}

void display_image_hori() {
	int i, j;
	
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	
	spi_send_recv(0x20); // Set adressing mode  P 30/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x00); // Horizontal adressing mode
	
	spi_send_recv(0x22); // Set page adress P 31/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x00); // Page start = 0
	spi_send_recv(0x04); // Page end = 3
	
	DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		//spi_send_recv(0xB0 + i);
		//spi_send_recv(0x22);
		//spi_send_recv(i);
		
		//spi_send_recv(x & 0xF);
		//spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 128; j++) {
			spi_send_recv(display_array[i*128 + j]);
		}
	}
}

void display_image_def() {
	int i, j;
    
    for(i = 0; i < 4; i++) {
        DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
        spi_send_recv(0x22);
        spi_send_recv(i);

        spi_send_recv(0 & 0xF);
        spi_send_recv(0x10 | ((0 >> 4) & 0xF));

        DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;

        for(j = 0; j < 128; j++)
            spi_send_recv(display_array[i*128 + j]);
    }
}

void display_clear() {
	int i, j;
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	
	spi_send_recv(0x20); // Set adressing mode  P 30/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x00); // Horizontal adressing mode
	
	spi_send_recv(0x22); // Set page adress P 31/59 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	spi_send_recv(0x00); // Page start = 0
	spi_send_recv(0x04); // Page end = 3
	
	DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
	
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 128; j++) {
			spi_send_recv(0x00);
		}
	}
}

void clear() {
	int i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 128; j++) {
			display_array[i*128 + j] = 0x00;
		}
	}
}

void draw_pixel(int x, int y) {
	int page_offset = 0;
	if(y > 0) {
		page_offset = (y/8);
	}
	display_array[128*page_offset + x] |= 1 << (y - page_offset * 8);
}
uint8_t check_pixel(int x, int y) {
	int page_offset = 0;
	if(y > 0) {
		page_offset = (y/8);
	}
	return display_array[128*page_offset + x] & 1 << (y - page_offset * 8);
}
void draw_paddle(int x, int y){
	int i;
	for(i = 0; i < PADDLE_HEIGHT; i++){
		draw_pixel(x, (y+i));
	}
}
void draw_score_tennnis(int score1, int score2) {
	int i;
	if(score1 == score2 && score1 > 3 && score2 > 3) {
		draw_string(44, 1, "deuce", display_array);
		return;
	}
	if(score1 == score2 +1) {
		draw_string(0, 1, "advantage", display_array);
		return;
	}
	if(score2 == score1 +1) {
		draw_string(56, 1, "advantage", display_array);
		return;
	}
	if(score1 == 0 && score2 == 0) return;
	draw_string(60, 1, ":", display_array);
	if(score1 == 0) draw_string(24, 1, "LOVE", display_array);
	if(score1 == 1) draw_string(42, 1, "15", display_array);
	char score1_str[3];
	if(10 > score1 && score1 > 1) {
		for (i = 0; i < 8; i++) {
			display_array[52 + i] = font['0' * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[52 - 8 + i] = font[( (score1 + 1) + '0') * 8 + i];
		}
	}
	if(score2 == 0) draw_string(74, 1, "LOVE", display_array);
	if(score2 == 1) draw_string(66, 1, "15", display_array);
	char score2_str[3];
	if(10 > score2 && score2 > 1) {
		for (i = 0; i < 8; i++) {
			display_array[68 + 8 + i] = font['0' * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[68 + i] = font[( (score2 + 1) + '0') * 8 + i];
		}
	}
}

void draw_score_30(int score1, int score2) {
	int i;
	if( score1 < 10) {
		for (i = 0; i < 8; i++) {
			display_array[52 + i] = font[(score1 + '0') * 8 + i];
		}
	}
	if(10 <= score1 && score1 < 100) {
		for (i = 0; i < 8; i++) {
			display_array[52 - 8 + i] = font[(((score1)/10) + '0') * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[52 + i] = font[((score1 - ((score1)/10)*10) + '0') * 8 + i];
		}
	}
	if(100 <= score1) {
		for (i = 0; i < 8; i++) {
			display_array[52 - 8 - 8 + i] = font[((score1/100) + '0') * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[52 - 8 + i] = font[(((score1 - ((score1/100)*100))/10) + '0' ) * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[52 + i] = font[((score1 - ((score1/100)*100) - (((score1/10)*10) - ((score1/100)*100))) + '0') * 8 + i];
		}
	}
    for (i = 0; i < 8; i++) {
        display_array[60 + i] = font[':' * 8 + i];
    }
	if( score2 < 10) {
		for (i = 0; i < 8; i++) {
			display_array[68 + i] = font[(score2 + '0') * 8 + i];
		}
	}
	if(10 <= score2 && score2 < 100) {
		for (i = 0; i < 8; i++) {
			display_array[68 + i] = font[(((score2)/10) + '0') * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[68 + 8 + i] = font[((score2 - ((score2)/10)*10) + '0') * 8 + i];
		}
	}
	if(100 <= score2) {
		for (i = 0; i < 8; i++) {
			display_array[68 + i] = font[((score2/100) + '0') * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[68 + 8 + i] = font[(((score2 - ((score2/100)*100))/10) + '0' ) * 8 + i];
		}
		for (i = 0; i < 8; i++) {
			display_array[68 + 8 + 8 + i] = font[((score2 - ((score2/100)*100) - (((score2/10)*10) - ((score2/100)*100))) + '0') * 8 + i];
		}
	}
}
void draw_score(int score1, int score2, short tennis) {
	if(tennis == 0) {
		draw_score_tennnis( score1, score2);
	}
	else {
		draw_score_30( score1, score2);
	}
}
struct Ball ball;
struct Paddle paddle_l;
struct Paddle paddle_r;

void reset_score() {
	draw_string(44, 3, "START", display_array);
	paddle_l.score = 0;
	paddle_r.score = 0;
}

void left_win(){
	draw_string(20, 2, "Left Player", display_array);
	draw_string(52, 3, "Win", display_array);
}
void right_win() {
	draw_string(16, 2, "Right Player", display_array);
	draw_string(52, 3, "Win", display_array);
}

void game_init(short tennis){ // Starts a new round of the game
	delay(1000);
	clear();
	if(tennis == 0) {
		if(paddle_l.score >= paddle_r.score +2) {
			left_win();
			display_image_hori();
			return;
		}
		if(paddle_r.score >= paddle_l.score +2) {
			right_win();
			display_image_hori();
			return;
		}
	}
	else {
		if(paddle_l.score >= 30) {
			left_win();
			display_image_hori();
			return;
	}
		if(paddle_r.score >= 30) {
			right_win();
			display_image_hori();
			return;
		}
	}
	
	draw_score(paddle_l.score, paddle_r.score, tennis);
	draw_paddle(paddle_l.x, paddle_l.y);
	draw_paddle(paddle_r.x, paddle_r.y);
	draw_pixel(ball.x, ball.y);
	draw_string((64-8-4), 3, "GO", display_array);
	display_image_hori();
	
	ball.x= 64;
	ball.y= 16;
	ball.speed_x = 13;
	ball.speed_y = 13;
	
	int r = 1;
	if(r == 1) {ball.direction_x = 1;}
	else { ball.direction_x = -1;}
	
	if(r == 1) {ball.direction_y = 1;}
	else { ball.direction_y = -1;}
	
	paddle_l.x = 2;
	paddle_l.y = 16;
	
	paddle_r.x = 126;
	paddle_r.y = 16;
	delay(5000000);
};

void make_screen_game(short tennis) {
	clear();
	draw_score(paddle_l.score, paddle_r.score, tennis);
	draw_paddle(paddle_l.x, paddle_l.y);
	draw_paddle(paddle_r.x, paddle_r.y);
	draw_pixel(ball.x, ball.y);
}
int btns;
void paddle_update() {
	if(btns = get_btns()){
		if (btns & 1) paddle_r.y--;
		if (btns & 2) paddle_r.y++;
		if (btns & 4) paddle_l.y--;
		if (btns & 8) paddle_l.y++;
	}
	
	if( paddle_r.y < 0) paddle_r.y = 0;
	if( paddle_r.y + PADDLE_HEIGHT > 32) paddle_r.y = (32 - PADDLE_HEIGHT);
	
	if( paddle_l.y < 0) paddle_l.y = 0;
	if( paddle_l.y + PADDLE_HEIGHT > 32) paddle_l.y = (32 - PADDLE_HEIGHT);
}
uint8_t interrupt_x, interrupt_y;
void check_collision(){ // Checks colison with paddles and ball
	int i;
	// Paddle_l
	if(ball.x == paddle_l.x +1 && ball.y == paddle_l.y  - 1) {
		ball.direction_x *= -1;
		ball.direction_y = -1;
	}
	if(ball.x == paddle_l.x +1 && ball.y == paddle_l.y + paddle_l.HEIGHT + 1) {
		ball.direction_x *= -1;
		ball.direction_y = 1;
	}
	for(i = 0; i < PADDLE_HEIGHT; i++){
		if(ball.x == paddle_l.x +1 && ball.y == paddle_l.y + i) {
			ball.direction_x *= -1;
			if(btns & 1) {
				ball.direction_y = -1;
				ball.speed_x = 13;
				ball.speed_y = 13;
			}
			if (btns & 2) {
				ball.direction_y = 1;
				ball.speed_x = 13;
				ball.speed_y = 13;
			}
			else {
				ball.speed_x = 9;
				ball.speed_y = 20;
			}
		}
	}
	if(ball.x == paddle_l.x && ball.y == paddle_l.y - 1) {
		ball.direction_x *= -1;
		ball.direction_y = -1;
	}
	if(ball.x == paddle_l.x && ball.y == paddle_l.y + paddle_l.HEIGHT + 1) {
		ball.direction_x *= -1;
		ball.direction_y = 1;
	}
	// Paddle_l
	
	// Paddle_r
	if(ball.x == paddle_r.x +1 && ball.y == paddle_r.y  - 1) {
		ball.direction_x *= -1;
		ball.direction_y = -1;
		ball.speed_x = 13;
		ball.speed_y = 9;
	}
	if(ball.x == paddle_r.x +1 && ball.y == paddle_r.y + paddle_r.HEIGHT + 1) {
		ball.direction_x *= -1;
		ball.direction_y = 1;
		ball.speed_x = 13;
		ball.speed_y = 9;
	}
	for(i = 0; i < PADDLE_HEIGHT; i++){
		if(ball.x == paddle_r.x  - 1 && ball.y == paddle_r.y + i) {
			ball.direction_x *= -1;
			if(btns & 1) {
				ball.direction_y = -1;
				ball.speed_x = 13;
				ball.speed_y = 13;
			}
			if (btns & 2) {
				ball.direction_y = 1;
				ball.speed_x = 13;
				ball.speed_y = 13;
			}
			else {
				ball.speed_x = 9;
				ball.speed_y = 20;
			}
		}
	}
	if(ball.x == paddle_r.x && ball.y == paddle_r.y - 1) {
		ball.direction_x *= -1;
		ball.direction_y = -1;
		ball.speed_x = 17;
		ball.speed_y = 7;
	}
	if(ball.x == paddle_r.x && ball.y == paddle_r.y + paddle_r.HEIGHT + 1) {
		ball.direction_x *= -1;
		ball.direction_y = 1;
		ball.speed_x = 17;
		ball.speed_y = 7;
	}
	// Paddle_r
}
void ball_update(short tennis) {
	
	check_collision();
	
	if(ball.x <= 0) { 
		paddle_r.score += 1; 
		game_init(tennis);
	}
	if(ball.x >= 128) {
		paddle_l.score += 1;
		game_init(tennis);
	}
	
	if( ball.y <= 0) { ball.direction_y = 1; }
	if( ball.y >= 32) { ball.direction_y = -1; }
	
	if(interrupt_x >= ball.speed_x ){
		ball.x += ball.direction_x;
		interrupt_x = 0;
	}
	if(interrupt_y >= ball.speed_y ){
		ball.y += ball.direction_y;
		interrupt_y = 0;
	}
	
};
void ball_interrupt(){ // Different timers are used to make the ball move faster 
	if(IFS(0) & 0x100) { // Ball speed in the x axis
		IFS(0) &= ~0x100;
		interrupt_x++;
	}
	if(IFS(0) & 0x1000) { // Ball speed in the y axis
		IFS(0) &= ~0x1000;
		interrupt_y++;
	}
}
int game_check_win(short tennis) {
	if(tennis == 0) {
		if(paddle_l.score >= paddle_r.score +2) {
			left_win();
			return 1;
		}
		if(paddle_r.score >= paddle_l.score +2) {
			right_win();
			return 1;
		}
	}
	else {
		if(paddle_l.score >= 30 ) {
			left_win();
			return 1;
		}
		if( paddle_r.score >= 30) {
			right_win();
			return 1;
		}
	}
	return 0;
}
int game_run(short tennis){ // Main game loop
	if ( game_check_win(tennis) == 1 ) {
		display_image_hori();
		delay(50000000);
		return 0;
	}
	ball_interrupt(); // Update ball speed
	ball_update(tennis); // Update ball position
	if(IFS(0) & 0x10) {
		IFS(0) &= ~0x10; 
		paddle_update();
	}
	make_screen_game(tennis);
	display_image_hori();
	return 1;
}
uint8_t main_menu_display[4 * 128] = {0};
void main_menu_init() {
	return;
}
bool selected_down = 0;
void draw_selector() {
	int i;
	if(selected_down) {
		draw_string(14, 1, "->", display_array);
	}
	else {
		draw_string(24, 3, "->", display_array);
	}
}
void make_screen_menu() {
	clear();
	draw_selector();
	draw_string(32, 1, "Go to 30", display_array);
	draw_string(40, 3, "Tennis", display_array);
}
void selector_down() {
	selected_down = 1;
}
void selector_up() {
	selected_down = 0;
}
int update_selector(){
	if(btns = get_btns()){
		if (btns & 4) selector_up(); // Selector up
		if (btns & 8) selector_down(); // Selector down
		if (btns & 1) { // Select
			return selected_down;
		}
	}
	return 2;
}
int main_menu_run() {
	int retVal = update_selector();
	make_screen_menu();
	display_image_hori();
	return retVal;
}
/*
void make_screen_game_old(uint8_t* retArray) {
	int i, j, t, ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex;
	ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex = 0;
	//memcpy((int *)&temp_screen[0], (int *)&bool_screen[0], 32*128);
	for(i = 0; i < 32/8; i++) {
		for(t = 0; t < 8; t++) {
			for(j = 0; j < 128; j++) {
				bool_screen[i*128*8 + j*8 + t] = 0;
			}
		}
	}
	for(i = 0; i < 32/8; i++) {
		for(t = 0; t < 8; t++) {
			for(j = 0; j < 128; j++) {
				if(currentBall.x <= j && (currentBall.x + 12) >= j) {
					if(currentBall.y/8 <= i && (currentBall.y + 16)/8 >= i) {
						if(currentBall.y%8 <= t && (currentBall.y + 16)%8 >= t || (currentBall.y/8 < i && (currentBall.y + 16)/8 > i) || (currentBall.y%8 <= t) && (currentBall.y + 16)/8 > i || (currentBall.y + 16)%8 >= t && currentBall.y/8 < i){
							
							bool_screen[i*128*8 + j*8 + t] |= currentBall.bool_array[(i-currentBall.y/8)*12*8 + (j-currentBall.x)*8 + t];
							ball_inc_bindex = 1;
							//debugVar = (i-(currentBall.y/8))*12*8;
							//display_debug(&debugVar);
							//delay(100000);
							
						}
					}
				}
				if(paddle_l.x <= j && (paddle_l.x + 9) >= j) {
					if(paddle_l.y/8 <= i && (paddle_l.y + 32)/8 >= i) {
						if((paddle_l.y%8 <= (7-t) && (paddle_l.y + 32)%8 >= (7-t)) || (paddle_l.y/8 < i && (paddle_l.y + 32)/8 > i) || (paddle_l.y%8 <= (7-t)) && (paddle_l.y + 32)/8 > i || (paddle_l.y + 32)%8 >= (7-t) && paddle_l.y/8 < i){
							bool_screen[i*128*8 + j*8 + t] |= paddle_l.bool_array[(i-(paddle_l.y/8))*9*8 + (j-paddle_l.x)*8 + t];
							paddle_l_inc_bindex = 1;
						}
					}
				}
				if(paddle_r.x <= j && (paddle_r.x + 9) >= j) {
					if(paddle_r.y/8 <= i && (paddle_r.y + 32)/8 >= i) {
						if((paddle_r.y%8 <= (7-t) && (paddle_r.y + 32)%8 >= (7-t)) || (paddle_r.y/8 < i && (paddle_r.y + 32)/8 > i) || (paddle_r.y%8 <= (7-t)) && (paddle_r.y + 32)/8 > i || (paddle_r.y + 32)%8 >= (7-t) && paddle_r.y/8 < i){
							bool_screen[i*128*8 + j*8 + t] |= paddle_r.bool_array[(i-(paddle_r.y/8))*9*8 + (j-paddle_r.x)*8 + t];
							paddle_r_inc_bindex = 1;
						}
					}
				}
					i*128*8 + j*8 + t 
				if(64 <= j && (64 + 128) >= j) {
					if(16/8 <= i && (16 + 32)/8 >= i) {
						if((16%8 <= (7-t) && (16 + 32)%8 >= (7-t)) || (16/8 < i && (16 + 32)/8 > i) || (16%8 <= (7-t)) && (16 + 32)/8 > i || (16 + 32)%8 >= (7-t) && 16/8 < i){
							//bool_screen[i*128*8 + j*8 + t] |= bool_bleck[(i-(16/8))*128*8 + (j-64)*8 + (t-32%8)];
						}
					}
				}
				
			}
			if(ball_inc_bindex) {
					ball_inc_bindex = 0;
					currentBall.bit_index++;
					if(currentBall.bit_index == 7) {
						currentBall.bit_index = 0;
						currentBall.page_index++;
				}
			}
			if(paddle_r_inc_bindex) {
					paddle_r_inc_bindex = 0;
					paddle_r.bit_index++;
					if(paddle_r.bit_index == 7) {
						paddle_r.bit_index = 0;
						paddle_r.page_index++;
				}
			}
			if(paddle_r_inc_bindex) {
					paddle_r_inc_bindex = 0;
					paddle_r.bit_index++;
					if(paddle_r.bit_index == 7) {
						paddle_r.bit_index = 0;
						paddle_l.page_index++;
				}
			}
		}
	}
	bool_to_vbyte(&bool_screen[0], retArray);
	return;
}
*/
/*
void make_screen_game_old_old(uint8_t* retArray) {
	int i, j, t, ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex;
	ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex = 0;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 128; j++) {
			//retArray[i*128 + j] = 0;
		}
	}
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 128; j++) {
			if(currentBall.x <= j && (currentBall.x + 12) >= j) {
				if(currentBall.y/8 <= i && (currentBall.y + 16)/8 >= i) {
					retArray[i*128 + j]  |= ball_sprite[currentBall.page_index*128 + currentBall.bit_index];
					ball_inc_bindex = 1;
				}
			}	
		}
		if(ball_inc_bindex) {
					ball_inc_bindex = 0;
					currentBall.bit_index++;
					if(currentBall.bit_index == 128) {
						currentBall.bit_index = 0;
						currentBall.page_index++;
				}
			}
			if(paddle_r_inc_bindex) {
					paddle_r_inc_bindex = 0;
					paddle_r.bit_index++;
					if(paddle_r.bit_index == 128) {
						paddle_r.bit_index = 0;
						paddle_r.page_index++;
				}
			}
			if(paddle_r_inc_bindex) {
					paddle_r_inc_bindex = 0;
					paddle_r.bit_index++;
					if(paddle_r.bit_index == 128) {
						paddle_r.bit_index = 0;
						paddle_l.page_index++;
				}
			}
		}
	return;
}
*/
