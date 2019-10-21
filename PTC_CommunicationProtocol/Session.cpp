/*
 * Session.cpp
 *
 *  Created on: 3 de out de 2019
 *      Author: vini
 */

#include "Session.h"

Session::Session(long tout, uint8_t id_sessao,bool log) : Layer(tout),id(id_sessao),log(log) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();
}

Session::Session(int fd, long tout, uint8_t id_sessao,bool log) : Layer(fd, tout),id(id_sessao),log(log) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();
}

Session::~Session() {}

void Session::init() {
	if(log) log_print("[SESSION] START");
	Evento ev = Evento(Start, nullptr, 0);
	if(not _lower->is_enabled()){
		_lower->init();
	}
	handle_fsm(ev);
}

void Session::close() {
	if(log) log_print("[SESSION] STOP");
	Evento ev = Evento(Stop, nullptr, 0);
	handle_fsm(ev);
}

void Session::send(char *buffer, int bytes) {
	char string[bytes];
	string[bytes-1] = 0;
	memcpy(string,buffer+1,bytes-1);
	if(log) log_print("[SESSION] tx=%s",string);
	Evento ev = Evento(Payload, buffer, bytes);
	handle_fsm(ev);
}

void Session::notify(char * buffer, int len) {
	Evento ev;
	char state[10];
	get_ascii_state(state,_state);

	if(buffer[1] == (char)Session_Proto){
		ev = Evento(Controle, buffer, len);
		char ctrl[3];
		get_ascii_ctrl(ctrl,buffer[2]);
		if(log) log_print("[SESSION] state=%s rx= %s",state,ctrl);

	}else{
		if(log) log_print("[SESSION] state=%s rx= Data",state);
		ev = Evento(Quadro, buffer, len);
	}
	handle_fsm(ev);
}

void Session::notifyERR() {
	char state[10];
	get_ascii_state(state,_state);
	if(log) log_print("[SESSION] state=%s Erro, tentando reconectar...",state);
	Evento ev = Evento(Erro, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle() {}

void Session::handle_timeout() {
	char state[10];
	get_ascii_state(state,_state);
	if(log) log_print("[SESSION] state=%s Timeout!",state);
	Evento ev = Evento(Timeout, nullptr, 1);
	handle_fsm(ev);
}

void Session::handle_fsm(Evento & e) {

	//?Erro --Comportamento padrão para todos os estados
	if(e.tipo == Erro){
		_lower->disable(); // disable pra retornar o numero sequencia do ARQ
		if (_state == HALF1 || _state == HALF2) {
			_state = DISC;
			disable();
		} else {
			// Vai pro HAND1 sem mandar CR. Somente vai enviar novo CR quando
			// ocorrer um timeout
			_state = HAND1;
			reload_timeout();
			enable_timeout();
			_lower->enable();
			char state[10];
			get_ascii_state(state,_state);
		}
		//std::cout <<"Erro no controle de sessão, desconectando...");
		return;
	}

	/*
	 *O timer deste callbeck serve para o keep alive
	 *Start e close são definidos pelo enable/disable
	 *enable e disable do descritor de arquivo(não existente neste callbeck) são utilizados para monitorar o estado de conexão
	 */

	switch (_state) {
	case DISC:
		//?start !CR ->HAND1
		if(e.tipo == Start){
			if(log) log_print("[SESSION] Tentando conectar...");
			_state = HAND1;
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
		}
		break;
	case HAND1:
		//?CR !CC ->HAND2
		if(e.tipo==Controle and e.ptr[2] == CR){
			char buffer[3] = {id,(char)Session_Proto,CC};
			_state = HAND2;
			_lower->send(buffer,3);
			reload_timeout();
			enable_timeout();
		}
		//?CC ->HAND3
		else if(e.tipo==Controle and e.ptr[2] == CC){
			_state = HAND3;
			reload_timeout();
			enable_timeout();
		} else if (e.tipo==Timeout) {
			_state = HAND1;

			// ! CR
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
		}
		break;
	case HAND2:
		//?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
		}

		//?CC ->CON
		else if(e.tipo==Controle and e.ptr[2] == CC){
			if(log) log_print("[SESSION] Conectou!");
			_state = CON;
			enable_timeout();
			enable();
		}

		break;
	case HAND3:
		//?CR !CC ->CON
		if(e.tipo==Controle and e.ptr[2] == CR){
			if(log) log_print("[SESSION] Conectou!");
			_state = CON;
			enable();
			enable_timeout();
			char buffer[3] = {id,(char)Session_Proto,CC};
			_lower->send(buffer,3);
		}
		break;
	case CON:
		//?close !DR ->HALF1
		if(e.tipo == Stop){
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			_state = HALF1;
			disable_timeout();
		}
		// app?payload !DATA ->CON
		else if(e.tipo == Payload){
			_state = CON;
			reload_timeout();
			char buffer[e.bytes+1];
			buffer[0]=id;
			memcpy(buffer+1,e.ptr,e.bytes);
			_lower->send(buffer,e.bytes+1);
		}
		// ?DATA app!payload -> CON
		else if(e.tipo==Quadro){
			_state = CON;
			reload_timeout();
			_upper->notify(e.ptr+1,e.bytes-1);
		}
		// ?KR !KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CON;
			reload_timeout();
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
		}
		// ?DR !DR -> HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			disable_timeout();
		}
		// ?checkInterval !KR
		if(e.tipo == Timeout){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KR};
			_lower->send(buffer,3);
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
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
		}
		// ?DATA app!payload ->CON
		else if(e.tipo==Quadro){
			_state = CON;
			enable_timeout();
			_upper->notify(e.ptr+1,e.bytes-1);
		}
		//?KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KC){
			enable_timeout();
			_state = CON;
		}
		// ?DR !DR->HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
		}
		break;
	case HALF1:
		//?data app!payload ->HALF1
		if(e.tipo==Quadro){
			_state = HALF1;
			_upper->notify(e.ptr+1,e.bytes-1);
		}
		//?KR !DR ->HALF1
		else if(e.tipo==Controle and e.ptr[2] == KR){
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			_state = HALF1;
		}
		//?DR !DC ->DISC
		else if(e.tipo==Controle and e.ptr[2] == DR){
			char buffer[3] = {id,(char)Session_Proto,DC};
			_lower->send(buffer,3);
			disable();
			_state = DISC;
			if(log) log_print("[SESSION] DESCONECTANDO");
		}

		break;
	case HALF2:
		// ?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
		}
		//?DC -> DISC
		else if(e.tipo==Controle and e.ptr[2] == DC){
			disable();
			_state = DISC;
			if(log) log_print("[SESSION] DESCONECTANDO");
		}
		break;
	}
}

void Session::get_ascii_ctrl(char* ascii,char byte){
	switch(byte){
	case CR:
		sprintf(ascii,"CR");
		break;
	case CC:
		sprintf(ascii,"CC");
		break;
	case KR:
		sprintf(ascii,"KR");
		break;
	case KC:
		sprintf(ascii,"KC");
		break;
	case DR:
		sprintf(ascii,"DR");
		break;
	case DC:
		sprintf(ascii,"DC");
		break;
	default:
		sprintf(ascii,"??");
		break;
	}

}

void Session::get_ascii_state(char* ascii,char state){
	switch(state){
	case DISC:
		sprintf(ascii,"DISC");
		break;
	case HAND1:
		sprintf(ascii,"HAND1");
		break;
	case HAND2:
		sprintf(ascii,"HAND2");
		break;
	case HAND3:
		sprintf(ascii,"HAND3");
		break;
	case CON:
		sprintf(ascii,"CON");
		break;
	case CHECK:
		sprintf(ascii,"CHECK");
		break;
	case HALF1:
		sprintf(ascii,"HALF1");
		break;
	case HALF2:
		sprintf(ascii,"HALF2");
		break;
	default:
		sprintf(ascii,"??");
		break;
	}

}

