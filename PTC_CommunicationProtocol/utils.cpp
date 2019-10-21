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
    const boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::local_time();

    const boost::posix_time::time_duration td = now.time_of_day();

    const long hours        = td.hours();
    const long minutes      = td.minutes();
    const long seconds      = td.seconds();
    const long milliseconds = td.total_milliseconds() -
                              ((hours * 3600 + minutes * 60 + seconds) * 1000);

    char buf[40];
    sprintf(buf, "%02ld:%02ld:%02ld.%03ld",
        hours, minutes, seconds, milliseconds);

    char message[255];
	va_list args;
	va_start(args, fmt);
	vsprintf(message,fmt, args);
	va_end(args);
    printf("%s %s\n", buf, message);
}
