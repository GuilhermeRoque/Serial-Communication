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
#include "utils.h"

#define FLAG 0x7E
#define ESCAPE 0x7D
#define XOR_FLAG 0x20
#define PPPINITFCS16 0xffff  /* Initial FCS value */
#define PPPGOODFCS16 0xf0b8  /* Good final FCS value */

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
    void notifyERR() {};

    // métodos de callback ... chamados pelo poller
    void handle();
    void handle_timeout();
    int recebeu_completo; //apenas para debug
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

    /* Implementacoes CRC */
    static uint16_t _fcstab[256];

    // verifica o CRC do conteúdo contido em "buffer". Os dois últimos
    // bytes desse buffer contém o valor de CRC
    bool _check_crc(char * buffer, int len);

    // gera o valor de CRC dos bytes contidos em buffer. O valor de CRC
    // é escrito em buffer[len] e buffer[len+1]
    void _gen_crc(char * buffer, int len);

    // calcula o valor de CRC dos bytes contidos em "cp".
    // "fcs" deve ter o valor PPPINITFCS16
    // O resultado é o valor de CRC (16 bits)
    // OBS: adaptado da RFC 1662 (enquadramento no PPP)
    uint16_t _pppfcs16(uint16_t fcs, char * cp, int len);

};


#endif //COMMUNICATIONPROTOCOL_FRAMMING_H
