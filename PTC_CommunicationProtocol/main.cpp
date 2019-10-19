#include <iostream>
#include "Serial.h"
#include "Framming.h"
#include "Poller.h"
#include "ARQ.h"
#include "App.h"
#include "Session.h"
#include "Tun.h"
#include "CallbackTun.h"

using namespace std;

int main(int argc, char ** argv) {
	uint8_t id_sessao;
	if(argc < 3){
		printf("Utilize a seguinte sintaxe para execucao:\n");
		printf("./PTC_CommunicationProtocol <FD_serial> <id_sessao>\n");
		printf("Ex.: ./PTC_CommunicationProtocol /dev/pts/3 55\n");
		return -1;
	}

	try {
		id_sessao = (uint8_t) atoi(argv[2]);
	} catch (int e) {
		printf("%% Informe um numero entre 0 e 255 para o ID da sessao.\n");
		return -1;
	}

	char * path = argv[1];

	Serial rf(path, B9600);
    Framming framming(rf, 1026, 1000); //agr tem mais 2 bytes de ctrl então é 1026
    ARQ arq(TIMEOUT_ACK, id_sessao);
    Session sessao(-1, TIMEOUT_SESSION, id_sessao);
    App app(0,5000);
//    Tun tun("ptc_iface", "10.10.10.2", "10.10.10.1");
//    tun.start();
//    CallbackTun ctun(tun, 1000);

    framming.set_upper(&arq);
    arq.set_lower(&framming);
    arq.set_upper(&sessao);
    sessao.set_lower(&arq);
    sessao.set_upper(&app);
    app.set_lower(&sessao);
//    sessao.set_upper(&ctun);
//    ctun.set_lower(&sessao);
//    ctun.init();

    Poller sched;
    sched.adiciona(&framming);
    sched.adiciona(&arq);
    sched.adiciona(&sessao);
    sched.adiciona(&app);
//    sched.adiciona(&ctun);
    sched.despache();
    
    return 0;
}
