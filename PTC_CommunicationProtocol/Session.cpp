/*
 * Session.cpp
 *
 *  Created on: 3 de out de 2019
 *      Author: vini
 */

#include "Session.h"
#include <boost/date_time/posix_time/posix_time.hpp>

Session::Session(int fd, long tout) : Layer(fd, tout) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();
	srand (time(NULL));
	//id = rand()%0xFF;
//	id = 0xAA;
}

Session::Session(long tout) : Layer(tout) {
	bytes_tx = 0;
	_state = DISC;
	disable();
	disable_timeout();
	//id = rand()%0xFF;
//	id = 0xAA;
}

Session::Session(int fd, long tout, uint8_t id_sessao) : Session(fd, tout) {
	id = (char) id_sessao;
}

Session::~Session() {}

void Session::init() {
	log_print("[SESSION]START sessao");
	Evento ev = Evento(Start, nullptr, 0);
	if(not _lower->is_enabled()){
		_lower->init();
	}
	handle_fsm(ev);
}

void Session::close() {
	log_print("[SESSION]STOP sessao");
	Evento ev = Evento(Stop, nullptr, 0);
	handle_fsm(ev);
}

void Session::send(char *buffer, int bytes) {
	Evento ev = Evento(Payload, buffer, bytes);
	handle_fsm(ev);
}

void Session::notify(char * buffer, int len) {
	Evento ev;
	log_print("[SESSION]Sessao recebeu ");
	//buffer[2] = id_proto
	if(buffer[1] == (char)Session_Proto){
		ev = Evento(Controle, buffer, len);
		log_print("Controle:");
		////print_buffer(buffer,len);

	}else{
		log_print("Quadro:");
		////print_buffer(buffer,len);
		ev = Evento(Quadro, buffer, len);
	}
	handle_fsm(ev);
}

void Session::notifyERR() {
	log_print("[SESSION]Sessao recebeu erro");
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
		_lower->disable();
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
		log_print("[SESSION]IN DISC");
		if(e.tipo == Start){
			log_print("Tentando conectar...");
			_state = HAND1;
			char buffer[3] = {id,(char)Session_Proto,CR};
			_lower->send(buffer,3);
		}
		break;
	case HAND1:
		log_print("[SESSION]IN HAND1");
		//?CR !CC ->HAND2
		if(e.tipo==Controle and e.ptr[2] == CR){
			char buffer[3] = {id,(char)Session_Proto,CC};
			_state = HAND2;
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO HAND2");
		}
		//?CC ->HAND3
		else if(e.tipo==Controle and e.ptr[2] == CC){
			_state = HAND3;
			log_print("[SESSION]GOTO HAND3");
		}
		break;
	case HAND2:
		//?DR !DR ->HALF2
		log_print("[SESSION]IN HAND2");
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO HALF2");
		}

		//?CC ->CON
		else if(e.tipo==Controle and e.ptr[2] == CC){
			_state = CON;
			enable_timeout();
			enable();
			log_print("[SESSION]GOTO CON");
		}

		break;
	case HAND3:
		log_print("[SESSION]IN HAND3");
		//?CR !CC ->CON
		if(e.tipo==Controle and e.ptr[2] == CR){
			_state = CON;
			enable();
			enable_timeout();
			char buffer[3] = {id,(char)Session_Proto,CC};
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO CON");
		}
		break;
	case CON:
		log_print("[SESSION]IN CON");
		//?close !DR ->HALF1
		if(e.tipo == Stop){
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			_state = HALF1;
			disable_timeout();
			log_print("[SESSION]GOTO HALF1");
		}
		// app?payload !DATA ->CON
		else if(e.tipo == Payload){
			_state = CON;
			reload_timeout();
			char buffer[e.bytes+1];
			buffer[0]=id;
			memcpy(buffer+1,e.ptr,e.bytes);
			_lower->send(buffer,e.bytes+1);
			log_print("[SESSION]GOTO CON 1");
		}
		// ?DATA app!payload -> CON
		else if(e.tipo==Quadro){
			_state = CON;
			reload_timeout();
			_upper->notify(e.ptr+1,e.bytes-1);
			log_print("[SESSION]GOTO CON 2");
		}
		// ?KR !KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CON;
			reload_timeout();
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO CON 3");
		}
		// ?DR !DR -> HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			disable_timeout();
			log_print("[SESSION]GOTO HALF2");

		}
		// ?checkInterval !KR
		if(e.tipo == Timeout){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KR};
			_lower->send(buffer,3);
			disable_timeout();
			log_print("[SESSION]GOTO CHECK");
		}
		break;
	case CHECK:
		log_print("[SESSION]IN CHECK");
		// app?payload !DATA ->CHECK
		if(e.tipo == Payload){
			_state = CHECK;
			_lower->send(e.ptr,e.bytes);
			log_print("[SESSION]GOTO CHECK");
		}
		// ?KR !KC ->CHECK
		else if(e.tipo==Controle and e.ptr[2] == KR){
			_state = CHECK;
			char buffer[3] = {id,(char)Session_Proto,KC};
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO CHECK");
		}
		// ?DATA app!payload ->CON
		else if(e.tipo==Quadro){
			_state = CON;
			enable_timeout();
			_upper->notify(e.ptr+1,e.bytes-1);
			log_print("[SESSION]GOTO CON");
		}
		//?KC ->CON
		else if(e.tipo==Controle and e.ptr[2] == KC){
			enable_timeout();
			_state = CON;
			log_print("[SESSION]GOTO CON 2");
		}
		// ?DR !DR->HALF2
		else if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO HALF 2");
		}
		break;
	case HALF1:
		log_print("[SESSION]IN HALF 1");
		//?data app!payload ->HALF1
		if(e.tipo==Quadro){
			_state = HALF1;
			_upper->notify(e.ptr+1,e.bytes-1);
			log_print("[SESSION]GOTO HALF1");
		}
		//?KR !DR ->HALF1
		else if(e.tipo==Controle and e.ptr[2] == KR){
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			_state = HALF1;
			log_print("[SESSION]GOTO HALF1 2");
		}
		//?DR !DC ->DISC
		else if(e.tipo==Controle and e.ptr[2] == DR){
			char buffer[3] = {id,(char)Session_Proto,DC};
			_lower->send(buffer,3);
			disable();
			_state = DISC;
			log_print("DESCONECTANDO");
		}

		break;
	case HALF2:
		log_print("[SESSION]IN HALF2 ");
		// ?DR !DR ->HALF2
		if(e.tipo==Controle and e.ptr[2] == DR){
			_state = HALF2;
			char buffer[3] = {id,(char)Session_Proto,DR};
			_lower->send(buffer,3);
			log_print("[SESSION]GOTO HALF2");
		}
		//?DC -> DISC
		else if(e.tipo==Controle and e.ptr[2] == DC){
			disable();
			_state = DISC;
			log_print("DESCONECTANDO");
		}
		break;
	}
}

void Session::log_print(char *message) {
    const boost::posix_time::ptime now =
        boost::posix_time::microsec_clock::local_time();

    const boost::posix_time::time_duration td = now.time_of_day();

    const long hours        = td.hours();
    const long minutes      = td.minutes();
    const long seconds      = td.seconds();
    const long milliseconds = td.total_milliseconds() -
                              ((hours * 3600 + minutes * 60 + seconds) * 1000);

    char buf[40];
    sprintf(buf, "%02ld:%02ld:%02ld.%03ld",
        hours, minutes, seconds, milliseconds);

    printf("%s %s\n", buf, message);
}

