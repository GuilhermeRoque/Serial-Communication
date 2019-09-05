/*
 * ARQ.h
 *
 *  Created on: 5 de set de 2019
 *      Author: guilherme
 */

#ifndef ARQ_H_
#define ARQ_H_
#include "Layer.h"
#include "Framming.h"

class ARQ : public Layer {
 public:
    ARQ(Framming & fr,long tout);
    ~ARQ();
    void init() {};

    // envia o quadro apontado por buffer
    // o tamanho do quadro é dado por bytes
    // coloca flag no inicio e fim e envia
    void send(char * buffer, int bytes);

    // não recebe notificações, então ...
    void notify(char * buffer, int len);

    // métodos de callback ... chamados pelo poller
    void handle();
    void handle_timeout();

 private:
  enum TipoEvento {Payload, Quadro, Timeout};
  enum States {Idle, WaitAck};
  States _state;
  int n_rx,n_tx;

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

#endif /* ARQ_H_ */
