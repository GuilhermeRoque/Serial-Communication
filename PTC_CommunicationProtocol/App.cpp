/*
 * App.cpp
 *
 *  Created on: 21 de set de 2019
 *      Author: guilherme
 */

#include "App.h"

App::App(int tty_fd,long tout): tty_fd(tty_fd),Layer(tty_fd, tout){
	enable_timeout();
}

App::~App() {}

void App::send(char * buffer, int bytes){}

void App::notify(char * buffer, int len){
	//-----------para debug apenas
		printf("App recebeu: ");
	    print_buffer(buffer,len);
	//----------------------------
}

void App::handle(){
	buffer[0] = 0x00;
	int n = read(tty_fd, buffer+1, 1023);
	n++;
	if(_lower->is_enabled()){
		if(n-1 == 5 and buffer[1] == 'q' and buffer[2] == 'u' and buffer[3] == 'i' and buffer[4] == 't'){
			_lower->close();
		}
		else{
			this->_lower->send(buffer,n-1);
		}
	}
	else{
		printf("Erro: Não há conexão\n");
		this->enable_timeout();
	}
}

void App::handle_timeout(){
	if(not _lower->is_enabled()){
		_lower->init();
	}else{
		printf("Conectado!!\n");
		disable_timeout();
	}
}

void App::notifyERR() {
	printf("APP ERROR!\n");
}
