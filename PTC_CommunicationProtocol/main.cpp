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
	if(argc != 3 && argc != 5){
		printf("Utilize a seguinte sintaxe para execucao:\n");
		printf("./PTC_CommunicationProtocol <FD_serial> <id_sessao> <IP_origem> <IP_destino>\n");
		printf("Ex.: ./PTC_CommunicationProtocol /dev/pts/3 55 10.10.10.1 10.10.10.2\n\n");

		printf("Obs.: caso nao seja informado o IP de origem e destino, sera inicializada a fake layer no lugar da TUN.\n");
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

    framming.set_upper(&arq);
    arq.set_lower(&framming);
    arq.set_upper(&sessao);
    sessao.set_lower(&arq);

    Poller sched;
    sched.adiciona(&framming);
    sched.adiciona(&arq);
    sched.adiciona(&sessao);

    if (argc == 3) {
    	App app(0,5000);
    	sessao.set_upper(&app);
    	app.set_lower(&sessao);
    	sched.adiciona(&app);
    	app.init();
    	sched.despache();
    } else {
		Tun tun("ptc_iface", argv[3], argv[4]);
		tun.start();
		CallbackTun ctun(tun, 5000);
		sessao.set_upper(&ctun);
		ctun.set_lower(&sessao);
		sched.adiciona(&ctun);
//		ctun.init();
		sched.despache();
    }


    return 0;
}
