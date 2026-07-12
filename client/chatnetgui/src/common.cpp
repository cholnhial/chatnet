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

#include "global.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <iostream>


/* This function sends all data that a simple
 * call to send() might not send,
 *It handles all send() errors correctly*/
int sendall(int s, char* buf, int *len)
{
    int total = 0; // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;
    while (total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) {
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *len = total; // return number actually sent here
    return n==-1?-1:0; // return -1 on failure, 0 on success
}

/* This function is the same as the function above accept it's
 * job is to recieve data, it recv all data that simple call to recv()
 * might have not received. It also handles all recv() errors correctly*/
int recvall(int s, char* buf,int *len)
{
// the workbuffer for storing currently received buffer
    char workbuffer[*len];
// Holds the number of received bytes */
    int total = 0;
    // Holds the number of sent bytes
    int n;
    //holds the number of bytes left to received
    int bytesleft = *len;
    while (total < *len)
    {
        // recv and append to workbuffer
        n = recv(s,workbuffer+total,bytesleft,0);
        if (n== -1 || n == 0) {
            break;
        }

        // increment total by the number of received bytes
        total += n;
        bytesleft -= n;
    }
    // Copy workbuffer to to buf
    memcpy(buf,workbuffer,sizeof(workbuffer));
    switch(n)
    {
      case -1:
	return -1;
      break;
      case 0:
	return 0;
      break;
      default:
	return total;
    }
}

int parse_argurments(const string& buffer, string args[], int n)
{
using namespace std;
istringstream iss(buffer);
vector<string> tokens;
copy(istream_iterator<string>(iss),
istream_iterator<string>(),
back_inserter<vector<string> >(tokens));

int i = 0;
string temp;
for(vector<string>::iterator it = tokens.begin();it != tokens.end() && i < n; it++,i++)
{
temp = *it;

if(temp[0] != ':')
{
args[i] = *it;
}
else
{
args[i] = ""; // clear it
while(it != tokens.end())
{
args[i] = args[i] + *it + " ";
it++;
}
args[i].assign(args[i],0,args[i].size()-1);
i++;
break;
}
}
return i;
}
