#include "sprites.h"
#include <stdint.h>
#include <stdlib.h>
#include <pic32mx.h>

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

void display_clear();
void display_fill();

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

void display_image_hori(const uint8_t data[]) {
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
			spi_send_recv(data[i*128 + j]);
		}
	}
}

void display_image_def(const uint8_t data[]) {
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0xB0 + i);
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 128; j++) {
			spi_send_recv(data[i*128 + j]);
		}
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

void draw_pixel(int x, int y, uint8_t* retArray) {
	int page = 0; // Which page is the pixel on (Y-Axis)
	int offset = 0; // How much offset within the page (Y-Axis)
	if(y > 0) {
		page = (y/8);
		offset = (y/4);
	}
	retArray[127*page + x] |= offset;
}

struct Ball currentBall;
struct Paddle paddle_l;
struct Paddle paddle_r;
void game_init(){
	currentBall.x= 65;
	currentBall.y= 15;
	currentBall.bit_index = 0;
	currentBall.page_index = 0;
	paddle_l.x = 2;
	paddle_l.y = 2;
	paddle_l.bit_index = 0;
	paddle_l.page_index = 0;
	paddle_r.x = 117;
	paddle_r.y = 0;
	paddle_r.bit_index = 0;
	paddle_r.page_index = 0;
	vbyte_to_bool( 32, 128, &blank_screen[0], &bool_screen[0]);
	//vbyte_to_bool( 32, 128, &bleck_screen[0], &bool_bleck[0]);
	vbyte_to_bool( 16, 12, &ball_sprite[0], &currentBall.bool_array[0]);
	vbyte_to_bool( 9, 32, &paddle_sprite[0], &paddle_l.bool_array[0]);
	vbyte_to_bool( 9, 32, &paddle_sprite[0], &paddle_r.bool_array[0]);
};

void make_screen_game(uint8_t* retArray) {
	
}

void make_screen_game_old(uint8_t* retArray) {
	int i, j, t, ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex;
	ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex = 0;
	debugVar = 0;
	memcpy((int *)&temp_screen[0], (int *)&bool_screen[0], 32*128);
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
				/*	i*128*8 + j*8 + t 
				if(64 <= j && (64 + 128) >= j) {
					if(16/8 <= i && (16 + 32)/8 >= i) {
						if((16%8 <= (7-t) && (16 + 32)%8 >= (7-t)) || (16/8 < i && (16 + 32)/8 > i) || (16%8 <= (7-t)) && (16 + 32)/8 > i || (16 + 32)%8 >= (7-t) && 16/8 < i){
							//bool_screen[i*128*8 + j*8 + t] |= bool_bleck[(i-(16/8))*128*8 + (j-64)*8 + (t-32%8)];
						}
					}
				}
				*/
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
void make_screen_game_old_old(uint8_t* retArray) {
	int i, j, t, ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex;
	ball_inc_bindex, paddle_l_inc_bindex, paddle_r_inc_bindex = 0;
	debugVar = 0;
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
					//debugVar = currentBall.page_index*128 + currentBall.bit_index;
					//display_debug(&debugVar);
					//delay(100000);
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