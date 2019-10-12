/*
 * App.cpp
 *
 *  Created on: 21 de set de 2019
 *      Author: guilherme
 */

#include "App.h"

App::App(int tty_fd,long tout): tty_fd(tty_fd),Layer(tty_fd, tout){
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
	int n = read(tty_fd, buffer, 1024);
	if(_lower->is_enabled()){
		if(n-1 == 4 and buffer[0] == 'q' and buffer[1] == 'u' and buffer[2] == 'i' and buffer[3] == 't'){
			_lower->close();
		}
		else{
			this->_lower->send(buffer,n-1);
		}
	}
	else{
		printf("Erro: Não há conexão\n");
	}
}

void App::handle_timeout(){
	if(not _lower->is_enabled()){
		_lower->init();
	}else{
		disable_timeout();
	}
}

void App::notifyERR() {
	printf("APP ERROR!\n");
}
