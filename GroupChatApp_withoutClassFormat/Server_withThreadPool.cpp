#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <pthread.h>
#include <mutex>
#include <queue>
#include <condition_variable>
#define MAX_LEN 200
#define NUM_COLORS 6
#define THREAD_POOL_SIZE 2
using namespace std;

thread ThreadPool[THREAD_POOL_SIZE];

struct terminal
{
    int id;
    string name;
    int socket;
};
queue<pair<int, int>> ClientQ;
vector<terminal> clients;

string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0;
mutex cout_mtx, clients_mtx, tt;
condition_variable cv;
string color(int code);
void set_name(int id, char name[]);
void shared_print(string str, bool endLine);
int broadcast_message(string message, int sender_id);
int broadcast_message(int num, int sender_id);
void end_connection(int id);
void handle_client(int client_socket, int id);

void ThreadFunction()
{
    shared_print("inside threadfunction", true);

    while (true)
    {

            int ClientSocket,id;
           
          {  
             unique_lock<mutex> lock(clients_mtx);

             cv.wait(lock,[](){return !ClientQ.empty();});

             if(ClientQ.empty()) break;

             ClientSocket = ClientQ.front().first;
             id = ClientQ.front().second;

            ClientQ.pop();
        
         }

           
            handle_client(ClientSocket, id);
            
        
    }
}
int main()
{

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
        ThreadPool[i] = thread(ThreadFunction);

    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&server.sin_zero, 0);

    if ((bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) == -1)
    {
        perror("bind error: ");
        exit(-1);
    }

    if ((listen(server_socket, 8)) == -1)
    {
        perror("listen error: ");
        exit(-1);
    }

    struct sockaddr_in client;
    int client_socket;
    unsigned int len = sizeof(sockaddr_in);

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl
         << def_col;
  
    while (1)
    {

       

        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("accept error: ");
            exit(-1);
        }
               
            if(seed > THREAD_POOL_SIZE)
            {
                 clients_mtx.lock();
                send(client_socket,"rejected",9,0);
                close(client_socket);
                seed--;
                clients_mtx.unlock();
                continue;
            }
            else
          {   clients_mtx.lock();
            send(client_socket,"yes",4,0);
            clients_mtx.unlock();
          }
            
  seed++;
        clients_mtx.lock();
        shared_print("client added " + to_string(seed), true);
        ClientQ.push({client_socket, seed});
        clients.push_back({seed, "Anonymous", client_socket});
        clients_mtx.unlock();
       
          cv.notify_one();
    }

    
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        if (ThreadPool[i].joinable())
            ThreadPool[i].join();
    }

    close(server_socket);
    return 0;
}

string color(int code)
{
    return colors[code % NUM_COLORS];
}

// Set name of client
void set_name(int id, char name[])
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            clients[i].name = string(name);
        }
    }
}

// For synchronisation of cout statements
void shared_print(string str, bool endLine = true)
{
    lock_guard<mutex> guard(cout_mtx);

    cout << str;
    if (endLine)
        cout << endl;
}

// Broadcast message to all clients except the sender
int broadcast_message(string message, int sender_id)
{
    shared_print("inside brodcast", true);
    char temp[MAX_LEN];
    strcpy(temp, message.c_str());
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id != sender_id)
        {
            send(clients[i].socket, temp, sizeof(temp), 0);
        }
    }
}

// Broadcast a number to all clients except the sender
int broadcast_message(int num, int sender_id)
{
    shared_print("inside brodcast", true);
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id != sender_id)
        {
            send(clients[i].socket, &num, sizeof(num), 0);
        }
    }
}

void end_connection(int id)
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            lock_guard<mutex> guard(clients_mtx);
            clients.erase(clients.begin() + i);
            close(clients[i].socket);
            seed--;
            break;
        }
    }
}

void handle_client(int client_socket, int id)
{
    char name[MAX_LEN], str[MAX_LEN];
    recv(client_socket, name, sizeof(name), 0);
    set_name(id, name);

    // Display welcome message
    string welcome_message = string(name) + string(" has joined");
    broadcast_message("#NULL", id);
    broadcast_message(id, id);
    broadcast_message(welcome_message, id);
    shared_print(color(id) + welcome_message + def_col);

    while (1)
    {
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0)
            return;
        if (strcmp(str, "#exit") == 0)
        {
            // Display leaving message
            string message = string(name) + string(" has left");
            broadcast_message("#NULL", id);
            broadcast_message(id, id);
            broadcast_message(message, id);
            shared_print(color(id) + message + def_col);
            end_connection(id);
            return;
        }
        broadcast_message(string(name), id);
        broadcast_message(id, id);
        broadcast_message(string(str), id);
        shared_print(color(id) + name + " : " + def_col + str);
    }
}
