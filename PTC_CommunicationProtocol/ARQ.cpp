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
	bytes_tx = 0;
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
    		// app?payload/(!dataN,enable_timeout)
    		if(e.tipo == Payload){
    			char buffer[e.bytes + 2];
    			buffer[0] = N?0x8:0; //Quadro de dados e sequência N
    			buffer[1] = 0; //Proto (não utilizado ainda)
    			memcpy(buffer +2,e.ptr,e.bytes); //copia payload pro buffer com os bytes de ARQ para enviar
    			memcpy(buffer_tx,buffer,e.bytes+2); //copia quadro a ser enviado para se necessário reenviar
    			bytes_tx = e.bytes+2;
    			_lower->send(buffer,e.bytes +2); //passa pro Framming
    			enable_timeout(); //ativa o timer
    			_state = WaitAck;
    		}
    		// ?dataM/(!ackM,app!payload,M=M/)
    		else if(e.tipo == Quadro and !((e.ptr[0]>>7)&1) and !(((e.ptr[0]>>3)&1)^M)){
    			char buffer[1024];
    			memcpy(buffer,e.ptr + 2,e.bytes -2);
    			_upper->notify(buffer,e.bytes - 2);
    			char buffer_ACK[2];
    			buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
    			buffer_ACK[1] = 0; //Proto (não utilizado ainda)
    			_lower->send(buffer,2);
    			M = !M;
    			_state = Idle;
    		}
    		break;

    	case WaitAck:
    		// ?ackN/(disable_timeout,N:=N/)
    		if(e.tipo == Quadro and ((e.ptr[0]>>7)&1) and !(((e.ptr[0]>>3)&1)^N)){
    			N = !N;
    			_state = Idle;
    			disable_timeout(); //desativa o timer
    		}
    		// ?dataM/(!ackM,app!payload,M=M/)
    		else if(e.tipo == Quadro and !((e.ptr[0]>>7)&1) and !(((e.ptr[0]>>3)&1)^M)){
    			char buffer[1024];
    			memcpy(buffer,e.ptr + 2,e.bytes -2);
    			_upper->notify(buffer,e.bytes - 2);
    			char buffer_ACK[2];
    			buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
    			buffer_ACK[1] = 0; //Proto (não utilizado ainda)
    			_lower->send(buffer_ACK,2);
    			M = !M;
    			_state = WaitAck;
    		}
    		// ?dataM//(!ackM/)
    		else if(e.tipo == Quadro and !((e.ptr[0]>>7)&1) and (((e.ptr[0]>>3)&1)^M)){
    			char buffer[2];
    			buffer[0] = M?0x80:0x88; //Quadro de ACK e sequência M/
    			buffer[1] = 0; //Proto (não utilizado ainda)
    			_lower->send(buffer,2);
    			_state = WaitAck;
    		}
    		// ?ackN//(!dataN/)
    		else if(e.tipo == Quadro and ((e.ptr[0]>>7)&1) and (((e.ptr[0]>>3)&1)^N)){
    			_lower->send(buffer_tx,bytes_tx); //passa pro Framming
    		}
    		break;
    }
}

void ARQ::send(char *buffer, int bytes) {
	eventos.push(Evento(Payload,buffer,bytes));
}
