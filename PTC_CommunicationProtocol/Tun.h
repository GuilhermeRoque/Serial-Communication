/*
 * File:   Tun.h
 * Author: msobral
 *
 * Created on 9 de Março de 2016, 13:15
 */

#ifndef TUN_H
#define	TUN_H

#include <net/if.h>
#include "Definitions.h"

// TunFrame: usado internamente para representar quadros recebidos
// e enviados para a interface tun. Esses quadros têm um cabeçalho
// de 4 bytes (ver documentação)

struct TunFrame {
    char buffer[MTU+4];
    uint16_t len;

    TunFrame() : len(0) {}
    TunFrame(int fd);
    TunFrame(char * data, uint16_t len, uint16_t proto);
    uint16_t get_proto() const;
};

// Tun: representa uma interface tun
class Tun {
public:
    Tun();
    Tun(const Tun& orig);
    virtual ~Tun();

    // Cria um objeto tun, mas ainda não cria a interface
    // name: nome da interface a ser criada
    // ip: endereço ip desta interface
    // dstip: endereço IP da outra ponta do enlace
    Tun(const char * name, const char * ip, const char * dstip);

    // inicia a interface tun: cria a interface e a disponibiliza
    // no sistema operacional
    void start();

    // obtém o descritor de arquivo associado à interface
    int get();

    // recebe um quadro vindo da interface (vindo do subsistema de rede)
    // esse quadro tem um cabeçalho inserido pelo sistema operacional
    TunFrame * get_frame();

    // envia um quadro através da interface (para o subsistema de rede)
    void send_frame(TunFrame * f);

    // envia dados para o subsistema de rede
    int write(char * buffer, int len);
private:
    int fd;
    char dev[IFNAMSIZ], ip[16], dstip[16];

    void alloc();
    void setip();
};

#endif	/* TUN_H */
