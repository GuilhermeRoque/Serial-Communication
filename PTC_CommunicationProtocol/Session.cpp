/*
 * Session.cpp
 *
 *  Created on: 3 de out de 2019
 *      Author: vini
 */

#include "Session.h"

Session::Session(int fd, long tout) : Layer(fd, tout) {
	disable_timeout();
	_state = DISC;
	bytes_tx = 0;
}

Session::Session(long tout) : Layer(tout) {
	disable_timeout();
	_state = DISC;
	bytes_tx = 0;
}

Session::~Session() {
	// TODO Auto-generated destructor stub
}

void Session::init() {
	// Do something ... (init handshaking ?)
}

void Session::send(char *buffer, int bytes) {
	Evento ev = Evento(Payload, buffer, bytes);
	handle_fsm(ev);
}

void Session::notify(char * buffer, int len) {
	Evento ev = Evento(Quadro, buffer, len);
	handle_fsm(ev);
}

void Session::notifyERR() {
	Evento ev = Evento(Erro, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle() {

}

void Session::handle_timeout() {
	Evento ev = Evento(Timeout, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle_fsm(Evento & e) {

}

