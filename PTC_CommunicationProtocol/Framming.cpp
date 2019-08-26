//
// Created by aluno on 15/08/2019.
//

#include "Framming.h"


Framming::Framming(Serial &dev, int max_bytes, long tout) : _port(dev), Layer(dev.get(), tout) {
    _max_bytes = max_bytes;
    _nbytes = 0;
    memset(_buffer, 0, sizeof(_buffer));
    _state = Idle;
}

Framming::~Framming() {
}

void Framming::send(char *buffer, int bytes) {
    _port.write(buffer, bytes);
}

//int Framming::receive(char *buffer) {
//    if (_state != Idle)
//        throw -2;
//
//    while (true) {
//        uint8_t a_byte;
//
//        if (_port.read((char*) &a_byte, 1, true) != 1 ) {
//            throw -3;
//        }
//
//        if (_handle(a_byte)) {
//            memcpy(buffer, _buffer, _nbytes);
//            return _nbytes;
//        }
//
//    }
//
//}

void Framming::notify(char * buffer, int len) {
    // for now, just print
    std::cout << "notify\n";
}

void Framming::handle() {
    uint8_t b;

    _port.read((char*) &b, 1, true);

    Event ev(b);
    if (_handle_fsm(ev)) {
        // A complete frame was received
        std::cout << _buffer << std::endl;
    }
}

void Framming::handle_timeout() {

}

bool Framming::_handle_fsm(Event & ev) {
    bool complete_frame = false;
    char byte = ev.octet;

    switch (_state) {
        case Idle: // estado Ocioso
            memset(_buffer, 0, sizeof(_buffer));
            _nbytes = 0;

            if (byte == FLAG)
                _state = RX;

            break;

        case RX: // estado RX
            if (byte == ESCAPE) {
                _state = ESC;
                break;
            }

            if ((byte == FLAG) && (_nbytes > 0)) {
                // End a frame
                complete_frame = true;
                _state = Idle;
                break;
            }

            if (_nbytes > _max_bytes) {
                _state = Idle;
                break;
            }

            _buffer[_nbytes] = byte;
            _nbytes++;
            break;

        case ESC: // estado ESC
            if ((byte == FLAG) || (byte == ESCAPE)) {
                _state = Idle;
                break;
            }

            byte = byte xor XOR_FLAG;
            _buffer[_nbytes] = byte;
            _nbytes++;
            _state = RX;
            break;
    }

    return complete_frame;
}
