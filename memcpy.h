#ifndef SPRITES
#define SPRITES
void memcpy ( void * destination, const void * source, int num ) {
	int i;
	for(i=0; i < num; i++) {
		destination[0] = source[0];
	}
}
#endif