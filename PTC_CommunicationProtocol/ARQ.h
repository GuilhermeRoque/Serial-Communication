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
#include "utils.h"

#define TIMEOUT_BACKOFF 1 // millisegundos

class ARQ : public Layer {
 public:
    ARQ(long tout);
    ~ARQ();
    void init() {};
    void close() {};

    void send(char * buffer, int bytes);

    void notify(char * buffer, int len);
    void notifyERR();

    // métodos de callback ... chamados pelo poller
    void handle();
    void handle_timeout();

 private:
  enum TipoEvento {Payload, Quadro, Timeout};
  enum States {Idle, WaitAck, BackoffAck, BackoffRelay};
  States _state;
  bool M,N;
  char buffer_tx[1026];
  int bytes_tx;
  int retry_counter;

  bool is_ACK(uint8_t byte);
  bool is_DATA(uint8_t byte);
  bool check_SEQ(uint8_t byte,bool bit);
  void set_backoff();

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
