/*
 * CallbackTun.h
 *
 *  Created on: 21 de ago de 2019
 *      Author: vini
 */

#ifndef CALLBACKTUN_H_
#define CALLBACKTUN_H_

#include "Layer.h"
#include "Tun.h"

class CallbackTun : public Layer {
public:
    CallbackTun(Tun &tun, long tout);
    virtual ~CallbackTun();

    void init();
    void close();
    void send(char * buffer, int bytes);
    void notify(char * buffer, int len);
    void notifyERR();

    void handle();
    void handle_timeout();
private:
    Tun &_tun;
};

#endif /* CALLBACKTUN_H_ */
