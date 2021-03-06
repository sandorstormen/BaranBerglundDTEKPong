#include <pic32mx.h>
#include <stdint.h>
#include <stdlib.h>
#include "u32helpers.h"
#include "memcpy.h"
#include "eeprom.h"
#include "io.h"
#include "display.h"


#define STATE_MAIN_MENU 1
#define STATE_MAIN_GAME 2

#define EEPROM_ADDRESS 0x50

/*	The display has a D/C pin (display or command select) that is used to determine whether bytes sent to the display
**	are interpreted as commands or as display data. The D/C pin is set high for display buffer access and low for
**	command access.
*/

/* void eeprom_init() {
	
	I2C1CON = 0x0;
	
	
	I2C1BRG = 0x0C2;
	I2C1STAT = 0x0;
	I2C1CONSET = 1 << 13; //SIDL = 1
	I2C1CONSET = 1 << 15; // ON = 1

	do {
		i2c_start();
	} while(!i2c_send(EEPROM_ADDRESS << 1));
	
	i2c_send(0x4);
	
	i2c_send(0x0);
	
	i2c_stop();
} */
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
	/* Set btn pins */
	TRISDSET = 0xE0;
	TRISFSET = 2;
	
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
	T1CON = 0x9830; // bit 15 = 1 timer enabled | bit 5-4 = 11 => 1:256 prescaling | 
	PR1 = 3125; //	80 000 000 / 256 / 31250 = 10 => 1/10
	
	T2CON = 0x8040; // bit 15 = 1 timer enabled| Bit 6-4 = 111 1:256 prescaling | bit 3 = 0 16-bit mode | bit 1 = 0 => internal clock sås
	PR2 = 1250; //	80 000 000 / 64 / 31250 = 10 => 1/10
	
	T3CON = 0x8040; // bit 15 = 1 timer enabled| Bit 6-4 = 111 1:256 prescaling | bit 3 = 0 16-bit mode | bit 1 = 0 => internal clock sås
	PR3 = 1250;  //	80 000 000 / 64 / 31250 = 10 => 1/10
	
	T4CON = 0x8070;
	PR4 = 31250;
	
	// bit 15 = 1 timer enabled| Bit 6-4 = 111 1:256 prescaling | bit 3 = 0 16-bit mode | bit 1 = 0 => internal clock sås
	//	80 000 000 / 256 / 31250 = 10 => 1/10
	
	// Initialize lights to be off
	PORTE = 0;
}

int current_game_state, prev_game_state;
int main() {
	port_init();
	display_init();
	prev_game_state = -1;
	current_game_state = STATE_MAIN_MENU;
	short int tennis = 2;
	while(1) {
		switch (current_game_state){
			case STATE_MAIN_GAME:
				if( 0 == game_run(tennis) ) { 
				current_game_state = STATE_MAIN_MENU; }
			break;
			case STATE_MAIN_MENU:
				tennis = main_menu_run();
				if( tennis < 2 )
					current_game_state = STATE_MAIN_GAME;
			break;
		}
		if(prev_game_state != current_game_state) {
			switch (current_game_state) {
				case STATE_MAIN_GAME:
					reset_score();
					game_init();
				break;
				case STATE_MAIN_MENU:
					main_menu_init();
				break;
			}
		}
		prev_game_state = current_game_state;
		}
	return 0;	
}