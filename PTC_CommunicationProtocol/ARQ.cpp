/*
 * ARQ.cpp
 *
 *  Created on: 5 de set de 2019
 *      Author: guilherme
 */

#include "ARQ.h"

ARQ::ARQ(Framming & fr,long tout):Layer(tout) {
	disable_timeout();
	_lower =fr;
	n_rx = 0;
	n_tx = 0;
	_state = Idle;
}

ARQ::~ARQ() {
}

void ARQ::notify(char * buffer, int len) {

}

void ARQ::handle() {
	char * frame;
}

void ARQ::handle_timeout() {

}
void ARQ::handle_fsm(Evento & e) {
    switch (_state) {
    	case Idle:

    		break;
    	case WaitAck:
    		break;
}

void ARQ::send(char *buffer, int bytes) {
}
