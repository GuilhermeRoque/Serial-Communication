#include <iostream>
#include "Serial.h"
#include "Framming.h"

using namespace std;

int main() {
    char buf[2048];
    Serial rf("/dev/pts/5", B9600);
    Framming framming(rf, 1, 2048);
    framming.receive(buf);

    cout << "Recebeu: " << buf << endl;

    return 0;
}
