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
    ARQ arq(1000);
    arq.set_lower(&framming);
    framming.set_upper(&arq);
    Poller sched;
    sched.adiciona(&framming);
    sched.adiciona(&arq);

    framming.init();

    sched.despache();

    return 0;
}
