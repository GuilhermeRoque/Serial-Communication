/*
 * App.h
 *
 *  Created on: 21 de set de 2019
 *      Author: guilherme
 */

#ifndef APP_H_
#define APP_H_

#include <cstdint>
#include <string.h>
#include <iostream>
#include "Layer.h"
#include "utils.h"
#include <fcntl.h>
#include <unistd.h>

class App : public Layer{
public:
	App(int tty_fd, long tout);
	~App();

    void send(char * buffer, int bytes);

    // não recebe notificações, então ...
    void notify(char * buffer, int len);
    void notifyERR() {};

    // métodos de callback ... chamados pelo poller
    void handle();
    void handle_timeout();
    void init() {};

private:
    int tty_fd;
    char buffer[1024];

};

#endif /* APP_H_ */
