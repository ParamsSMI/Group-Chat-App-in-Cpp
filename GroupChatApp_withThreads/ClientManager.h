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
#include <signal.h>
#include <mutex>
#define MAX_LEN 200
using namespace std;
class Client
{
    string m_Name;
    int m_ClientSocket;
    string m_ServerIP;
    int m_Port;
    mutex cout_lock;
    thread SenderThread;
    thread ReceiverThread;
    bool exit_flag{0};
public:
    Client(){};
    Client(string, int);
    void ConnectToServer();
    void PrintToMyTerminal(string);
    void InitSenderThread();
    void InitReceiverThread();
    void sendNameToServer();
    void JoinThreads();
};
