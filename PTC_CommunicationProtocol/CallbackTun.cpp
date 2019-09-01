/*
 * CallbackTun.cpp
 *
 *  Created on: 21 de ago de 2019
 *      Author: vini
 */

#include "CallbackTun.h"

//CallbackTun::CallbackTun(Tun &tun, long tout) : _tun(tun), Layer(tun.get(), tout) {
//
//}

void CallbackTun::init() {
	_tun.start();
}

void CallbackTun::send(char * buffer, int bytes) {

}

void CallbackTun::notify(char * buffer, int len) {

}

void CallbackTun::handle() {

}

void CallbackTun::handle_timeout() {

}
