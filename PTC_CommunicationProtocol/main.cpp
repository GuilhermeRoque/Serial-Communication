#include <iostream>
#include "Serial.h"
#include "Framming.h"

using namespace std;

int main() {
    char buf[2048];
    Serial rf("/dev/pts/3", B9600);
    Framming framming(rf, 1, 2048);

    while (true) {
        memset(buf, 0, sizeof(buf));

    	int rec_bytes = framming.receive(buf);
    	cout << "Recebeu: " << buf << endl;
    	buf[rec_bytes] = '\n';
    	framming.send(buf, rec_bytes+1);
    	cout << "Enviado.\n";
    }


    return 0;
}
