#include <iostream>
#include "Serial.h"
#include "Framming.h"
#include "Poller.h"
#include "ARQ.h"

using namespace std;

int main() {
    char buf[1024];
    Serial rf("/dev/pts/10", B9600);
    Framming framming(rf, 1024, 1000);
    ARQ arq(framming,1000);
    Poller sched;
    sched.adiciona(&framming);
    sched.adiciona(&arq);

    framming.init();

    sched.despache();

    return 0;
}
