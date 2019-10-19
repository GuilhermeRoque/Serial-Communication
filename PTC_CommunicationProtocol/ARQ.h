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
#include <queue>

#define TIMEOUT_BACKOFF 1 // millisegundos
#define TIMEOUT_ACK 1000 // millisegundos

class ARQ : public Layer {
 public:
    ARQ(long tout);
    ARQ(long tout, uint8_t id_sessao);
    ~ARQ();
    void init();
    void close() {};

    void send(char * buffer, int bytes);

    void notify(char * buffer, int len);
    void notifyERR();

    // métodos de callback ... chamados pelo poller
    void handle();
    void handle_timeout();

    void disable();

 private:
  enum TipoEvento {Payload, Quadro, Timeout};
  enum States {Idle, WaitAck, BackoffAck, BackoffRelay};
  States _state;
  bool M,N;
  int retry_counter;
  char id_sessao;

  bool is_ACK(uint8_t byte);
  bool is_DATA(uint8_t byte);
  bool check_SEQ(uint8_t byte,bool bit);
  void set_backoff();

  // esta struct descreve um Evento
  struct Evento {
    TipoEvento tipo;
    char ptr[1026];
    int bytes;

    // construtor sem parâmetros: cria um Evento Timeout
    Evento() { tipo = Timeout;}

    // construtor com parâmetros: cria um evento Payload ou Quadro
    Evento(TipoEvento t, char * p, int len) : tipo(t), bytes(len) {
    	memcpy(ptr,p,bytes);
    }
  };

  // executa a MEF, passando como parâmetro um evento
  void handle_fsm(Evento & e);

  std::queue<Evento> buffer_ev;
  void send_payload(Evento ev);

  char buffer_tx[1026];
  int bytes_tx;

};

#endif /* ARQ_H_ */
