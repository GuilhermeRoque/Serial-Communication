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
	char state[10];
	get_ascii_state(state,_state);
	if(log) log_print("[SESSION] state=%s START!",state);
	Evento ev = Evento(Start, nullptr, 0);
	if(not _lower->is_enabled()){
		_lower->init();
	}
	handle_fsm(ev);
}

void Session::close() {
	char state[10];
	get_ascii_state(state,_state);
	if(log) log_print("[SESSION] state=%s STOP!",state);
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
		if(log) log_print("[SESSION] state=%s rx=%s",state,ctrl);

	}else{
		if(log) log_print("[SESSION] state=%s rx=Data",state);
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

	switch (_state) {
	case DISC:
		//?start !CR ->HAND1
		if(e.tipo == Start){
			_state = HAND1;
			if(log) log_print("[SESSION] state=DISC tx=CR");
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
			reload_timeout();
			enable_timeout();
		}
		break;
	case HAND1:
		//?CR !CC ->HAND2
		if(e.tipo==Controle and e.ptr[2] == CR){
			_state = HAND2;
			if(log) log_print("[SESSION] state=HAND1 tx=CC");
			char buffer[3] = {id,(char)Session_Proto,CC};
			_lower->send(buffer,3);
			reload_timeout();
		}
		//?CC ->HAND3
		else if(e.tipo==Controle and e.ptr[2] == CC){
			_state = HAND3;
			reload_timeout();
		}
		//?TIMEOUT  !CR ->HAND1
		else if (e.tipo==Timeout or e.tipo==Erro) {
			_state = HAND1;
			if(log) log_print("[SESSION] state=HAND1 tx=CR");
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
			reload_timeout();
		}
		break;
	case HAND2:
		//?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			if(log) log_print("[SESSION] state=HAND2 tx=DR");
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			reload_timeout();

		}

		//?CC ->CON
		else if(e.tipo==Controle and e.ptr[2] == CC){
			if(log) log_print("[SESSION] Conectou!");
			_state = CON;
			reload_timeout();
			enable();
		}
		//?TIMEOUT  !CR ->HAND1
		else if (e.tipo==Timeout or e.tipo==Erro) {
			_state = HAND1;
			if(log) log_print("[SESSION] state=HAND2 tx=CR");
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
			reload_timeout();
		}
		break;
	case HAND3:
		//?CR !CC ->CON
		if(e.tipo==Controle and e.ptr[2] == CR){
			_state = CON;
			if(log) log_print("[SESSION] state=HAND3 tx=CC Conectou!");
			char buffer[3] = {id,(char)Session_Proto,CC};
			_lower->send(buffer,3);
			enable();
			reload_timeout();
		}
		//?TIMEOUT  !CR ->HAND1
		else if (e.tipo==Timeout or e.tipo==Erro) {
			_state = HAND1;
			if(log) log_print("[SESSION] state=HAND3 tx=CR");
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
			reload_timeout();
		}
		break;
	case CON:
		//?close !DR ->HALF1
		if(e.tipo == Stop){
			_state = HALF1;
			if(log) log_print("[SESSION] state=CON tx=DR");
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			reload_timeout();
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
			if(log) log_print("[SESSION] state=CON tx=KC");
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
		}
		// ?DR !DR -> HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			if(log) log_print("[SESSION] state=CON tx=DR");
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			reload_timeout();
		}
		// ?checkInterval !KR
		else if(e.tipo == Timeout){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KR};
			if(log) log_print("[SESSION] state=CON tx=KR");
			_lower->send(buffer,3);
			reload_timeout();
		}
		//?Erro  !CR ->HAND1
		else if (e.tipo==Erro) {
			_state = HAND1;
			if(log) log_print("[SESSION] state=CON tx=CR");
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
			reload_timeout();
		}
		break;
	case CHECK:
		// app?payload !DATA ->CHECK
		if(e.tipo == Payload){
			_state = CHECK;
			char buffer[e.bytes+1];
			buffer[0]=id;
			memcpy(buffer+1,e.ptr,e.bytes);
			_lower->send(buffer,e.bytes+1);
		}
		// ?KR !KC ->CHECK
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CHECK;
			if(log) log_print("[SESSION] state=CHECK tx=KC");
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
		}
		// ?DATA app!payload ->CON
		else if(e.tipo==Quadro){
			_state = CON;
			_upper->notify(e.ptr+1,e.bytes-1);
			reload_timeout();
		}
		//?KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KC){
			reload_timeout();
			_state = CON;
		}
		// ?DR !DR->HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			if(log) log_print("[SESSION] state=CHECK tx=DR");
			_lower->send(buffer,3);
			reload_timeout();
		}
		//?TIMEOUT  !CR ->HAND1
		else if (e.tipo==Timeout or e.tipo==Erro) {
			_state = HAND1;
			if(log) log_print("[SESSION] state=CHECK tx=CR");
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
			reload_timeout();
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
			if(log) log_print("[SESSION] state=HALF1 tx=DR");
			_lower->send(buffer,3);
			_state = HALF1;
		}
		//?DR !DC ->DISC
		else if(e.tipo==Controle and e.ptr[2] == DR){
			char buffer[3] = {id,(char)Session_Proto,DC};
			if(log) log_print("[SESSION] state=HALF1 tx=DC");
			_lower->send(buffer,3);
			disable_timeout();
			disable();
			_state = DISC;
			if(log) log_print("[SESSION] DESCONECTADO");
		}
		//?TIMEOUT ->DISC
		else if (e.tipo==Timeout or e.tipo==Erro) {
			_state = DISC;
			if(log) log_print("[SESSION] state=HALF1 DESCONECTADO");
			disable();
			disable_timeout();
		}
		break;
	case HALF2:
		// ?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			if(log) log_print("[SESSION] state=HALF2 tx=DR");
			_lower->send(buffer,3);
		}
		//?DC -> DISC
		else if(e.tipo==Controle and e.ptr[2] == DC){
			disable();
			disable_timeout();
			_state = DISC;
			if(log) log_print("[SESSION] DESCONECTADO");
		}
		//?TIMEOUT ->DISC
		else if (e.tipo==Timeout or e.tipo==Erro) {
			_state = DISC;
			if(log) log_print("[SESSION] state=HALF2 DESCONECTADO");
			disable();
			disable_timeout();
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

