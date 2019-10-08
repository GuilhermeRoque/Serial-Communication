/*
 * Session.cpp
 *
 *  Created on: 3 de out de 2019
 *      Author: vini
 */

#include "Session.h"

Session::Session(int fd, long tout) : Layer(fd, tout) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();
}

Session::Session(long tout) : Layer(tout) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();

}

Session::~Session() {
	// TODO Auto-generated destructor stub
}

void Session::init() {
	Evento ev = Evento(Start, nullptr, 0);
	handle_fsm(ev);
}

void Session::close() {
	Evento ev = Evento(Stop, nullptr, 0);
	handle_fsm(ev);
}

void Session::send(char *buffer, int bytes) {
	Evento ev = Evento(Payload, buffer, bytes);
	handle_fsm(ev);
}

void Session::notify(char * buffer, int len) {
	Evento ev;
	//buffer[2] = id_proto
	if(buffer[2] == 255){
		ev = Evento(Controle, buffer, len);
	}else{
		ev = Evento(Quadro, buffer, len);
	}
	handle_fsm(ev);
}

void Session::notifyERR() {
	Evento ev = Evento(Erro, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle() {}

void Session::handle_timeout() {
	Evento ev = Evento(Timeout, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle_fsm(Evento & e) {

	//?Erro --Comportamento padrão para todos os estados
	if(e.tipo == Erro){
		_state = DISC;
		disable();
		std::cout <<"Erro no controle de sessão, desconectando...\n";
		return;
	}

	/*
	 *O timer deste callbeck serve para o keep alive
	 *Start e close são definidos pelo enable/disable timeout
	 *enable e disable do descritor de arquivo(não existente neste callbeck) são utilizados para monitorar o estado de conexão
	 */

	switch (_state) {
	case DISC:
		//?start !CR ->HAND1
		if(e.tipo == Start){
			std::cout<<"Tentando conectar...\n";
			_state = HAND1;
			char buffer[1] = {CR};
			_lower->send(buffer,1);
		}
		break;
	case HAND1:
		//?CR !CC ->HAND2
		if(e.tipo==Controle and e.ptr[3] == CR){
			_state = HAND2;
			char buffer[1] = {CC};
			_lower->send(buffer,1);
		}
		//?CC ->HAND3
		else if(e.tipo==Controle and e.ptr[3] == CC){
			_state = HAND3;
		}
		break;
	case HAND2:
		//?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[3] == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}

		//?DATA ?CC ->CON
		else if(e.tipo==Quadro or (e.tipo==Controle and e.ptr[3] == CC)){
			_state = CON;
			enable_timeout();
			enable();
		}

		break;
	case HAND3:
		//?CR !CC ->CON
		if(e.tipo==Controle and e.ptr[3] == CR){
			_state = CON;
			enable();
			enable_timeout();
			char buffer[1] = {CC};
			_lower->send(buffer,1);
		}
		break;
	case CON:
		//?close !DR ->HALF1
		if(e.tipo == Stop){
			char buffer[1] = {DR};
			_lower->send(buffer,1);
			_state = HALF1;
			disable_timeout();
		}
		// app?payload !DATA ->CON
		else if(e.tipo == Payload){
			_state = CON;
			reload_timeout();
			_lower->send(e.ptr,e.bytes);
		}
		// ?DATA app!payload -> CON
		else if(e.tipo==Quadro){
			_state = CON;
			reload_timeout();
			_upper->notify(e.ptr,e.bytes);
		}
		// ?KR !KC ->CON
		else if(e.tipo==Controle and e.ptr[3] == KR){
			_state = CON;
			reload_timeout();
			char buffer[1] = {KC};
			_lower->send(buffer,1);
		}
		// ?DR !DR -> HALF2
		else if(e.tipo==Quadro and e.ptr[3] == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
			disable_timeout();

		}
		// ?checkInterval !KR
		if(e.tipo == Timeout){
			_state = CHECK;
			char buffer[1] = {KR};
			_lower->send(buffer,1);
			disable_timeout();
		}
		break;
	case CHECK:
		// app?payload !DATA ->CHECK
		if(e.tipo == Payload){
			_state = CHECK;
			_lower->send(e.ptr,e.bytes);
		}
		// ?KR !KC ->CHECK
		else if(e.tipo==Controle and e.ptr[3] == KR){
			_state = CHECK;
			char buffer[1] = {KC};
			_lower->send(buffer,1);
		}
		// ?DATA app!payload ->CON
		else if(e.tipo==Quadro){
			_state = CON;
			enable_timeout();
			_upper->notify(e.ptr,e.bytes);
		}
		//?KC ->CON
		else if(e.tipo==Controle and e.ptr[3] == KC){
			enable_timeout();
			_state = CON;
		}
		// ?DR !DR->HALF2
		else if(e.tipo==Controle and e.ptr[3] == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}
		break;
	case HALF1:
		//?data app!payload ->HALF1
		if(e.tipo==Quadro){
			_state = HALF1;
			_upper->notify(e.ptr,e.bytes);
		}
		//?KR !DR ->HALF1
		else if(e.tipo==Controle and e.ptr[3] == KR){
			char buffer[1] = {DR};
			_lower->send(buffer,1);
			_state = HALF1;
		}
		//?DR !DC ->DISC
		else if(e.tipo==Controle and e.ptr[3] == DR){
			char buffer[1] = {DC};
			_lower->send(buffer,1);
			disable();
			_state = DISC;
		}

		break;
	case HALF2:
		// ?DR !DR ->HALF2
		if(e.tipo==Quadro and e.ptr[3] == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}
		//?DC
		else if(e.tipo==Quadro and e.ptr[3] == DC){
			disable();
			_state = DISC;
		}
		break;
	}
}

