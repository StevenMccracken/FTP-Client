/**
    C++ client example using sockets.
*/
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
    
    sleep(500000); // Change based on connection strength
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
        cout << "ERROR" << endl;
        return false;
    }
    return true;
}

int pasvConnection(int socket) {
    string reply = requestReply(socket, "PASV\r\n");
    sleep(500000);
    if(!checkStatus("227", reply)) exit(0);
    
    // Obtain substring of IP and port information from reply message
    string ip_port = reply.substr(reply.find("(") + 1, reply.find(")") - reply.find("(") - 1);
    
    // Parse ip information
    string ip_address = "", a = "", b = "";
    int i = 0, part = 0;
    for(; part < 4; i++) {
        if(ip_port[i] == ',') {
            part++;
            if(part != 4) ip_address += ".";
        } else {
            ip_address += ip_port[i];
        }
    }
    
    for(; i < ip_port.length(); i++) {
        if(ip_port[i] == ',') {
            part++;
        } else if(part == 4) {
            a += ip_port[i];
        } else if(part == 5) {
            b += ip_port[i];
        }
    }
    
    int port = atoi(a.c_str()) << 8 | atoi(b.c_str());
    cout << "Attempting connection to " << ip_address << ":" << port << endl;
    int newConnection = createConnection(ip_address, port);
    sleep(500000);
    
    return newConnection;
}

string issueCommand(int socket, string message) {
    int newConnection = pasvConnection(socket);
    cout << endl;
    
    string response = requestReply(socket, message);
    sleep(500000);
    if(!checkStatus("150", response)) exit(0);
    
    response = reply(newConnection);
    sleep(500000);
    
    close(newConnection);
    return response;
}

void sleep(int duration) {
    usleep(duration);
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
    
    // Establish initial connection
    strReply = reply(sockpi);
    if(!checkStatus("220", strReply)) exit(0);
    cout << strReply  << endl;
    
    strReply = requestReply(sockpi, "USER anonymous\r\n");
    if(!checkStatus("331", strReply)) exit(0);
    cout << strReply  << endl;
    
    strReply = requestReply(sockpi, "PASS asa@asas.com\r\n");
    if(!checkStatus("230", strReply)) exit(0);
    cout << strReply  << endl;
    
    sleep(500000);
    
    // Start issuing commands to DTP server
    strReply = issueCommand(sockpi, "LIST\r\n");
    cout << strReply << endl;
    sleep(500000);
    
    strReply = issueCommand(sockpi, "RETR NOTICE\r\n");
    cout << strReply << endl;
    sleep(500000);
    
    return 0;
}
