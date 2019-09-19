#include <iostream>
#include "Serial.h"
#include "Framming.h"
#include "Poller.h"
#include "ARQ.h"

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
    sched.adiciona(&framming);
    sched.adiciona(&arq);

    sched.despache();

    // teste completo
//    string payload;;
//    while(1){
//    	while(1){
//    		framming.handle();
//    		if(framming.recebeu_completo == true) break;
//    	}
//    	cout <<"Enviar:";
//    	getline(cin,payload);
//    	arq.send((char*)payload.c_str(),payload.length());
//    }

    return 0;
}
