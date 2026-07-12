/* Copyright (C) 2012 Chol Nhial <chol.nhial.chol@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctime>
#include <sys/select.h>
#include <signal.h>
#include <cctype>

#ifndef CHATNET_H_
#define CHATNET_H_
using std::string;
using std::cerr;


void *LaunchMemberFunction(void *obj);

/* Represents the connected clients */
struct client
{
    string name;
    int sockfd;
};


/* This class represents the chatnet server which manages
the clients*/
class chatnet
{
private:
    int sfd, newfd,rv;
    struct addrinfo hints, * servinfo, *p;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
//very important vector that holds the list of all connected clients.
    std::vector<client> client_list;
    pthread_t client_thread;
    pthread_mutex_t mutex;
    struct timeval tv;
    fd_set rfds;
    enum {MAX_ARGS = 3, MAX_NAME_SIZE = 16};
    string args[MAX_ARGS];
public:
    ~chatnet();
    int server(const char * lport);
    void * serve_client(void * client_sfd);
    int get_client_name(unsigned int & c, std::string & n,unsigned int sock);
    void remove_client(int position);
    int find_client(string& c_name, int socket);
    bool is_illegal_name(const string& c_name);
};

// This thread launcher class launches serveClient thread
class ThreadLauncher
{
public:
    ThreadLauncher(chatnet *Obj, void *arg) : Obj(Obj), Arg(arg) {}

    void *launch()
    {
        return Obj->serve_client(Arg);
    }

    chatnet *Obj;
    void *Arg;
};

#endif
