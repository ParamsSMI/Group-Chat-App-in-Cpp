#include "ServerManager.h"

using namespace std;

int main()
{
    Server MyServer(8888);
    MyServer.InitServer();
    return 0;
}