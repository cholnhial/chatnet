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

#include "network.h"
#include "chatnetgui.h"
/* Intiliazes structures to be used by getaddrinfo(), binds  to a local port and
 * puts the open socket fd in variable sockfd for latter use. or
 * if type is for client connect to a remote port.
 */
int network::start(const char * host, const char * port, const short  type)
{
    close_socket();
    int yes = 1;
    int rv;


    switch (type)
    {
    case SERVER:
    {
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;


        if ((rv = getaddrinfo(NULL, port, &hints,&servinfo)) != 0)
        {
            std::cerr << gai_strerror(rv) << "\n";
            return -1;
        }

        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
            if (sockfd == -1)
            {
                perror("socket");
                continue;
            }
            if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, &yes,p->ai_addrlen) == -1)
            {
                std::cerr << "ERORR: setsockopt()\n";
                return -1;
            }
            if (bind(sockfd,p->ai_addr,p->ai_addrlen) == -1)
            {
                perror("bind");
                close(sockfd);
                sockfd = -1;
                continue;
            }

            break;

        }
        if (p != NULL)
        {
            int rv;
            if ((rv = listen(sockfd, 2)) == -1)
                perror("listen");
            size = sizeof(clientaddr);
            if ((newfd = accept(sockfd, (struct sockaddr *) &clientaddr, &size)) == -1)
                perror("accept");
            sockfd = newfd;
        }
        if (p == NULL)
        {
            std::cerr << "Unable to host\n";
            output_buffer = "Unable to host";
            return -1;
        }
    }
    break;
    case CLIENT:
    {
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;


        if ((rv = getaddrinfo(host, port, &hints,&servinfo)) != 0)
        {
            std::cerr << gai_strerror(rv) << "\n";
            output_buffer = gai_strerror(rv);
            return -1;
        }

        for (p = servinfo; p != NULL; p = p->ai_next)
        {
            sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
            if (sockfd == -1)
            {
                perror("socket");
                continue;
            }
            if (connect(sockfd,p->ai_addr,p->ai_addrlen) == -1)
            {
                perror("connect");
                close(sockfd);
                sockfd = -1;
                continue;
            }
            break;

        }
        if (p == NULL)
        {
            std::cerr << "Unable to connect.\n";
            output_buffer = "Unable to connect.";
            freeaddrinfo(servinfo);
            return -1;
        }
    }
    break;
    default:
        cerr << "Type is not correct, valid types are SERVER=1 CLIENT=2\n";

    }

    freeaddrinfo(servinfo);
    return 0;
}

int network::send_msg(const string & username, const char * msg)
{
    //------- Check the size of the msg ----------------//
    if(strlen(msg) > MAX_MSG)
    {
        std::cerr << "Message is too long!\n";
        return 2;
    }
    int length = MAX_BUFFER;
    //---- No Problem send the message -------/
    strcpy(buffer,"/msg ");
    strcat(buffer, username.c_str());
    strcat(buffer, " :");
    strncat(buffer, msg,strlen(msg));
    if(sendall(sockfd, buffer,&length) == -1)
        return 1; // Error sending

    // at success
    return 0;
}

void network::out(const string& text)
{

    output_buffer = text;
    event_signal_output.emit();
}

void network::out(char c)
{
    output_buffer = c;
    event_signal_output.emit();
}

void network::mainloop()
{


    socket_watch.disconnect();
    received = 0;
    inchat = false;
    check_ischat = false;
    colour = "darkgreen";
    first_output = 2;
    out("--- Welcome to chatnet. ---\n--- Ready to serve. ---\n");
    first_output = 0;
    // GTK widgets and network state are owned by the main-loop thread.
    socket_watch = Glib::signal_io().connect(
        sigc::mem_fun(*this, &network::on_socket_ready), sockfd,
        Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR | Glib::IO_NVAL);
}

bool network::on_socket_ready(Glib::IOCondition condition)
{
    int length = MAX_BUFFER;
    const ssize_t len = recv(sockfd, receive_buffer + received,
                             MAX_BUFFER - received, MSG_DONTWAIT);
    if(len < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
       && !(condition & (Glib::IO_HUP | Glib::IO_ERR | Glib::IO_NVAL)))
        return true;
    if(len <= 0)
    {
        output_buffer = "Fatal error: Connection closed by host.\n";
        close_socket();
        event_network_error.emit();
        return false;
    }
    received += len;
    // TCP may split a protocol frame across several readiness callbacks.
    if(received < MAX_BUFFER)
        return true;
    memcpy(buffer, receive_buffer, MAX_BUFFER);
    buffer[MAX_BUFFER] = 0;
    received = 0;

    // no problem check what the received command is
    parse_argurments(buffer,args,MAX_ARGS);
// This message is sent by chatnet server if a message was

    if(args[0] == "/msg")
    {
        // a message
        // print senders name
        if(person_inchat == args[1])
        {
            const string::size_type colon = args[2].find(':');
            if(colon != string::npos)
            {
                colour = "darkgreen";
                out("<" + person_inchat + "> " + args[2].substr(colon + 1) + "\n");
            }

        }
    }


    // not sent to the user
    else if(args[0]  == "MSGNOTSENT")
    {
        colour = "red";
        out("Your message was not sent to " + person_inchat + ".\n");
        out("The user you are trying to send the message to might be offline.\n");
    }
    // a chat
    else if(args[0] == "xchat")
    {
        if(inchat == false && !check_ischat)
        {
            person_inchat = args[1];
            colour = "darkgreen";
            out("Your are now in a chat session with " + person_inchat + ".\n");
            inchat = true;
        }

    }

    //usually an indication that a name has not been found
    else if(args[0] == "NOTFOUND")
    {
        check_ischat = false;
        colour = "red";
        out("That user has not been found.\n");
    }

    // an indication the name is illegal
    else if(args[0] == "ILLEGALNAME")
    {
        check_ischat = false;
        colour = "red";
        out("The name might contain illegal characters, or too shot or long.\n");
    }

    else if(args[0] == "xendchat")
    {
        if(inchat == true)
        {
            inchat = false;
            //person_inchat = "";
            colour = "darkorange";
            out("NOTICE: " + person_inchat + " has ended the chat session.\n");
            person_inchat = "";
        }
    }
    else if(args[0] == "chating" && check_ischat)
    {
        check_ischat = false;
        if(args[2] == "yes")
        {
            colour = "red";
            out(person_inchat + " Is already in a chat session.\n");
        }
        else if(args[2] == "no" && !inchat)
        {
            const string request = "/chat " + person_inchat;
            memset(buffer, 0, sizeof(buffer));
            request.copy(buffer, MAX_BUFFER - 1);
            if(sendall(sockfd, buffer, &length) == -1)
            {
                output_buffer = strerror(errno);
                close_socket();
                event_network_error.emit();
                return false;
            }
            inchat = true;
            colour = "darkgreen";
            out("Your are now in a chat session with " + person_inchat + ".\n");
        }
    }
    else if(args[0] == "xischat")
    {
        if(inchat == true)
        {
            strcpy(buffer,"chating ");
            strcat(buffer, args[1].c_str());
            strcat(buffer," yes");
        }
        else
        {
            strcpy(buffer,"chating ");
            strcat(buffer, args[1].c_str());
            strcat(buffer," no");
        }

        if(sendall(sockfd,buffer,&length) == -1)
        {
            //FIXME:
            output_buffer = strerror(errno);
            event_network_error.emit();
            close_socket();
            return false;
        }

    }
    else if(args[0] == "add")
    {
        output_buffer = args[1];
        event_new_user.emit();
    }
    else if(args[0] == "remove")
    {
        output_buffer = args[1];
        event_remove_user.emit();
        if(inchat == true && person_inchat == args[1])
        {
            inchat = false;
            colour = "darkorange";
            out("Notice: " + person_inchat + " has left the chat session!\n");
        }

    }
    else if(args[0] == "NOSUCHCOMMAND:")
    {
        colour = "red";
        out("The command that you entered was not found. Type help for a list of valid commands.\n");
    }
    else
    {
// just display what has been received
        colour = "black";
        out(buffer);
    }
    return true;
}


int network::check_name(const string& n)
{
    int length = MAX_BUFFER;
    int recv_bytes; // holds the number of bytes received
    string cmp; // used to compare the received buffer
    name = n;
    sprintf(buffer, "%s",name.c_str());
    if(sendall(sockfd,buffer,&length) == -1)
    {
        output_buffer = strerror(errno);
        return -1;
    }
    //----------- recv respond from chatnet server ------------//
    recv_bytes = recvall(sockfd,buffer,&length);
    if(recv_bytes == -1 || recv_bytes == 0)
    {
        output_buffer = strerror(errno);
        return -1;
    }

    buffer[recv_bytes] = 0;
    cmp = buffer;

    /*-------- If respond from chatnet server is not 'NAMEACCEPTED', then is probably
     an error. display the error to the user------------*/
    if(cmp != "NAMEACCEPTED")
    {
        temp_int = 0;
        int temp = 0;
        while(buffer[temp_int++] != ':')
            {};

        while(buffer[temp_int++] != '\0')
            buffer[temp++] = buffer[temp_int];
        buffer[temp] = 0;
        output_buffer = buffer;
        return 1;
    }

    return 0; // no problem
}

int network::input_to_server(const string& str)
{
    int length = MAX_BUFFER;

    int len;
    len = str.size();
//---------------- Check if command size is large ---------------//

// if something is received, do checks against it.
    if(len)
    {
        if(len > 546)
        {
//system("clear");
            colour = "red";
            out("Input is too long!\n");
            return 0;
        }

        // is it a command
        if(str[0] == '/')
        {

            // lets use ParseServerMsg to parse input
            // Server command
            parse_argurments(str,args,MAX_ARGS);

            if(args[0] == "/chat" && args[1][0] != '\0')
            {
                //field_2[strlen(field_2) -1] = '\0';
                //------- No problem, not trying to chat with ur self ---------//
                if(inchat == false && !check_ischat)
                {
                    person_inchat = args[1];

                    if(person_inchat != name)
                    {
                        if(check_inchat(person_inchat) == -1)
                        {
                            output_buffer = strerror(errno);
                            event_network_error.emit();
                        }
                        return 0;

                    }
                    else
                    {
                        colour = "red";
                        out ("You can't chat to your self.\n");
                        return 0;
                    }
                }
                //------- Problem user is already in a chat session --------//
                else
                {
                    colour = "darkgreen";
                    out("Your are already chating with " + person_inchat + ".\n");
                    return 0;
                }
            }


            if(str == "/endchat")
            {
                if(inchat == true)
                {
                    strcpy(buffer, "endchat ");
                    strcat(buffer, person_inchat.c_str());
                    //send(sockfd,terminal_buffer,1024);
                    person_inchat = name;
                    inchat = false;
                    colour = "darkgreen";
                    out("Chat session with " + person_inchat + " has ended.\n");
                    // send to server
                    if(sendall(sockfd,buffer,&length) == -1)
                    {
                        perror("send");
                        //FIXME
                        exit(EXIT_FAILURE);
                    }
                    return 0;
                }
                else
                {
                    colour = "red";
                    out("Your are not in a chat session.\n");
                    return 0;

                }
            }



            if(sendall(sockfd,(char* )str.c_str(),&length) == -1)
            {
                output_buffer = strerror(errno);
                event_network_error.emit();
            }

        }
        // it's a message to a client

        if(inchat == true && str[0] != '/')
        {
            if(send_msg(person_inchat,str.c_str()) != 0)
            {
                exit(EXIT_FAILURE);
            }

        }

        // user is trying to send a message
        if(inchat == false && str[0] != '/')
        {
            colour = "red";
            out("Your are not in a chat session!\n");

        }
    }
    return 0;
}


string & network::get_name()
{
    return name;
}

bool network::is_chat()
{
    return inchat;
}


/* Request chat status. The reply is handled by on_socket_ready(). */

int network::check_inchat(const string& user)
{
    int length = MAX_BUFFER;
    strcpy(buffer,"ischat ");
    strcat(buffer,user.c_str());
    if(sendall(sockfd,buffer,&length))
    {
        return -1; // error
    }
    check_ischat = true;
    return 0;
}

const string& network::get_text_colour()
{
    return colour;
}

void network::close_socket()
{
    socket_watch.disconnect();
    if(sockfd >= 0)
    {
        close(sockfd);
        sockfd = -1;
    }
    received = 0;
}

int network::get_sockfd()
{
    return sockfd;
}


void network::reset()
{
    inchat = false;
    check_ischat = false;
    person_inchat = "";
}

network::network() : sockfd(-1), received(0), inchat(false), check_ischat(false)
{
    colour = "black";
    first_output = false;
}

network::~network()
{
    close_socket();
}
