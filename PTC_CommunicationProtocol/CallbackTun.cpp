/*
 * CallbackTun.cpp
 *
 *  Created on: 21 de ago de 2019
 *      Author: vini
 */

#include "CallbackTun.h"
#include <stdio.h>

CallbackTun::CallbackTun(Tun &tun, long tout) : _tun(tun), Layer(tun.get(), tout) {
}

CallbackTun::~CallbackTun() {
	
}

void CallbackTun::init() {
}

void CallbackTun::close() {

}

void CallbackTun::send(char * buffer, int bytes) {
}

void CallbackTun::notify(char * buffer, int len) {
	_tun.write(buffer, len);
}

void CallbackTun::notifyERR() {

}

void CallbackTun::handle() {
	printf("handle\n");
	TunFrame *f = _tun.get_frame();
	printf("%s\n", (char *) f);
	_lower->send(f->buffer, f->len);
}

void CallbackTun::handle_timeout() {

}