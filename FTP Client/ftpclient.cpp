/**
    C++ client example using sockets.
    This programs can be compiled in linux and with minor modification in 
	   mac (mainly on the name of the headers)
    Windows requires extra lines of code and different headers
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment(lib, "Ws2_32.lib")
...
WSADATA wsaData;
iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
...
*/
#include <iostream>    //cout
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <stdio.h> //printf
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define BUFFER_LENGTH 2048

using namespace std;

int createConnection(string host, int port)
{
    int s;
    struct sockaddr_in sockaddr;
    
    memset(&sockaddr,0, sizeof(sockaddr));
    s = socket(AF_INET,SOCK_STREAM,0);
    sockaddr.sin_family=AF_INET;
    sockaddr.sin_port= htons(port);
    
    int a1,a2,a3,a4;
    if (sscanf(host.c_str(), "%d.%d.%d.%d", &a1, &a2, &a3, &a4 ) == 4)
    {
        cout << "by ip";
        sockaddr.sin_addr.s_addr =  inet_addr(host.c_str());
    }
    else {
        cout << "by name";
        hostent *record = gethostbyname(host.c_str());
        in_addr *addressptr = (in_addr *)record->h_addr;
        sockaddr.sin_addr = *addressptr;
    }
    if(connect(s,(struct sockaddr *)&sockaddr,sizeof(struct sockaddr))==-1)
    {
        perror("connection fail");
        exit(1);
        return -1;
    }
    return s;
}

int request(int sock, string message)
{
    return send(sock, message.c_str(), message.size(), 0);
}

string reply(int s)
{
    string strReply;
    int count;
    char buffer[BUFFER_LENGTH+1];
    
    usleep(100000); // Change based on connection strength
    do {
        count = recv(s, buffer, BUFFER_LENGTH, 0);
        buffer[count] = '\0';
        strReply += buffer;
    }while (count ==  BUFFER_LENGTH);
    return strReply;
}

string requestReply(int s, string message)
{
	if (request(s, message) > 0)
    {
    	return reply(s);
	}
	return "";
}

bool checkStatus(string desiredStatus, string stringReply) {
    size_t status = stringReply.find(desiredStatus);
    if (status == string::npos) {
        cout << "Error! (" << stringReply << ")" << endl;
        return false;
    }
    return true;
}

int createDTPConnection(string strReply) {
    // Parse IP and port
    string ip_port_info = strReply.substr(strReply.find("(")+1, 23);
    for (int i = 0; i < ip_port_info.length(); i++) {
        if (ip_port_info[i] == ',' || ip_port_info[i] == ')' || ip_port_info[i] == '.')
            ip_port_info[i] = ' ';
    }
    
    stringstream ss(ip_port_info);
    string ip_address, temp;
    int counter = 0, port1, port2;
    while (ss >> temp) {
        if(counter < 3) {
            ip_address = ip_address + temp + ".";
        } else if(counter == 3) {
            ip_address += temp;
        } else if(counter == 4) {
            istringstream iss(temp);
            iss >> port1;
        } else {
            istringstream iss(temp);
            iss >> port2;
        }
        counter++;
    }
    
    port1 = port1 << 8;
    ostringstream oss;
    oss << port1 << port2;
    istringstream iss(oss.str());
    int port;
    iss >> port;
    
    cout << ip_address << ":" << port << endl;
    return createConnection(ip_address, port);
}

int main(int argc , char *argv[])
{
    int sockpi;
    string strReply;
    
    //TODO  arg[1] can be a dns or an IP address.
    if (argc > 2)
        sockpi = createConnection(argv[1], atoi(argv[2]));
    if (argc == 2)
        sockpi = createConnection(argv[1], 21);
    else
        sockpi = createConnection("130.179.16.134", 21);
    
    strReply = reply(sockpi);
    if(!checkStatus("220", strReply)) return 0;
    cout << strReply  << endl;
    
    strReply = requestReply(sockpi, "USER anonymous\r\n");
    if(!checkStatus("331", strReply)) return 0;
    cout << strReply  << endl;
    
    strReply = requestReply(sockpi, "PASS asa@asas.com\r\n");
    if(!checkStatus("230", strReply)) return 0;
    cout << strReply  << endl;
    
    // PASV
    strReply = requestReply(sockpi, "PASV\r\n");
    if(!checkStatus("227", strReply)) return 0;
    cout << strReply  << endl;
    
    int hello = createDTPConnection(strReply);
    
    //TODO implement PASV, LIST, RETR.
    // Hint: implement a function that set the SP in passive mode and accept commands.
    return 0;
}
