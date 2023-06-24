#include "ClientManager.h"
using namespace std;

Client::Client(string serverIP, int port) : m_Port(port), m_ServerIP(serverIP){};
Client::~Client(){
    cout << "in client destructor\n";
    close(m_ClientSocket);
}

void Client::PrintToMyTerminal(string str)
{
    lock_guard<mutex> guard(cout_lock);
    cout << str;
}
void Client::ConnectToServer()
{
    PrintToMyTerminal("connecting to server\n");

    if ((m_ClientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        return;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(m_Port);
    server_address.sin_addr.s_addr = INADDR_ANY; // inet_addr(m_ServerIP.c_str());

    bzero(&server_address.sin_zero, 0);

    if ((connect(m_ClientSocket, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in))) == -1)
    {
        perror("connect: ");
        return;
    }
    PrintToMyTerminal("connected to server\n");

    sendNameToServer();
    InitSenderThread();
    InitReceiverThread();
    JoinThreads();
}

void Client::InitReceiverThread()
{
    ReceiverThread = thread([this]()
                            {
        while(true){
                
            if(exit_flag == 1) return;

            char message[MAX_LEN];
            
		    

            int bytes_received =recv(m_ClientSocket, message, sizeof(message), 0);
            if (bytes_received <= 0)
			    continue; 
                
            string recvmsg(message);

            char back =8;
            for(int i=0;i<6;i++)cout <<back;
            
            PrintToMyTerminal(recvmsg + "\n");
            PrintToMyTerminal("You : ");
            fflush(stdout);
        } });
}

void Client::InitSenderThread()
{
    SenderThread = thread([this]()
                          {
        while (true)
	    {
        PrintToMyTerminal("you : ");
	
        string msg;
		getline(cin,msg);
        string tt = msg;
		

		if (msg == "exit")
		{
            exit_flag = 1;
            send(m_ClientSocket, msg.c_str(), sizeof(msg), 0);
            ReceiverThread.detach();
			close(m_ClientSocket);
			return;
		}

        send(m_ClientSocket, msg.c_str(), sizeof(msg), 0);

	    } });
}
void Client::sendNameToServer()
{
  
  //  cout << "in setname\n";
    string Name;
    PrintToMyTerminal("Enter your name : ");
    getline(cin, Name);
    m_Name = Name;
  //   cout << Name << endl;
    send(m_ClientSocket, m_Name.c_str(), sizeof(Name), 0);
     
}
void Client::JoinThreads()
{
    if (SenderThread.joinable())
        SenderThread.join();
    if (ReceiverThread.joinable())
        ReceiverThread.join();
}