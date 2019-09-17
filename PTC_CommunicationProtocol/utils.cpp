/*
 * utils.cpp
 *
 *  Created on: 15 de set de 2019
 *      Author: guilherme
 */

#include "utils.h"

void print_buffer(char * ptr,int n){
	for (int x = 0; x<n-1;x++){
		printf("%x:",(uint8_t)ptr[x]);
	}
	printf("%x",(uint8_t)ptr[n-1]);
	printf("\n");
}



