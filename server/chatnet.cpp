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


/*** Chatnet class methods ***/


#include "chatnet.h"
#include <pthread.h>
#include "global.h"

using std::cout;
using std::endl;


//protocols used to communicate with clients
const char * protocol[] =
{
    "LONGNAME",
    "SHORTNAME",
    "ILLEGALNAME",
    "NAMEINUSE",
    "NAMEACCEPTED",
    "NOSUCHCOMMAND",
    "MSGNOTSENT",
    "NOTFOUND"
};
//--------------------------------------------------//

/* Responsible for creating a socket and listening on it.
when it accepts a connection it creates thread to handle
the connected client.*/
int chatnet::server(const char* lport)
{   
    // Initializing the mutex
    pthread_mutex_init(&mutex,NULL);

    char yes = '1'; // used with setsockopt

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

//Do DNS lookup
    if((rv = getaddrinfo(NULL,lport,&hints,&servinfo)) == -1)
    {
        cout << "Error: " << endl;
        cout << gai_strerror(rv) << endl;
        return -1;
    }

// loop through the linklist and find a correct socket
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        sfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
        if(sfd == -1)
        {
            cout << "Error: creating socket...trying again" << endl;
            continue;
        }

//The address might be used again
        if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
        {
            cerr << "Error: setsockopt" << endl;
            return -1;
        }

//Try Bind to the socket
        if(bind(sfd,p->ai_addr,p->ai_addrlen) == -1)
        {
            cout << "[X] Error: bind" << endl;
            continue;
        }
//We are binded to the socket and everything went well break the loop
        break;
    }

//Things might have not went well
    if(p == NULL)
    {
        cout << "Unable to host" << endl;
        return -1;
    }

//We are done with the linklist
    freeaddrinfo(servinfo);

//Listen on the port specified
    if(listen(sfd,10) == -1)
    {
        cout << "[X] Error: listen" << endl;
        return -1;
    }

    /* The heart of the server, this while loop will
    loop forever accepting connections and creating threads
    to handle new connected users. */
    while(true)
    {
        client_addr_size = sizeof(client_addr);
        //Accept a new client
        newfd = accept(sfd,(sockaddr *) &client_addr,&client_addr_size);

        //---------- Error occured while accepting ---------//
        if(newfd == -1)
        {
            perror("accept");
            continue;
        }

        /*This thread is started to serve a client.
        The second argurment to ThreadLauncher is the clients socket
        The client socket passed is used by the function that serves the client
        */
        ThreadLauncher launcher(this, &newfd);
        pthread_create(&client_thread,NULL, LaunchMemberFunction, &launcher);
    }

}


/*This is the thread function that serves the client
wheather the client wants to chat...etc*/
void * chatnet::serve_client(void * client_sfd)
{

//get the socket for this client
    int  client_sock = *(int *)client_sfd;

//  set up a local char buffer
    char buffer[MAX_BUFFER];
    int length = MAX_BUFFER;
//some local variables
    unsigned int code;
    int dont_complain = 0;
    int argc = 0;
    pthread_t pinger_thread;
    int location;

//Holds the name of the client
    std::string name;

    /* Read the clients name and check it against errors.
    if error handle it properly. */
    do
    {
//Read a name
        rv = get_client_name(code,name,client_sock);

        if(rv == -1)
        {
//A fatal error was detected
            close(client_sock);
            return NULL;
        }

//ERROR: name might be illegal
        if(rv != 0)
        {
            strcpy(buffer,protocol[code]);
            strcat(buffer,": ");
            if(code == 0)
                strcat(buffer,"Your nickname can only be 16 characters long.\n");
            if(code == 1)
                strcat(buffer,"Your nickname has to be atleast 3 characters long.\n");
            if(code == 2)
                strcat(buffer,"Your nickname can only contain characters and digits.\n");
            if(code == 3)
                strcat(buffer,"The nickname your are trying to use is already in use.\n");
            // Send error to client
            if(sendall(client_sock,buffer,&length) == -1)
            {
                //Fatal error
                close(client_sock);
                return NULL;
            }
        }

//Get the name until there's no error//
    }
    while(rv != 0);

//Client name has been accepted
// Let the client know that the name has been accepted
    strcpy(buffer,protocol[4]);
    if(sendall(client_sock,buffer,&length) == -1)
    {
        close(client_sock);
        return NULL;
    }


//set up client information structure
    client temp;
    temp.name = name;
    temp.sockfd = client_sock;
//add new client to the client list
    client_list.push_back(temp);

    int recv_bytes,sent_bytes;

//send the list of clients
    for(int i = 0; i < client_list.size(); i++)
    {
        temp = client_list.at(i);
        strcpy(buffer,"add ");
        strcat(buffer,temp.name.c_str());
        sendall(client_sock,buffer,&length);
    }

// now send this clients user name to all other clients
    for(int i = 0; i < client_list.size(); i++)
    {
        temp = client_list.at(i);
        strcpy(buffer,"add ");
        strcat(buffer,name.c_str());
        if(temp.sockfd != client_sock)
            sendall(temp.sockfd,buffer,&length);
    }

// start accepting commands, while the command is not /exit//
    do
    {
        // Receive the command ----//
        recv_bytes = recvall(client_sock,buffer,&length);


        /*Check for erros from recvall.
         If a error ocurred the do-while loop is
        broken and ServeClient cleans up and returns*/
        if(recv_bytes == -1 || recv_bytes == 0)
            break;
        buffer[recv_bytes -1] = 0;

        // parse the received string
        argc = parse_argurments(buffer,args,MAX_ARGS);

        //list command sends a list of connected clients names
        if(args[0] == "/list")
        {
            // Loop through the client list
            for(int i = 0; i < client_list.size(); i++)
            {
                //Get the client info structure at the location (i)
                temp = client_list.at(i);

                //send names of connected clients to client
                strcpy(buffer,temp.name.c_str());
                strcat(buffer,"\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    break; // Fatal error
            }
            strcpy(buffer,"*end of list.*\n");
            if(sendall(client_sock,buffer,&length) == -1)
                break; // error ocurred
        }

        /*msg command sends a message to a specified client.
        NOTE: the clients might not display it.
        NOTE: The client must be connected in order to send the message.
        NOTE: /msg takes exactly two argurments*/
        else if(args[0] == "/msg")
        {

            dont_complain = 0;
//Check if the client as execeeded the expected size of argurment
            if(args[1].size() > MAX_NAME_SIZE)
            {
                strcpy(buffer,"First argurment to /msg can only be 16 characters long\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1; // okay display this warning only
                continue; // got to the beginning of the loop
            }

//check if the client as execeeded the expected size of argurment
            //FIXME: the max message size needs to be set latter
            if(args[2].size() > 1025)
            {
                strcpy(buffer,"Message is too long for 512 bytes\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1; // display this warning only
                continue; // got to the beginning of the loop
            }



            /* Check if too few argurments are specified.
            /Let the client know*/
            if(argc < 2 && dont_complain == 0)
            {
                strcpy(buffer,"Remember: /msg takes 2 argurment\n");
                strcat(buffer,"/msg <name> :<msg>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1;
                continue; // got to the top of the loop
            }


//- Check if the start of message is there( ':' indicates the start of the msg)
            if(args[2][0] != ':' && dont_complain == 0)
            {
                strcpy(buffer,"Oops did you forget ':'\n");
                strcat(buffer,"/msg <name> :<msg>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    continue; // got to the beginning of the loop
            }


            int isfound;

// No problems. Message the client
            if(argc == 3 && args[2][0] == ':')
            {
                //Search for the name in the client list
                isfound = find_client(args[1],client_sock);
//Check if the client has not been found Client not found in the list
                if(isfound == -1)
                    continue;

//If the client name exist try and send  a message.
                if(isfound >= 0)
                {
                    temp = client_list.at(isfound);
                    strcpy(buffer,"/msg ");
                    strcat(buffer,name.c_str());
                    strcat(buffer," ");
                    strcat(buffer,args[2].c_str());
                    //Send message if not sending to self.
                    if(name != temp.name)
                        if(sendall(temp.sockfd,buffer,&length) == -1)
                        {
                            strcpy(buffer,protocol[6]);
                            if(sendall(client_sock,buffer,&length) == -1)
                                continue;
                        }
                }

                //Client sending a message to self,that's odd
                if(name == args[1])
                {
                    strcpy(buffer,"It's not very wise to send a message to your self\n");
                    if(sendall(client_sock,buffer,&length) == -1)
                        continue; // an  error occured, goto the top of the loop
                }

            }


        }


//exit exits the client
        else if(args[0] == "/exit")
        {

        }



        /*/chat tells another client to chat with this client.
        /NOTE: /chat takes 2 argurments*/
        else if(args[0] == "/chat")
        {
            dont_complain = 0; // use to tell other control statements to not send msg to client

//Check if there's a first argurment


//Check if the client as execeeded the expected size of argurment
            if(args[1].size() > MAX_NAME_SIZE)
            {
                strcpy(buffer,"First argurment to /chat can only be 16 characters long\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1; // okay display this warning only
                continue; // got to the beginning of the loop
            }

//report if no argurments were given or too many argurments
            if(argc < 2 && dont_complain == 0)
            {
                strcpy(buffer,"Remember: /chat takes 1 argurment\n");
                strcat(buffer,"/chat <name>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1;
                continue;
            }
            if(args[1][0] == ' ')
            {
                strcpy(buffer,"Remember: /chat takes 1 argurment\n");
                strcat(buffer,"/chat <name>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1;
                continue;
            }
            int isfound;

//check if the user the client wants to chat to is online
            isfound = find_client(args[1],client_sock);
            if(isfound == -1)
                continue;

            if(isfound >= 0)
            {
                temp = client_list.at(isfound);
                sprintf(buffer,"%s %s","xchat",name.c_str());
                if(sendall(temp.sockfd,buffer,&length) == -1)
                {
                    strcpy(buffer,temp.name.c_str());
                    strcat(buffer," Has gone offline.\n");
                    if(sendall(client_sock,buffer,&length) == -1)
                        continue;
                }
            }

        }
        /* Tells another client to end the chat it is sent to the client it's destined for.
        /endchat <name>
        */
        else if(args[0]== "endchat")
        {
            dont_complain = 0; // use to tell other control statements to not send msg to client

//Check if the client as execeeded the expected size of argurment
            if(args[1].size() > MAX_NAME_SIZE)
            {
                strcpy(buffer,"First argurment to endchat can only be 16 characters long\n");
                if(sendall(client_sock,buffer,&length) == -1)
                {

                }
                strcpy(buffer,protocol[2]);
                if(sendall(client_sock,buffer,&length) == -1)
                {

                }
                dont_complain = 1; // okay display this warning only
                continue; // got to the beginning of the loop
            }

//report if no argurments were given or too many argurments
            if(argc < 2 && dont_complain == 0)
            {
                strcpy(buffer,"Remember: endchat takes 1 argurment\n");
                strcat(buffer,"endchat <name>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1;
                continue;
            }

            int isfound;

//check if the user the client wants to chat to is online
            isfound = find_client(args[1],client_sock);
            if(isfound == -1)
                continue;

            if(isfound >= 0)
            {
                temp = client_list.at(isfound);
                strcpy(buffer, "xendchat");
                if(sendall(temp.sockfd,buffer,&length) == -1)
                {
                    strcpy(buffer,temp.name.c_str());
                    strcat(buffer," Has gone offline.\n");
                    if(sendall(client_sock,buffer,&length) == -1)
                        continue;
                }
            }

        }


        else if(args[0] == "ischat" )
        {
            dont_complain = 0; // use to tell other control statements to not send msg to client

//- Check if the client as execeeded the expected size of argurment
            /*if(args[1].size() > MAX_NAME_SIZE)
            {
                strcpy(buffer,"First argurment to /ischat can only be 16 characters long\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1; // okay display this warning only
                continue; // got to the beginning of the loop
            }*/

//report if no argurments were given or too many argurments
            if(argc < 2 && args[1].size() == 0 && dont_complain == 0)
            {
                strcpy(buffer,"Remember: /schat takes 1 argurment\n");
                strcat(buffer,"ischat <name>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1;
                continue;
            }

            int isfound;

//check if the user the client wants to chat to is online
            isfound = find_client(args[1],client_sock);
            if(isfound == -1)
            {
                continue;
            }
            if(isfound >= 0)
            {
                temp = client_list.at(isfound);
                strcpy(buffer, "xischat ");
                strcat(buffer, name.c_str());
                if(sendall(temp.sockfd,buffer,&length) == -1)
                {
                    strcpy(buffer,temp.name.c_str());
                    strcat(buffer," Has gone offline.\n");
                    if(sendall(client_sock,buffer,&length) == -1)
                        continue;
                }
            }

        }

        else if(args[0] == "chating")
        {

//use to tell other control statements not send complains to the client.


//Check if the client as execeeded the expected size of argurment
            if(args[1].size() > MAX_NAME_SIZE)
            {
                strcpy(buffer,"First argurment to 'chating' can only be 16 characters long\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1; // okay display this warning only
                continue; // got to the beginning of the loop
            }

            /*Check if too few argurments are specified.
            Let the client know */
            if(argc < 2  && dont_complain == 0)
            {
                strcpy(buffer,"Remember: chating takes 2 argurment\n");
                strcat(buffer,"chating <name> <yes|no>\n");
                if(sendall(client_sock,buffer,&length) == -1)
                    dont_complain = 1;
                continue; // got to the top of the
            }


            int isfound;

//No problems the argurments are correct
            if(argc == 3)
            {
                // Search for the name in the client list
                isfound = find_client(args[1],client_sock);
//Check if the client has not been found Client not found in the list
                if(isfound == -1)
                    continue;

// the client is found, forward the buffer to that client
                if(isfound >= 0)
                {
                    temp = client_list.at(isfound);
                    if(sendall(temp.sockfd,buffer,&length) == -1)
                    {
                        strcpy(buffer,protocol[6]);
                        if(sendall(client_sock,buffer,&length) == -1)
                            continue;
                    }
                }


            }


        }
        else
        {
            strcpy(buffer,protocol[5]);
            strcat(buffer,": ");
            strcat(buffer,"'");
            strcat(buffer,args[0].c_str());
            strcat(buffer,"'");
            strcat(buffer," Is not a valid command. Enter /help for help.\n");
            if(sendall(client_sock,buffer,&length) == -1)
                break;
        }

    }
    while(args[0] != "/exit");

    /* client has  exited */

// Remove user from the list
    location = find_client(name,client_sock);
    if(location >= 0)
        remove_client(location);
// tell every connected client to remove this client
    for(int i = 0; i < client_list.size(); i++)
    {
        temp = client_list.at(i);
        strcpy(buffer,"remove ");
        strcat(buffer,name.c_str());
        sendall(temp.sockfd,buffer,&length);
    }
    close(client_sock);
    return NULL;
}

// This function is called by ServeClient to get the name of the user a check against common errors
int chatnet::get_client_name(unsigned int & c,std::string & n,unsigned int sock)
{
    char buffer[MAX_BUFFER];
    int length = MAX_BUFFER;
    int recv_bytes;
    recv_bytes = recvall(sock,buffer,&length);
    if(recv_bytes == -1 || recv_bytes == 0)
        return -1;

    buffer[strlen(buffer) + 1] = 0;
    //check if the name is shorter than 3 characters
    if(strlen(buffer) < 3)
    {
        c = 1;
        return 1;
    }

    // check if the name is longer than 16 characters
    if(strlen(buffer) > MAX_NAME_SIZE)
    {
        c = 0;
        return 1;
    }


// check if the name is legal
    for(int i = 0; i < strlen(buffer); i++)
    {
        // @#~!*...etc are not allowed
        if(!isdigit(buffer[i]) && !isalpha(buffer[i]))
        {
            c = 2;
            return 1;
        }
    }

//check if the name already exist in the list
    client temp;
    for(int i = 0; i < client_list.size(); ++i)
    {
        temp = client_list.at(i);
        if(buffer == temp.name)
        {
            c = 3;
            return 1;
        }

    }

// No issues were found!
    n = buffer; // asign the name in the buffer to n for the caller function to use

// everthing went well return 0
    return 0;
}

void *LaunchMemberFunction(void *obj)
{
    ThreadLauncher *myLauncher = reinterpret_cast<ThreadLauncher *>(obj);
    return myLauncher->launch();
}


//This function removes a client for the client_list vector
void chatnet::remove_client(int position)
{
    pthread_mutex_lock(&mutex);
    client_list.erase(client_list.begin()+position);
    pthread_mutex_unlock(&mutex);
}


//if the name is found return the position of the clients details in client_list vector
// returns -1 if the name is not found
int chatnet::find_client(string& c_name, int socket)
{
    int found;
    client temp;
    int length = MAX_BUFFER;
    char buffer[MAX_BUFFER];
    found = -1; // will always equal -1, not until the name is found
    for(int i = 0; i < client_list.size(); i++)
    {
        temp = client_list.at(i);

        // is this the right client?
        if(temp.name == c_name)
        {
            found = i;
            break;
        }
    }


// Client not found in the list
    if(found == -1)
    {
        if(is_illegal_name(c_name))
        {
            strcpy(buffer,protocol[2]);
            if(sendall(socket,buffer,&length) == -1)
                return -1;
        }
        else
        {
            strcpy(buffer,protocol[7]);
            if(sendall(socket,buffer,&length) == -1)
                return -1;
        }


    }
    return found;
}

bool chatnet::is_illegal_name(const string& c_name)
{
    if(c_name.size() > MAX_NAME_SIZE || c_name.size() < 3)
    {
        return true;
    }
    else
    {
        for(int i = 0; i < c_name.size(); i++)
        {
            // @#~!*...etc are not allowed
            if(!isdigit(c_name[i]) && !isalpha(c_name[i]))
                return true;
        }
    }

// everything cool
    return false;
}

chatnet::~chatnet()
{

}
