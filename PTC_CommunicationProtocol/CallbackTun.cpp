/*
 * CallbackTun.cpp
 *
 *  Created on: 21 de ago de 2019
 *      Author: vini
 */

#include "CallbackTun.h"
#include <stdio.h>
#include <string.h>
#include "utils.h"

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
	print_buffer(buffer,len);

	char buf[1024];
	memset(buf, 0, 1024);
	char proto = buffer[0];

	if (proto == 0x01) {
		TunFrame f(buffer+1, len-1, 0x0800);
		_tun.send_frame(&f);
	} else if (proto == 0x02) {
		TunFrame f(buffer+1, len-1, 0x86dd);
		_tun.send_frame(&f);
	} else {
		printf("% Protocolo desconhecido.\n");
		return;
	}
}

void CallbackTun::notifyERR() {

}

void CallbackTun::handle() {
	if (not _lower->is_enabled())
		return;

	TunFrame *f = _tun.get_frame();
//	printf("PROTO: %x\n", f->get_proto());

	char buf[1024];
	memset(buf, 0, 1024);
	uint16_t proto = f->get_proto();
	if (proto == 0x0800) {
		buf[0] = 0x01;
	}
	else if (proto == 0x86dd) {
		buf[0] = 0x02;
	} else {
		printf("% Protocolo (ethertype) desconhecido. Descartando ... \n");
		delete f;
		return;
	}

	memcpy(buf+1, f->buffer + 4, f->len - 4);
	_lower->send(buf, f->len - 3);

	delete f;
}

void CallbackTun::handle_timeout() {
	if(not _lower->is_enabled()){
		_lower->init();
	}else{
		disable_timeout();
	}
}
