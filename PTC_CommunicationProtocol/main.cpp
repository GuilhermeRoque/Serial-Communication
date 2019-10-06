#include <iostream>
#include "Serial.h"
#include "Framming.h"
#include "Poller.h"
#include "ARQ.h"
#include "App.h"
#include "Session.h"

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
    Session sessao(1000);
    App app(0,2000);

    framming.set_upper(&arq);
    arq.set_lower(&framming);
    arq.set_upper(&sessao);
    sessao.set_lower(&arq);
    sessao.set_upper(&app);
    app.set_lower(&sessao);

    Poller sched;
    sched.adiciona(&framming);
    sched.adiciona(&arq);
    sched.adiciona(&app);
    sched.adiciona(&sessao);

    sched.despache();

    return 0;
}
