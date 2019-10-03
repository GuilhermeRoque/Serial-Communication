/*
 * Session.h
 *
 *  Created on: 3 de out de 2019
 *      Author: vini
 */

#ifndef SESSION_H_
#define SESSION_H_

#include "Layer.h"

class Session: public Layer {
public:
	Session(int fd, long tout);
	Session(long tout);
	virtual ~Session();

    void send(char * buffer, int bytes);

    void notify(char * buffer, int len);
    void notifyERR();

    void handle();
    void handle_timeout();
    void init();
private:
	enum TipoEvento {Payload, Quadro, Timeout, Controle, Erro};
	enum States {DISC, HAND1, HAND2, HAND3, CON, CHECK, HALF1, HALF2};
	States _state;
	char buffer_tx[1026];
	int bytes_tx;

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
