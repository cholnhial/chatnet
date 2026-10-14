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

#ifndef NETWORK_H_
#define NETWORK_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <errno.h>
#include <gtkmm.h>
#include "global.h"

typedef sigc::signal<void> signal_output_type;
typedef sigc::signal<void> signal_network_error_type;
typedef sigc::signal<void> signal_new_user_type;
typedef sigc::signal<void> signal_remove_user_type;

const short SERVER  = 1;
const short CLIENT =  2;
using std::cerr;
using std::string;

class network
{
    int sockfd,newfd,rv;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage clientaddr;
    socklen_t size;
    signal_output_type event_signal_output;
    signal_new_user_type event_new_user;
    signal_remove_user_type event_remove_user;
    signal_network_error_type event_network_error;
    string output_buffer;
    enum {MAX_ARGS = 3};
    string args[MAX_ARGS];
    string colour;
    int first_output;
    char buffer[MAX_BUFFER + 1];
    char receive_buffer[MAX_BUFFER];
    size_t received;
    sigc::connection socket_watch;
    bool on_socket_ready(Glib::IOCondition condition);
    int output_counter;
    int socket_buffer_counter,terminal_input_counter;
    int temp_int;
    bool inchat;
    bool no_signal;
    bool check_ischat;
    std::string person_inchat;
    std::string name;
    fd_set readfds;
public:
    network();
    ~network();
    int is_first_output()
    {
        return first_output;
    }
    int start(const char* host, const char* port, const short  type);
    int get_sockfd();
    int send_msg(const string& username, const char* msg);
    int check_name(const string& n);
    string & get_name();
    int input_to_server(const string& str);
    int check_inchat(const string& user);
    bool is_chat();
    void mainloop();
    const string& get_text_colour();
    void close_socket();
    void reset();
    signal_output_type on_signal_output() const
    {
        return event_signal_output;
    }
    signal_network_error_type on_signal_network_error() const
    {
        return event_network_error;
    }
    signal_new_user_type on_signal_new_user() const
    {
        return event_new_user;
    }
    signal_remove_user_type on_signal_remove_user() const
    {
        return event_remove_user;
    }
    void out(const string& text);
    void out(char c);
    const string& get_buffer()
    {
        return output_buffer;
    }
};
#endif

