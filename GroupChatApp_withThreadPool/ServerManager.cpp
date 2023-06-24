#include "ServerManager.h"
using namespace std;

Server::~Server()
{
    cout << "inside destructor\n";
    JoinTheThreads();
    CloseServer();
}

Server::Server(int port) : m_Port(port){};

void Server::PrintToMyTerminal(string str)
{
    lock_guard<mutex> guard(cout_lock);
    cout << str;
}

ClientNode::ClientNode(int id, string name, int socket) : m_ClientID(id), m_Name(name), m_Socket(socket){};

void Server::InitServer()
{

    if ((m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(m_Port);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);

    if ((bind(m_ServerSocket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == -1)
    {
        perror("bind error: ");
        exit(-1);
    }

    if ((listen(m_ServerSocket, 8)) == -1)
    {
        perror("listen error: ");
        exit(-1);
    }

    PrintToMyTerminal("server strated successfully\n");

    InitThreadPool();

    ListenToClients();
}

void Server::InitThreadPool()
{
    PrintToMyTerminal("init thread pool\n");

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        ThreadPool[i] = thread([=]
                               { this->handleClients(); });
    }
}

void Server::ListenToClients()
{

    PrintToMyTerminal("listening for client request ...\n");
    struct sockaddr_in client_address;
    int client_socket;
    unsigned int len = sizeof(client_address);

    int index = 0;

    while (true)
    {
        if ((client_socket = accept(m_ServerSocket, (struct sockaddr *)&client_address, &len)) == -1)
        {
            perror("accept error: ");
            continue;
        }

        mutex_lock.lock();

        cout << client_socket << endl;
        ClientNode *client = new ClientNode(index, "anonymous", client_socket);
        index++;
        ClientQ.push(client);
        clients.push_back(client);

        mutex_lock.unlock();

        cv.notify_one();
    }
}

void Server::JoinTheThreads()
{
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (ThreadPool[i].joinable())
            ThreadPool[i].join();
    }
}

void Server::CloseServer()
{
    close(m_ServerSocket);
    PrintToMyTerminal("server closed\n");
}

void Server::SetNameOfClient(string name, int id)
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i]->m_ClientID == id)
        {
            clients[i]->m_Name = name;
            break;
        }
    }
}

void Server::BroadCastMessage(string message, int SenderID)
{

    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i]->m_ClientID != SenderID)
        {
            send(clients[i]->m_Socket, message.c_str(), sizeof(message), 0);
        }
    }
}

void Server::SendMessageToClient(string sender_name, string message, int SenderID)
{
    string recv_name;
    string msg;
    for (int i = 1; i < message.length(); i++)
    {
        if (message[i] != ' ')
            recv_name += message[i];
        else
        {
            msg = message.substr(i, message.length() - i + 1);
            break;
        }
    }
    msg = sender_name + " : " + msg;
    // cout << recv_name << " " << msg << endl;

    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i]->m_Name == recv_name)
        {
            send(clients[i]->m_Socket, msg.c_str(), sizeof(msg), 0);
            break;
        }
    }
}

void Server::WelcomeMessageToClient(string name, int id)
{
    string welcome_message = string(name) + string(" has joined\n");
    BroadCastMessage(welcome_message, id);
    PrintToMyTerminal(welcome_message);
}

void Server::EndConnectionOfClient(ClientNode *client)
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i] == client)
        {
            lock_guard<mutex> guard(mutex_lock);
            clients.erase(clients.begin() + i);
            close(clients[i]->m_Socket);
            break;
        }
    }
}
bool Server::IsMessageForPersonalChat(string message)
{
    if (message[0] == '@')
        return 1;
    return 0;
}
void Server::handleClients()
{

    while (true)
    {
        ClientNode *Client;
        {
            unique_lock<mutex> lock(mutex_lock);

            this->cv.wait(lock, [this]
                          { return !ClientQ.empty(); });

            if (ClientQ.empty())
                break;

            Client = ClientQ.front();
            ClientQ.pop();
        }
        fflush(stdout);

        char name[MAX_LEN], message[MAX_LEN];

        recv(Client->m_Socket, name, sizeof(name), 0);

        string Name(name);
        SetNameOfClient(Name, Client->m_ClientID);

        WelcomeMessageToClient(Name, Client->m_ClientID);

        while (true)
        {
            fflush(stdout);

            int bytes_received = recv(Client->m_Socket, message, sizeof(message), 0);
            if (bytes_received <= 0)
                return;

            if (strcmp(message, "exit") == 0)
            {

                string leftClient = Name + "has left";
                BroadCastMessage(leftClient, Client->m_ClientID);
                EndConnectionOfClient(Client);
                cout << leftClient << endl;
                break;
            }

            if (IsMessageForPersonalChat(message))
            {
                string MessageWithName = Name + " : " + message;
                SendMessageToClient(Name, message, Client->m_ClientID);
            }
            else
            {
                string MessageWithName = Name + " : " + message;
                PrintToMyTerminal(MessageWithName + "\n");
                BroadCastMessage(MessageWithName, Client->m_ClientID);
            }
        }
    }
    return;
}
