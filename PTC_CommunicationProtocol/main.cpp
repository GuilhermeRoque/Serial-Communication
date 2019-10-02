#include <iostream>
#include "Serial.h"
#include "Framming.h"
#include "Poller.h"
#include "ARQ.h"
#include "App.h"

using namespace std;

int main(int argc, char ** argv) {
	if(argc < 2){
		cout <<"Especifique FD\n"<<endl;
		return -1;
	}
	char * path = argv[1];
	Serial rf(path, B9600);
    Framming framming(rf, 1026, 1000); //agr tem mais 2 bytes de ctrl então é 1026
    ARQ arq(1000);
    arq.set_lower(&framming);
    framming.set_upper(&arq);
    Poller sched;
    App app(0,2000);
    app.set_lower(&arq);
    arq.set_upper(&app);

    sched.adiciona(&framming);
    sched.adiciona(&arq);
    sched.adiciona(&app);

    sched.despache();

    return 0;
}
