/*
 * App.cpp
 *
 *  Created on: 21 de set de 2019
 *      Author: guilherme
 */

#include "App.h"

App::App(int tty_fd,long tout): tty_fd(tty_fd),Layer(tty_fd, tout){
	disable_timeout();

}

App::~App() {
}


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
		this->_lower->send(buffer,n-1);
	}
	else{
		printf("Erro: Não há conexão\n");
		_lower->enable_timeout();
	}
}

void App::handle_timeout(){}