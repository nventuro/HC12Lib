/*
 * cbtest.c
 * 
 */

#include <stdio.h>
#include <ctype.h>
#include "../Sources/cb.h"

int main(int argc, char **argv)
{
	char k[8], c;
	cbuf cb = ARRAY2CB(k);
	
	while ((c = getchar()) != EOF) {
		if (c == '-') {
			int u = cb_pop(&cb);
			if (u >= 0) {
				puts("::> ");
				putchar(u);
				putchar('\n');
			} else {
				puts("Not Ready\n");
			}
		} else if (isalpha(c)) {
			if (cb_push(&cb, c) < 0)
				puts("No memory\n");
		}
	}
	
	return 0;
}

