/*
 * Session.h
 *
 *  Created on: 3 de out de 2019
 *      Author: vini
 */

#ifndef SESSION_H_
#define SESSION_H_

#include "Layer.h"
#include <iostream>
#include "utils.h"
#define Session_Proto 0xFF

class Session: public Layer {
public:
	Session(int fd, long tout);
	Session(long tout);
	virtual ~Session();

    void send(char * buffer, int bytes);

    void notify(char * buffer, int len);
    void notifyERR();
    void notifyStart();
    void notifyStop();

    void handle();
    void handle_timeout();
    void init();
    void close();

private:
	enum TipoEvento {Payload, Quadro, Timeout, Controle,Erro,Start,Stop};
	enum States {DISC, HAND1, HAND2, HAND3, CON, CHECK, HALF1, HALF2};
	enum SessionAct {CR,CC,KR,KC,DR,DC};
	States _state;
	char buffer_tx[1026];
	int bytes_tx;
	char id;

 // esta struct descreve um Evento
 struct Evento {
	TipoEvento tipo;
	char * ptr;
	int bytes;

	// construtor sem parâmetros: cria um Evento Timeout
	Evento() { tipo = Timeout;}

	// construtor com parâmetros: cria um evento Payload ou Quadro
	Evento(TipoEvento t, char * p, int len) : tipo(t), ptr(p), bytes(len) {}
 };

 	// executa a MEF, passando como parâmetro um evento
	void handle_fsm(Evento & e);
};

#endif /* SESSION_H_ */
