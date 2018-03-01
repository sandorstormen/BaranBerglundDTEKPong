#ifndef MEMCPY
#define MEMCPY
// Made my own memcpy function beacuse the chipkit doesn't have one
void memcpy ( int * destination, const int * source, int num ) {
	int i;
	for(i=0; i < num; i++) {
		destination[0] = source[0];
	}
}
#endif