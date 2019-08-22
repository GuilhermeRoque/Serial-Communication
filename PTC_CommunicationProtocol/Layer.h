/*
 * Layer.h
 *
 *  Created on: 21 de ago de 2019
 *      Author: vini
 */

#ifndef LAYER_H_
#define LAYER_H_

#include "Callback.h"

class Layer : public Callback {
public:
    Layer(int fd, long tout);
    Layer(long tout);
    virtual ~Layer();

    void set_upper(Layer *upper) { _upper = upper; }
    void set_lower(Layer *lower) { _lower = lower; }

    virtual void init() = 0;

    // envia dados vindos da camada superior
    virtual void send(char *ptr, int len) = 0;

    // notificação vinda da camada inferior sobre a chegada de
    virtual void notify(char *ptr, int len) = 0;

private:
    Layer *_upper, *_lower;

};


#endif /* LAYER_H_ */
