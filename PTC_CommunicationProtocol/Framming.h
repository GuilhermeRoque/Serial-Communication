//
// Created by aluno on 15/08/2019.
//

#ifndef COMMUNICATIONPROTOCOL_FRAMMING_H
#define COMMUNICATIONPROTOCOL_FRAMMING_H

#include <cstdint>
#include <string.h>
#include <iostream>
#include "Serial.h"
#include "Layer.h"

#define FLAG 0x7E
#define ESCAPE 0x7D
#define XOR_FLAG 0x20;

class Framming : public Layer {
public:
    Framming(Serial & dev, int max_bytes, long tout);
    ~Framming();


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
    int _max_bytes; // tamanhos mínimo e máximo de quadro
    Serial & _port;
    char _buffer[4096]; // quadros no maximo de 4 kB (hardcoded)

    enum States {Idle, RX, ESC};

    // tipos de eventos associados à serial
    enum EventType {Octet, Timeout};

    // tipo Evento: representa um evento
    struct Event {
    	EventType type;
      uint8_t octet;

      // este construtor cria um Evento do tipo Timeout
      Event(): type(Timeout) {}

      // este construtor cria um Evento do tipo Dado
      Event(uint8_t b): type(Octet), octet(b) {}
    };

    // bytes recebidos pela MEF até o momento
    int _nbytes;

    // estado atual da MEF
    States _state;

    // aqui se implementa a máquina de estados de recepção
    // retorna true se reconheceu um quadro completo
    bool _handle_fsm(Event & ev);

};


#endif //COMMUNICATIONPROTOCOL_FRAMMING_H
