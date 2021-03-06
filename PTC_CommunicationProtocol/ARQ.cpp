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
	retry_counter = 0;
}

ARQ::~ARQ() {
}

bool ARQ::is_ACK(uint8_t byte){
	return ((byte>>7)&1);
}
bool ARQ::is_DATA(uint8_t byte){
	return (!((byte>>7)&1));
}
bool ARQ::check_SEQ(uint8_t byte,bool bit){
	return (!(((byte>>3)&1)^bit));
}


void ARQ::notify(char * buffer, int len) {
	//-----------para debug apenas
		printf("ARQ recebeu do Framming: ");
	    print_buffer(buffer,len);
	//----------------------------

	Evento ev = Evento(Quadro,buffer,len);
	handle_fsm(ev);
}

void ARQ::handle() {}

void ARQ::handle_timeout() {
	std::cout << "[ARQ]" << "timeout\n";
	Evento ev = Evento();
	handle_fsm(ev);
}

void ARQ::handle_fsm(Evento & e) {
	//bit 7 -> 0 = DATA e 1 = ACK
	//bit 3 Sequencia

	uint8_t ctrl_byte = e.ptr[0];
	switch (_state) {
    	case Idle:
			std::cout << "[ARQ]" << "IN IDLE\n";
    		// app?payload/(!dataN,enable_timeout)
    		if(e.tipo == Payload){
    			char buffer[e.bytes + 1];
    			buffer[0] = N?0x8:0; //Quadro de dados e sequência N
    			memcpy(buffer +1,e.ptr,e.bytes); //copia payload pro buffer com os bytes de ARQ para enviar
    			memcpy(buffer_tx,buffer,e.bytes+1); //copia quadro a ser enviado para se necessário reenviar
    			bytes_tx = e.bytes+1;
    			_lower->send(buffer,e.bytes +1); //passa pro Framming
    			reload_timeout();
    			enable_timeout(); //ativa o timer
    			_state = WaitAck;
    			std::cout << "[ARQ]" << "indo para waitack\n";
    		}
    		// ?dataM/(!ackM,app!payload,M=M/)
    		else if(e.tipo == Quadro and  is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)){
    			if (e.bytes > 0) {
					char buffer[e.bytes -1];
					memcpy(buffer,e.ptr + 1,e.bytes -1);
					_upper->notify(buffer,e.bytes - 1);
    			}

    			char buffer_ACK[1];
    			buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
    			_lower->send(buffer_ACK,1);
    			M = !M;
				disable_timeout();
    			_state = Idle;
    			std::cout << "[ARQ]" << "indo para idle 1\n";

    		}
    		// ?dataM//(!ackM/)
    		else if(e.tipo == Quadro and is_DATA(ctrl_byte) and not check_SEQ(ctrl_byte,M)){
    			if (e.bytes > 0) {
					char buffer_up[e.bytes -1];
					memcpy(buffer_up,e.ptr + 1,e.bytes -1);
					_upper->notify(buffer_up,e.bytes - 1);
    			}

    			char buffer[1];
    			buffer[0] = M?0x80:0x88; //Quadro de ACK e sequência M/
    			_lower->send(buffer,1);
				disable_timeout();
    			_state = Idle;
    			std::cout << "[ARQ]" << "indo para idle 2\n";
    		}
    		break;

    	case WaitAck:
			std::cout << "[ARQ]" << "IN WAITACK\n";
    		// ?ackN/(disable_timeout,N:=N/)
    		// ?ackN/set_backoff
    		if(e.tipo == Quadro and is_ACK(ctrl_byte) and check_SEQ(ctrl_byte,N)){
    			char buffer[e.bytes -1];
    			memcpy(buffer,e.ptr + 1,e.bytes -1);
    			_upper->notify(buffer,e.bytes - 1);
    			N = !N;
    			set_backoff();
    			_state = BackoffAck;
    			std::cout << "[ARQ]" << "indo para BackoffAck\n";
    		}
    		// ?dataM/(!ackM,app!payload,M=M/)
    		else if(e.tipo == Quadro and is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)){
    			char buffer[e.bytes -1];
    			memcpy(buffer,e.ptr + 1,e.bytes -1);
    			_upper->notify(buffer,e.bytes - 1);
    			char buffer_ACK[1];
    			buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
    			_lower->send(buffer_ACK,1);
    			M = !M;
    			_state = WaitAck;
    			std::cout << "[ARQ]" << "indo para WaitAck 2\n";
    		}
    		// ?dataM//(!ackM/)
    		else if(e.tipo == Quadro and is_DATA(ctrl_byte) and not check_SEQ(ctrl_byte,M)){
    			char buffer[1];
    			buffer[0] = M?0x80:0x88; //Quadro de ACK e sequência M/
    			_lower->send(buffer,1);
    			_state = WaitAck;
    			std::cout << "[ARQ]" << "indo para WaitAck 3\n";
    		}
    		// ?timeout or ?ackN/ / (!dataN/ reload_timeout)
    		// ?timeout or ?ackN/ / set_backoff
    		else if((e.tipo == Quadro and is_ACK(ctrl_byte) and not check_SEQ(ctrl_byte,N)) or e.tipo == Timeout){
//    			reload_timeout();
//    			_lower->send(buffer_tx,bytes_tx); //passa pro Framming

    			if (retry_counter == 3) {
    				retry_counter = 0;
    				_upper->notifyERR();
					disable_timeout();
    				_state = Idle;
					std::cout << "[ARQ]" << "indo para Idle (retry_counter)\n";
    			} else {
					set_backoff();
					retry_counter++;
					_state = BackoffRelay;
					std::cout << "[ARQ]" << "indo para BackoffRelay\n";
    			}

    		}
    		break;

    	case BackoffAck:
			std::cout << "[ARQ]" << "IN BackoffAck\n";
    		// backoff/
    		if (e.tipo == Timeout) {
    			_state = Idle;
    			disable_timeout();
    			std::cout << "[ARQ]" << "indo para idle from backoffack\n";
    		}
    		// ?dataM !ackM
    		else if (e.tipo == Quadro and is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)) {
				char buffer[e.bytes -1];
				memcpy(buffer,e.ptr + 2,e.bytes -1);
				_upper->notify(buffer,e.bytes - 1);
				char buffer_ACK[1];
				buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
				_lower->send(buffer_ACK,1);
				M = !M;
				_state = BackoffAck;
				set_backoff();
				std::cout << "[ARQ]" << "indo para BackoffAck from backoffack\n";
    		}


    		break;

    	case BackoffRelay:
			std::cout << "[ARQ]" << "IN BackoffRelay\n";
    		// backoff/!dataN
    		if (e.tipo == Timeout) {
    			int buf_size = strlen(buffer_tx);
    			char buffer[buf_size];
    			strcpy(buffer, buffer_tx);
    			_lower->send(buffer, buf_size); // reenvia quadro
    			reload_timeout();
    			_state = WaitAck;
    			std::cout << "[ARQ]" << "indo para WaitAck from BackoffRelay\n";
    		}
    		// ?dataM/(!ackM,app!payload,M=M/)
    		else if (e.tipo == Quadro and is_DATA(ctrl_byte) and not check_SEQ(ctrl_byte,M)) {
				char buffer[e.bytes -1];
				memcpy(buffer,e.ptr + 1,e.bytes -1);
				_upper->notify(buffer,e.bytes - 1);
				char buffer_ACK[1];
				buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
				_lower->send(buffer_ACK,1);
				M = !M;
				_state = BackoffRelay;
				set_backoff();
				std::cout << "[ARQ]" << "indo para BackoffRelay from BackoffRelay\n";
    		}
    		break;
    }
}

void ARQ::send(char *buffer, int bytes) {
	//-----------para debug apenas
		printf("ARQ recebeu para enviar: ");
	    print_buffer(buffer,bytes);
	//----------------------------
	Evento ev = Evento(Payload,buffer,bytes);
	enable_timeout();
	handle_fsm(ev);
}

void ARQ::set_backoff() {
//	 set_timeout(TIMEOUT_BACKOFF);
	tout = TIMEOUT_BACKOFF;
}

void ARQ::notifyERR() {

}

