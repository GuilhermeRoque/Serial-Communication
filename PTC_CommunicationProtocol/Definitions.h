/*
 * File:   Definitions.h
 * Author: msobral
 *
 * Created on 15 de Agosto de 2018, 14:24
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#define MTU 256
#define MASK "255.255.255.252"
#define FrameTimeout 1000 // 750 ms
#define ByteTimeout 50 // 20 ms
#define BackoffSlot 50 // 20 ms
#define CWmax 15 // backoff máximo
#define MAX_TRANSMISSIONS 4
#define QueueLen 10 // comprimento da fila da tun (txqueue)
#define MAX_SERIAL_BYTES 64

#endif	/* DEFINITIONS_H */
