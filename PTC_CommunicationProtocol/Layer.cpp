/*
 * Layer.cpp
 *
 *  Created on: 21 de ago de 2019
 *      Author: vini
 */

#include "Layer.h"


Layer::Layer(int fd, long tout) : Callback(fd, tout), _upper(nullptr), _lower(nullptr) {

}

Layer::Layer(long tout) : Callback(tout), _upper(nullptr), _lower(nullptr) {

}


Layer::~Layer() {
	// TODO Auto-generated destructor stub
}

