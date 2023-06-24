#include "ClientManager.h"
using namespace std;

int main()
{
    Client *client1;
    client1 = new Client("127.0.0.1", 8888);
    client1->ConnectToServer();
    return 0;
}