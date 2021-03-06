/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Callback.cpp
 * Author: msobral
 *
 * Created on 20 de Setembro de 2018, 13:41
 */

#include "Callback.h"
#include <iostream>

Callback::Callback(int fd, long tout): fd(fd), base_tout(tout),tout(tout), enabled_to(true), enabled(true), reloaded(false) {
    if (tout < 0) throw -1;
}

Callback::Callback(long tout) : fd(-1), base_tout(tout),tout(tout), enabled_to(true), reloaded(false) {
    if (tout < 0) throw -1;
}

int Callback::filedesc() const { return fd;}

void Callback::set_timeout(long t_out) {
    if (t_out < 0) throw -1;
    reloaded = true;
    tout = t_out;
}

int Callback::timeout() const { return tout;}

void Callback::reload_timeout() {
    tout = base_tout;
    reloaded = true;
}

void Callback::update(long dt) {
    if (! reloaded) {
        tout -= dt;
        if (tout < 0) tout = 0;
    } else reloaded = false;
}

bool Callback::operator==(const Callback & o) const {
  return (fd == o.fd) && (base_tout == o.base_tout);
}

void Callback::disable_timeout() {
    enabled_to = false;
}

void Callback::enable_timeout() {
    enabled_to = true;
}

void Callback::enable() {
    enabled = true;
}

void Callback::disable() {
    enabled = false;
}
