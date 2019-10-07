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

void Session::handle() {}

void Session::handle_timeout() {
	Evento ev = Evento(Timeout, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle_fsm(Evento & e) {

	//HELP VARIABLES
	uint8_t id_Proto;
	uint8_t session_act;
	bool is_session;
	bool session_frame;
	bool data_frame;

	//?Erro --Comportamento padrão para todos os estados
	if(e.tipo == Erro){
		_state = DISC;
		disable();
		std::cout <<"Erro no controle de sessão, desconectando...\n";
		return;
	}
	else if(e.tipo != Timeout){
		//HELP VARIABLES
		id_Proto = e.ptr[2];
		session_act = e.ptr[4];
		is_session = id_Proto == 255;
		session_frame = e.tipo == Quadro and is_session;
		data_frame = e.tipo == Quadro and not is_session;
	}


	/*Fora do estado CON o timer deste callbeck serve para tentar conexão
	 *No estado CON o timer deste callbeck serve para o keep alive
	 *Start e close são definidos pelo enable/disable timeout
	 *enable e disable do descritor de arquivo(não existente neste callbeck) são utilizados para
	 *monitorar o estado de conexão
	 */
	switch (_state) {
	case DISC:
		//start
		if(e.tipo == Timeout){
			std::cout<<"Tentando conectar...\n";
			_state = HAND1;
			char buffer[1] = {CR};
			_lower->send(buffer,1);
		}
		break;
	case HAND1:
		//?CR !CC
		if(session_frame and session_act == CR){
			_state = HAND2;
			char buffer[1] = {CC};
			_lower->send(buffer,1);
		}
		//?CC
		else if(session_frame and session_act == CC){
			_state = HAND3;
		}
		break;
	case HAND2:
		//?DR !DR
		if(session_frame and session_act == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}

		//?DATA ?CC
		else if(data_frame or (session_frame and session_act == CC)){
			_state = CON;
			reload_timeout();
			enable();
		}

		break;
	case HAND3:
		//?CR !CC
		if(session_frame and session_act == CR){
			_state = CON;
			enable();
			reload_timeout();
			char buffer[1] = {CC};
			_lower->send(buffer,1);
		}
		break;
	case CON:
		//?close !DR
		if(not timeout_enabled()){
			char buffer[1] = {DR};
			_lower->send(buffer,1);
			_state = HALF1;
		}
		// app?payload !DATA
		else if(e.tipo == Payload){
			_state = CON;
			reload_timeout();
			_lower->send(e.ptr,e.bytes);
		}
		// ?DATA app!payload
		else if(data_frame){
			_state = CON;
			reload_timeout();
			_upper->notify(e.ptr,e.bytes);
		}
		// ?KR !KC
		else if(data_frame and session_act == KR){
			_state = CON;
			reload_timeout();
			char buffer[1] = {KC};
			_lower->send(buffer,1);
		}
		// ?DR !DR
		else if(data_frame and session_act == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}
		// ?checkInterval !KR
		if(e.tipo == Timeout){
			_state = CHECK;
			char buffer[1] = {KR};
			_lower->send(buffer,1);
		}
		break;
	case CHECK:
		// app?payload !DATA
		if(e.tipo == Payload){
			_state = CHECK;
			_lower->send(e.ptr,e.bytes);
		}
		// ?KR !KC
		else if(session_frame and session_act == KR){
			_state = CON;
			reload_timeout();
			char buffer[1] = {KC};
			_lower->send(buffer,1);
		}
		// ?DATA app!payload
		else if(data_frame){
			_state = CON;
			reload_timeout();
			_upper->notify(e.ptr,e.bytes);
		}
		//?KC
		else if(session_frame and session_act == KC){
			enable();
			reload_timeout();
		}
		// ?DR !DR
		else if(data_frame and session_act == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}
		break;
	case HALF1:
		if(data_frame){
			_state = HALF1;
			_upper->notify(e.ptr,e.bytes);
		}
		if(session_frame and session_act == KR){
			char buffer[1] = {DR};
			_lower->send(buffer,1);
			_state = HALF1;
		}

		break;
	case HALF2:
		// ?DR !DR
		if(data_frame and session_act == DR){
			_state = HALF2;
			char buffer[1] = {DR};
			_lower->send(buffer,1);
		}
		//?DC
		if(data_frame and session_act == DC){
			_state = DISC;
			disable();
		}
		break;
	}
}

