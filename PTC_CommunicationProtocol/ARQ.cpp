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
	memset(buffer_tx,0,1026);
	retry_counter = 0;
	disable();
	while(not buffer_ev.empty()){
		buffer_ev.pop();
	}
}

ARQ::ARQ(long tout, uint8_t id_sessao) : ARQ(tout) {
	this->id_sessao = (char) id_sessao;
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
//	-----------para debug apenas
//		printf("ARQ recebeu do Framming: ");
//	    print_buffer(buffer,len);
//	----------------------------

	// Descarta pacotes com id sessao diferente
	if (buffer[1] != id_sessao)
		return;

	Evento ev = Evento(Quadro,buffer,len);
	handle_fsm(ev);
}

void ARQ::handle() {}

void ARQ::handle_timeout() {
	Evento ev = Evento();
	handle_fsm(ev);
}
void ARQ::send(char *buffer, int bytes) {
	//-----------para debug apenas
		//printf("ARQ recebeu para enviar: ");
	    //print_buffer(buffer,bytes);
	//----------------------------
	Evento ev = Evento(Payload,buffer,bytes);
	if(_state == Idle){
    	handle_fsm(ev);
    }else{
    	buffer_ev.push(ev);
    }
}
void ARQ::send_payload(Evento e){
	_state = WaitAck;
	char buffer[e.bytes + 1];
	buffer[0] = N?0x8:0; //Quadro de dados e sequência N
	memcpy(buffer +1,e.ptr,e.bytes); //copia payload pro buffer com os bytes de ARQ para enviar
	memcpy(buffer_tx,buffer,e.bytes+1); //copia quadro a ser enviado para se necessário reenviar
	bytes_tx = e.bytes+1;
	_lower->send(buffer,e.bytes +1); //passa pro Framming
	reload_timeout();
	enable_timeout(); //ativa o timer
}

void ARQ::handle_fsm(Evento & e) {
	//bit 7 -> 0 = DATA e 1 = ACK
	//bit 3 Sequencia

	uint8_t ctrl_byte = e.ptr[0];
	switch (_state) {
    	case Idle:
    		//printf("[ARQ]IN IDLE %d \n",e.tipo);
    		// app?payload !dataN !enable_timeout
    		if(e.tipo == Payload){
    			send_payload(e);
    		}
    		// ?dataM !ackM app!payload M=M/)
    		else if(e.tipo == Quadro and  is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)){
    			char buffer_ACK[3];
    			buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
    			buffer_ACK[1] = id_sessao;
    			buffer_ACK[2] = e.ptr[2];
    			_lower->send(buffer_ACK,3);

    			M = !M;
    			_state = Idle;

    			char buffer[e.bytes -1];
    			memcpy(buffer,e.ptr + 1,e.bytes -1);
    			_upper->notify(buffer,e.bytes - 1);

    		}
    		// ?dataM/ !ackM/
    		else if(e.tipo == Quadro and is_DATA(ctrl_byte) and not check_SEQ(ctrl_byte,M)){
    			char buffer[3];
    			buffer[0] = M?0x80:0x88; //Quadro de ACK e sequência M/
    			buffer[1] = id_sessao;
    			buffer[2] = e.ptr[2];
    			_lower->send(buffer,3);
    			_state = Idle;
    		}
    		break;

    	case WaitAck:
    		// ?ackN !set_backoff N:=N/
       		//printf("[ARQ] IN WAITACK %d\n",e.tipo);
    		if(e.tipo == Quadro and is_ACK(ctrl_byte) and check_SEQ(ctrl_byte,N)){
    			N = !N;
    			set_timeout(TIMEOUT_BACKOFF);
    			_state = BackoffAck;
    		}
    		// ?dataM !ackM app!payload M=M/
    		else if(e.tipo == Quadro and is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)){
    			char buffer_ACK[3];
    			buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
    			buffer_ACK[1] = id_sessao;
    			buffer_ACK[2] = e.ptr[2];
    			_lower->send(buffer_ACK,3);

    			M = !M;
    			_state = WaitAck;

    			char buffer[e.bytes -1];
    			memcpy(buffer,e.ptr + 1,e.bytes -1);
    			_upper->notify(buffer,e.bytes - 1);

    		}
    		// ?dataM/ !ackM/
    		else if(e.tipo == Quadro and is_DATA(ctrl_byte) and not check_SEQ(ctrl_byte,M)){
    			char buffer[3];
    			buffer[0] = M?0x80:0x88; //Quadro de ACK e sequência M/
    			buffer[1] = id_sessao;
    			buffer[2] = e.ptr[2];
    			_lower->send(buffer,3);
    			_state = WaitAck;
    		}
    		// ?timeout or ?ackN/ !dataN/ set_backoff
    		else if((e.tipo == Quadro and is_ACK(ctrl_byte) and not check_SEQ(ctrl_byte,N)) or e.tipo == Timeout){
    			if (retry_counter == 3) {
    				retry_counter = 0;
					disable_timeout();
					_state = Idle;
    				_upper->notifyERR();
    			} else {
    				set_timeout(TIMEOUT_BACKOFF);
					retry_counter++;
					_state = BackoffRelay;
    			}
    		}
    		break;

    	case BackoffAck:
       		//printf("[ARQ] IN BackoffAck %d\n",e.tipo);
    		// backoff/
    		if (e.tipo == Timeout) {
				if(not buffer_ev.empty()){
					Evento a = buffer_ev.front();
					buffer_ev.pop();
					//std::cout<<"[ARQ] Enviando oq faltou\n";
					send_payload(a);
				}else{
					_state = Idle;
					disable_timeout();
				}
    		}
    		// ?dataM !ackM !M=M/
    		else if (e.tipo == Quadro and is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)) {
				char buffer_ACK[3];
				buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
				buffer_ACK[1] = id_sessao;
				buffer_ACK[2] = e.ptr[2];
				_lower->send(buffer_ACK,3);

				M = !M;
				_state = BackoffAck;

    			char buffer[e.bytes -1];
				memcpy(buffer,e.ptr + 1,e.bytes -1);
				_upper->notify(buffer,e.bytes - 1);
    		}
    		break;

    	case BackoffRelay:
       		//printf("[ARQ] IN BackoffRelay %d\n",e.tipo);
    		// ?backoff !dataN !reload_timeout()
    		if (e.tipo == Timeout) {
				_lower->send(buffer_tx,bytes_tx); //passa pro Framming
				reload_timeout();
    			_state = WaitAck;
    		}
    		// ?dataM !ackM app!payload M=M/
    		else if (e.tipo == Quadro and is_DATA(ctrl_byte) and check_SEQ(ctrl_byte,M)) {
				char buffer_ACK[3];
				buffer_ACK[0] = M?0x88:0x80; //Quadro de ACK e sequência M
				buffer_ACK[1] = id_sessao;
				buffer_ACK[2] = e.ptr[2];
				_lower->send(buffer_ACK,3);

				M = !M;
				_state = BackoffRelay;

				char buffer[e.bytes -1];
				memcpy(buffer,e.ptr + 1,e.bytes -1);
				_upper->notify(buffer,e.bytes - 1);

    		}
    		break;
    }
}

void ARQ::notifyERR() {

}
void ARQ::init() {
	if(not _lower->is_enabled()){
		_lower->init();
	}
	enable();
	//printf("ARQ habilitado\n");

}

void ARQ::disable() {
	M = N = 0;
	enabled = false;
}
