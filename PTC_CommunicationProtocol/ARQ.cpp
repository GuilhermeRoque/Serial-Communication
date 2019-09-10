/*
 * ARQ.cpp
 *
 *  Created on: 5 de set de 2019
 *      Author: guilherme
 */

#include "ARQ.h"

ARQ::ARQ(long tout):Layer(tout) {
	disable_timeout();
	M = 0;
	N = 0;
	_state = Idle;
}

ARQ::~ARQ() {
}

void ARQ::notify(char * buffer, int len) {
	eventos.push(Evento(Quadro,buffer,len));
}

void ARQ::handle() {
	Evento ev = eventos.front();
	eventos.pop();
	handle_fsm(ev);
}

void ARQ::handle_timeout() {

}
void ARQ::handle_fsm(Evento & e) {
	//bit 7 -> 0 = DATA e 1 = ACK
	//bit 3 Sequencia

	switch (_state) {
    	case Idle:
    		if(e.tipo == Payload){
    			char buffer[e.bytes + 2];
    			buffer[0] = 0; //Quadro de dados e sequência 0
    			buffer[1] = 0;
    			memcpy(buffer +2,e.ptr,e.bytes);
    			_lower->send(buffer,e.bytes +2);
    			enable_timeout();
    		}
    		break;
    	case WaitAck:
    		if(e.tipo == Quadro and ((e.ptr[0]>>7)&1) and !(((e.ptr[0]>>3)&1)^N)){
    			char buffer[e.bytes - 2];
    			memcpy(buffer,e.ptr + 2,e.bytes -2);
    			_upper->notify(buffer,e.bytes - 2);
    			N = !N;
    		}
    		if(e.tipo == Quadro and !((e.ptr[0]>>7)&1) and !(((e.ptr[0]>>3)&1)^M)){
    		}

    		break;
    }
}

void ARQ::send(char *buffer, int bytes) {
	eventos.push(Evento(Payload,buffer,bytes));
}
