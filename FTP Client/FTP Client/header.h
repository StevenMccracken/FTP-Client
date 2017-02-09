//
//  header.h
//  FTP Client
//
//  Created by Steven McCracken on 2/8/17.
//  Copyright © 2017 Steven McCracken. All rights reserved.
//

#ifndef header_h
#define header_h

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#define BUFFER_LENGTH 2048

using namespace std;

// Functions included in skeleton
string reply(int);
int request(int,string);
string requestReply(int,string);
int createConnection(string,int,bool);

// Extra functions
void sleep();
void sleep(int);
int pasvConnection(int);
string issueCommand(int,string,string);
bool checkStatus(string,string);

#endif /* header_h */
