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
	Layer();
	virtual ~Layer();

	void send();
	void notify();

private:
	Layer *_upper;
	Layer *_lower;

};


#endif /* LAYER_H_ */
