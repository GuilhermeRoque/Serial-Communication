//
// Created by aluno on 15/08/2019.
//

#include "Framming.h"


Framming::Framming(Serial &dev, int min_bytes, int max_bytes) : _port(dev){
//    _port = dev;
    _min_bytes = min_bytes;
    _max_bytes = max_bytes;
    _nbytes = 0;
    memset(_buffer, 0, sizeof(_buffer));
    _state = Ocioso;
}

Framming::~Framming() {
}

void Framming::send(char *buffer, int bytes) {
	_port.write(buffer, bytes);
}

int Framming::receive(char *buffer) {
    if (_state != Ocioso)
        throw -2;

    while (true) {
        uint8_t a_byte;

        if (_port.read((char*) &a_byte, 1, true) != 1 ) {
            throw -3;
        }

        if (_handle(a_byte)) {
            memcpy(buffer, _buffer, _nbytes);
            return _nbytes;
        }

    }

}

bool Framming::_handle(char byte) {
    bool complete_frame = false;

    switch (_state) {
        case Ocioso: // estado Ocioso
            memset(_buffer, 0, sizeof(_buffer));
            _nbytes = 0;

            if (byte == 0x7E)
                _state = RX;

            break;
        case RX: // estado RX
            if (byte == 0x7D) {
                _state = ESC;
                break;
            }

            if ((byte == 0x7E) && (_nbytes > _min_bytes)) {
                // End a frame
                complete_frame = true;
                _state = Ocioso;
                break;
            }

            if (_nbytes > _max_bytes) {
                _state = Ocioso;
                break;
            }

            _buffer[_nbytes] = byte;
            _nbytes++;
            break;
        case ESC: // estado ESC
            if ((byte == 0x7E) || (byte == 0x7D)) {
                _state = Ocioso;
                break;
            }

            byte = byte xor 0x20;
            _buffer[_nbytes] = byte;
            _nbytes++;
            _state = RX;
            break;
    }

    return complete_frame;
}
