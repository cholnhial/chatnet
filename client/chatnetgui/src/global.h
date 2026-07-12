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


//NOTE: Must be included in every source file

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string>
#include <unistd.h>


#define MAX_BUFFER 512
#define MAX_MSG 512
const char* const COMMANDS = "/chat <name> - start a chat session.\n"
                             "/endchat     - end a chat session. \n"
                             "/list        - list connected users.\n"
                             "/clear       - clear the logs.\n";

 const char* const PACKAGE_VERSION = "0.2";

using std::string;
/* This function sends all data that a simple
 * call to send() might not send,
 *It handles all send() errors correctly*/
int sendall(int s,char* buf, int *len);

/* This function is the same as the function above accept it's
 * job is to recieve data, it recv all data that simple call to recv()
 * might have not received. It also handles all recv() errors correctly*/
int recvall(int s, char* buf,int *len);

int parse_argurments(const string& buffer, string args[], int n);
#endif
