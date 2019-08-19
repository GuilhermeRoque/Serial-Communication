//
// Created by aluno on 15/08/2019.
//

#ifndef COMMUNICATIONPROTOCOL_FRAMMING_H
#define COMMUNICATIONPROTOCOL_FRAMMING_H

#include <cstdint>
#include <string.h>
#include <iostream>
#include "Serial.h"

class Framming {
public:
    Framming(Serial & dev, int min_bytes, int max_bytes);
    ~Framming();

    // envia o quadro apontado por buffer
    // o tamanho do quadro é dado por bytes
    void send(char * buffer, int bytes);

    // espera e recebe um quadro, armazenando-o em buffer
    // retorna o tamanho do quadro recebido
    int receive(char * buffer);

private:
    int _min_bytes, _max_bytes; // tamanhos mínimo e máximo de quadro
    Serial & _port;
    char _buffer[4096]; // quadros no maximo de 4 kB (hardcoded)

    enum States {Ocioso, RX, ESC};

    // bytes recebidos pela MEF até o momento
    int _nbytes;

    // estado atual da MEF
    States _state;

    // aqui se implementa a máquina de estados de recepção
    // retorna true se reconheceu um quadro completo
    bool _handle(char byte);

};


#endif //COMMUNICATIONPROTOCOL_FRAMMING_H
