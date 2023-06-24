#pragma once
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>
#define THREAD_POOL_SIZE 5
#define MAX_LEN 500
using namespace std;

class ClientNode
{
public:
    int m_ClientID;
    string m_Name;
    int m_Socket;
    ClientNode();
    ClientNode(int, string, int);
};

class Server
{

private:
    int m_Port;
    int m_ServerSocket;
    vector<ClientNode *> clients;
    mutex mutex_lock;
    mutex cout_lock;
    condition_variable cv;
    void WelcomeMessageToClient(string, int);
    void EndConnectionOfClient(int);
    bool IsMessageForPersonalChat(string);
    void SetNameOfClient(string, int);
    void CloseServer();
    void ListenToClients();
public:
    Server(){};
    Server(int);
    ~Server();
    void InitServer();
    void handleClients(int,int);
    void BroadCastMessage(string, int);
    void SendMessageToClient(string,string, int);
    void PrintToMyTerminal(string);
};