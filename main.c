#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>
#include "sprites.h"
#include "u32helpers.h"
#include "memcpy.h"

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

/*	The display has a D/C pin (display or command select) that is used to determine whether bytes sent to the display
**	are interpreted as commands or as display data. The D/C pin is set high for display buffer access and low for
**	command access.
*/

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

void port_init() {
	/* Set up peripheral bus clock */
	OSCCON &= ~0x180000;
	OSCCON |= 0x080000;
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	
	/* Clear SPIROV*/
	SPI2STATCLR &= ~0x40;
	/* Set CKP = 1, MSTEN = 1; */
    SPI2CON |= 0x60;
	
	/* Turn on SPI */
	SPI2CONSET = 0x8000;
	
	// Timer setup
	T2CON = 0x8070; // bit 15 = 1 timer enabled| Bit 6-4 = 111 1:256 prescaling | bit 3 = 0 16-bit mode | bit 1 = 0 => internal clock sås
	PR2 = 31250;  //	80 000 000 / 256 / 31250 = 10 => 1/10
}

void display_image(const uint8_t data[]) {
	int i, j;
	spi_send_recv(0x20); // Set adressing mode
	spi_send_recv(0x2); // Page adressing mode
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0xB0 + i);
		//spi_send_recv(0x22);
		//spi_send_recv(i);
		
		//spi_send_recv(x & 0xF);
		//spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 128; j++) {
			spi_send_recv(data[i*128 + j]);
		}
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	}
}

void display_clear() {
	int i, j;
	spi_send_recv(0x20); // Set adressing mode
	spi_send_recv(0x2); // Page adressing mode
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		
		spi_send_recv(0xB0 + i);
		//spi_send_recv(i);
		
		//spi_send_recv(x & 0xF);
		//spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 128; j++) {
			spi_send_recv((uint8_t)0x00);
		}
	}
}

void main_menu_init();
void main_menu_update();
void main_menu_update();
int debugVar;
void vbyte_to_bool(int height, int width, const volatile uint8_t* src, volatile uint8_t* dst) {
	// Height has to be devisible by eight
	int i,j,t;
	for(i = 0; i < (height/8); i++) {
		for(t = 0; t < 8; t++) {
			for(j = 0; j < width; j++) {
				dst[i*128*8 + j*8 + t] = ((src[i*width + j]) >> (7-t)) & 0x01;
			}
		}
	}
	return;
}
void bool_to_vbyte(uint8_t* src, uint8_t* dst){
	// Height has to be devisible by eight
	int i,j,t;
	for(i = 0; i < 32/8; i++) {
		for(t = 0; t < 8; t++) {
			for(j = 0; j < 128; j++) {
				dst[i*128 + j] |= src[i*128*8 + j*8 + t] << (7-t);
			}
		}
	}
	return;
}
struct Ball currentBall;
struct Paddle paddle_l, paddle_r;
uint8_t bool_screen[32 * 128];
uint8_t temp_screen[32 * 128];
void game_init(){
	currentBall.x= 64-6;
	currentBall.y= 8;
	
	vbyte_to_bool( 32, 128, &blank_screen[0], &bool_screen[0]);
	vbyte_to_bool( 16, 12, &ball_sprite[0], &currentBall.bool_array[0]);
};
void game_update();
void make_screen_game(uint8_t* retArray, Ball ball, Paddle l_paddle, Paddle r_paddle) {
	int i, j, t;
	//memcpy((int *)&temp_screen[0], (int *)&bool_screen[0], 32*128);
	for(i = 0; i < 32/8; i++) {
		for(t = 0; t < 8; t++) {
			for(j = 0; j < 128; j++) {
				if(currentBall.x <= j && (currentBall.x + 12) >= j) {
					if(currentBall.y/8 <= i && (currentBall.y + 16)/8 >= i) {
						if((currentBall.y%8 <= (7-t) && (currentBall.y + 16)%8 >= (7-t)) || (currentBall.y/8 < i && (currentBall.y + 16)/8 > i) || (currentBall.y%8 <= (7-t)) && (currentBall.y + 16)/8 > i || (currentBall.y + 16)%8 >= (7-t) && currentBall.y/8 < i){
							bool_screen[i*128*8 + j*8 + t] |= currentBall.bool_array[(i-currentBall.y/8)*12*8 + (j-currentBall.x)*8 + t];
							//debugVar = (i-currentBall.y/8)*12*8 + (j-currentBall.x)*8 + t;
							//display_debug(&debugVar);
							//delay(1000000);
						}
					}
				}
			}
		}
	}
	/*
	for(i = 0; i < 32; i++) {
		for(j = 0; j < 128; j++) {
			if(currentBall.x <= j && (currentBall.x + 12) >= j && currentBall.y <= i && (currentBall.y + 16) >= i) {
				//bool_screen[i*128 + j] |= currentBall.bool_array[(i-currentBall.y)*12 + (j-currentBall.x)];
			}
			if(j < 128 && i == 0)
				bool_screen[i*128 + j] |= 1;
			
		}
	
	}
	*/
	bool_to_vbyte(&bool_screen[0], retArray);
	return;
}
uint8_t display_array[4 * 128];
void game_draw(){
	if(IFS(0) & 0x100) {
		IFS(0) &= ~0x100;
		//display_clear();
		make_screen_game(&display_array[0], currentBall, paddle_l, paddle_r);
		display_image(&display_array[0]);
		//currentBall.x++;
		//currentBall.y--;
	}
}



int main() {
	port_init();
	display_init();
	game_init();
	display_clear();
	while(1) {
		game_draw();
	}
	for(;;);
	return 0;
}
