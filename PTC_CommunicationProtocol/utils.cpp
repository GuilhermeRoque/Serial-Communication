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

void print_buffer_ascii(char * ptr, int n) {
	for (int x = 0; x<n-1;x++){
		printf("%c", ptr[x]);
	}
	printf("%c",ptr[n-1]);
	printf("\n");
}

void log_print(const char *fmt, ...) {
	time_t sec;
	time(&sec);
	struct tm *time_now = localtime(&sec);

	const long hours = time_now->tm_hour;
	const long minutes = time_now->tm_min;
	const long seconds = time_now->tm_sec;

    char buf[40];
    sprintf(buf, "%02ld:%02ld:%02ld", hours, minutes, seconds);

    char message[255];
	va_list args;
	va_start(args, fmt);
	vsprintf(message,fmt, args);
	va_end(args);
    printf("%s %s\n", buf, message);
}
