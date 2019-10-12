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
	srand (time(NULL));
	id = rand()%0xFF;
}

Session::Session(long tout) : Layer(tout) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();
	srand (time(NULL));
	id = rand();
}

Session::~Session() {}

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
	std::cout<<"Sessao recebeu ";
	//buffer[2] = id_proto
	if(buffer[1] == (char)Session_Proto){
		ev = Evento(Controle, buffer, len);
		std::cout<<"Controle: ";
		print_buffer(buffer,len);

	}else{
		std::cout<<"Quadro: ";
		print_buffer(buffer,len);
		ev = Evento(Quadro, buffer, len);
	}
	handle_fsm(ev);
}

void Session::notifyERR() {
	std::cout<<"Sessao recebeu erro\n";
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
	 *Start e close são definidos pelo enable/disable
	 *enable e disable do descritor de arquivo(não existente neste callbeck) são utilizados para monitorar o estado de conexão
	 */

	switch (_state) {
	case DISC:
		//?start !CR ->HAND1
		std::cout<<"IN DISC\n";
		if(e.tipo == Start){
			std::cout<<"Tentando conectar...\n";
			_state = HAND1;
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
		}
		break;
	case HAND1:
		//?CR !CC ->HAND2
		std::cout<<"IN HAND1\n";
		if(e.tipo==Controle and e.ptr[2] == CR){
			_state = HAND2;
			char buffer[3] = {id,(char)Session_Proto,CC};
			_lower->send(buffer,3);
			std::cout<<"GOTO HAND2\n";
		}
		//?CC ->HAND3
		else if(e.tipo==Controle and e.ptr[2] == CC){
			_state = HAND3;
			std::cout<<"GOTO HAND3\n";
		}
		break;
	case HAND2:
		std::cout<<"IN HAND2\n";
		//?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			std::cout<<"GOTO HALF2\n";
		}

		//?CC ->CON
		else if(e.tipo==Controle and e.ptr[2] == CC){
			_state = CON;
			enable_timeout();
			enable();
			std::cout<<"GOTO CON\n";
		}

		break;
	case HAND3:
		std::cout<<"IN HAND3\n";
		//?CR !CC ->CON
		if(e.tipo==Controle and e.ptr[2] == CR){
			_state = CON;
			enable();
			enable_timeout();
			char buffer[3] = {id,(char)Session_Proto,CC};
			_lower->send(buffer,3);
			std::cout<<"GOTO CON\n";
		}
		break;
	case CON:
		std::cout<<"IN CON\n";
		//?close !DR ->HALF1
		if(e.tipo == Stop){
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			_state = HALF1;
			disable_timeout();
			std::cout<<"GOTO HALF1\n";
		}
		// app?payload !DATA ->CON
		else if(e.tipo == Payload){
			_state = CON;
			reload_timeout();
			_lower->send(e.ptr,e.bytes);
			std::cout<<"GOTO CON 1\n";
		}
		// ?DATA app!payload -> CON
		else if(e.tipo==Quadro){
			_state = CON;
			reload_timeout();
			_upper->notify(e.ptr+1,e.bytes-1);
			std::cout<<"GOTO CON 2\n";
		}
		// ?KR !KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CON;
			reload_timeout();
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
			std::cout<<"GOTO CON 3\n";
		}
		// ?DR !DR -> HALF2
		else if(e.tipo==Quadro and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			disable_timeout();
			std::cout<<"GOTO HALF2\n";

		}
		// ?checkInterval !KR
		if(e.tipo == Timeout){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KR};
			_lower->send(buffer,3);
			disable_timeout();
			std::cout<<"GOTO CHECK\n";
		}
		break;
	case CHECK:
		std::cout<<"IN CHECK\n";
		// app?payload !DATA ->CHECK
		if(e.tipo == Payload){
			_state = CHECK;
			_lower->send(e.ptr,e.bytes);
			std::cout<<"GOTO CHECK\n";
		}
		// ?KR !KC ->CHECK
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
			std::cout<<"GOTO CHECK\n";
		}
		// ?DATA app!payload ->CON
		else if(e.tipo==Quadro){
			_state = CON;
			enable_timeout();
			_upper->notify(e.ptr+1,e.bytes-1);
			std::cout<<"GOTO CON\n";
		}
		//?KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KC){
			enable_timeout();
			_state = CON;
			std::cout<<"GOTO CON 2\n";
		}
		// ?DR !DR->HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			std::cout<<"GOTO HALF 2\n";
		}
		break;
	case HALF1:
		std::cout<<"IN HALF 1\n";
		//?data app!payload ->HALF1
		if(e.tipo==Quadro){
			_state = HALF1;
			_upper->notify(e.ptr+1,e.bytes-1);
			std::cout<<"GOTO HALF1\n";
		}
		//?KR !DR ->HALF1
		else if(e.tipo==Controle and e.ptr[2] == KR){
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			_state = HALF1;
			std::cout<<"GOTO HALF1 2\n";
		}
		//?DR !DC ->DISC
		else if(e.tipo==Controle and e.ptr[2] == DR){
			char buffer[3] = {id,(char)Session_Proto,DC};
			_lower->send(buffer,3);
			disable();
			_state = DISC;
			std::cout<<"GOTO DISC\n";
		}

		break;
	case HALF2:
		std::cout<<"IN HALF2\n";
		// ?DR !DR ->HALF2
		if(e.tipo==Quadro and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			std::cout<<"GOTO HALF2\n";
		}
		//?DC -> DISC
		else if(e.tipo==Quadro and e.ptr[2] == DC){
			disable();
			_state = DISC;
			std::cout<<"GOTO DISC\n";
		}
		break;
	}
}

