/*
 * File:   Tun.cpp
 * Author: msobral
 *
 * Created on 9 de Março de 2016, 13:15
 */

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <asm/ioctl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

#include "Tun.h"
#include "Definitions.h"

TunFrame::TunFrame(int fd) {
    len = ::read(fd, buffer, MTU+4);
}

TunFrame::TunFrame(char * data, uint16_t len, uint16_t proto):len(len+4) {
    buffer[0] = 0;
    buffer[1] = 0;
    proto = htons(proto);
    buffer[2] = proto & 0xff;
    buffer[3] = proto >> 8;

    memcpy(buffer+4, data, len);
}

uint16_t TunFrame::get_proto() const {
    uint16_t * proto = (uint16_t*)(buffer+2);
    return ntohs(*proto);
}

Tun::Tun() {
    fd = 0;
}

Tun::Tun(const Tun& orig) {
    fd = orig.fd;
}

Tun::Tun(const char* name, const char* ip, const char* dstip) {
    if (name) strncpy(dev, name, IFNAMSIZ);
    else dev[0] = 0;
    strncpy(this->ip, ip, 16);
    strncpy(this->dstip, dstip, 16);
    fd = -1;
}

Tun::~Tun() {
    close(fd);
}

void Tun::start() {
    if (fd >= 0) return;
    alloc();
    setip();
}

int Tun::get() {
     if (fd >= 0) return fd;
     throw -3;
}

TunFrame * Tun::get_frame() {
    auto tf = new TunFrame(fd);

    return tf;
}

void Tun::send_frame(TunFrame* tf) {
    write(tf->buffer, tf->len);
}

int Tun::write(char* buffer, int len) {
    if (fd <= 0) return 0;
    int n = ::write(fd, buffer, len);
    return n;
}
// tun_alloc: cria uma interface tun
// Parâmetro de entrada:
//  char * dev: o nome da interface ser criada. Se for NULL, interface será denominada pelo sistema (ex: tun0)
//
// Retorno: valor inteiro
//  - se > 0: o descritor de arquivo da interface tun criada.
//  - se < 0, ocorreu um erro
void Tun::alloc() {

      struct ifreq ifr;
      int err;

      if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
         throw -1;
      }

      memset(&ifr, 0, sizeof(ifr));

      /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
       *        IFF_TAP   - TAP device
       *
       *        IFF_NO_PI - Do not provide packet information
       */
      //ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
      ifr.ifr_flags = IFF_TUN;
      if( dev[0] )
         strncpy(ifr.ifr_name, dev, IFNAMSIZ);

      err = ioctl(fd, TUNSETIFF, (void *) &ifr);
      if( err < 0 ){
         close(fd);
         throw -1;
      }
      if (not dev[0]) strncpy(dev, ifr.ifr_name, IFNAMSIZ);
  }

// esta outra função configura a interface tun: define os endereços IP do enlace ponto-a-ponto,
// a máscara de rede /30 e ativa a interface. Ela faz o mesmo que o comando:
// ifconfig nome_tun IP_da_interface dstaddr IP_da_outra_ponta_do_enlace
//
// esta função deve ser usada assim, supondo que a interface tun se chame tun0, e os endereços IP
// do enlace sejam 10.0.0.1 e 10.0.0.2:
//
// if (set_ip("tun0", "10.0.0.1", "10.0.0.2") < 0) {
//   perror("so configurar a interface tun");
//   return 0;
// }
//
// Maiores detalhes sobre as chamadas de sistemas utilizadas: ver "man netdevice", ou
// http://man7.org/linux/man-pages/man7/netdevice.7.html

void Tun::setip() {
  struct ifreq ifr;
  struct sockaddr_in *addr;
  int ok;
  int sd;

  // cria um socket para configurar a interface
  // esse socket não tem nada de especial ...
  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0) throw -2;

  // zera todos os bytes da struct ifreq
  // essa struct contém os atributos a serem configurados na interface
  bzero(&ifr, sizeof(ifr));

  // usa o ponteiro addr para referenciar o campo de endereço da struct ifreq
  // isso facilita o preenchimento dos atributos desse campo
  addr = (struct sockaddr_in*)&(ifr.ifr_ifru.ifru_addr);

  // preenche o campo endereço com o enderço IP da interface
  addr->sin_addr.s_addr = inet_addr(ip);
  addr->sin_family = AF_INET;
  addr->sin_port = 0;

  // escreve o nome da interface na struct ifreq. Isso é necessário
  // para o kernel saber que interface é alvo da operação
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  // executa uma operação de configuração de endereço IP de interface
  ok = ioctl(sd, SIOCSIFADDR, &ifr);
  if (ok < 0) throw -2;

  // preenche o campo endereço com o endereço IP da outra ponta do enlace
  addr->sin_addr.s_addr = inet_addr(dstip);

  // executa uma operação de configuração de endereço IP de destino da interface
  ok = ioctl(sd, SIOCSIFDSTADDR, &ifr);
  if (ok < 0) throw -2;

  // preenche o campo endereço com a máscara de rede da interface
  addr->sin_addr.s_addr = inet_addr(MASK);


  // executa uma operação de configuração de máscara de rede da interface
  ok = ioctl(sd, SIOCSIFNETMASK, &ifr);
  if (ok < 0) throw -2;

  // executa uma operação de configuração de MTU da interface
  ifr.ifr_mtu = MTU;
  ok = ioctl(sd, SIOCSIFMTU, &ifr);
  if (ok < 0) throw -2;

  // modifica o tamanho da fila de saída
  ifr.ifr_qlen = QueueLen;
  ok = ioctl(sd, SIOCSIFTXQLEN, &ifr);
  if (ok < 0) throw -2;

  // lê as flags da interface
  ok = ioctl(sd, SIOCGIFFLAGS, &ifr);
  if (ok < 0) throw -2;

  // acrescenta flags UP (ativa) e RUNNING (pronta para uso)
  ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

  // executa uma operação de configuração flags da interface
  if (ioctl(sd, SIOCSIFFLAGS, &ifr) < 0) throw -2;
}
