#include "ClientManager.h"
using namespace std;

int main()
{

    Client *client1;
    client1 = new Client("127.0.0.1",8888);
    client1->ConnectToServer();
    client1->sendNameToServer();
    client1->InitSenderThread();
    client1->InitReceiverThread();
    client1->JoinThreads();
    return 0;
}