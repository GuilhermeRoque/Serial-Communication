/*
 * utils.h
 *
 *  Created on: 15 de set de 2019
 *      Author: guilherme
 */

#ifndef UTILS_H_
#define UTILS_H_
#include <stdio.h>
#include <iostream>
#include <string.h>
#include "Session.h"
#include <boost/date_time/posix_time/posix_time.hpp>

void print_buffer(char * ptr,int n);
void print_buffer_ascii(char * ptr, int n);
void log_print(const char *fmt, ...);

#endif /* UTILS_H_ */
