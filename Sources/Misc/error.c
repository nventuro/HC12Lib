#include "error.h"
#include "common.h"
#include <stdio.h>

void err_Throw(char* errMsg)
{
	printf("%s\n", errMsg);
	asm sei;
	
	while (1)
		;
		
	return;
}