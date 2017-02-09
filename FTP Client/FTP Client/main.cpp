/**
    C++ client example using sockets.
*/
#include "header.h"

int main(int argc , char *argv[])
{
    int sockpi;
    string strReply;
    
    //TODO  arg[1] can be a dns or an IP address.
    if (argc > 2)
        sockpi = createConnection(argv[1], atoi(argv[2]), false);
    if (argc == 2)
        sockpi = createConnection(argv[1], 21, false);
    else
        sockpi = createConnection("130.179.16.134", 21, false);
    
    // Establish server connection
    strReply = reply(sockpi);
    if(!checkStatus("220", strReply)) exit(0);
    cout << strReply  << endl;
    
    strReply = requestReply(sockpi, "USER anonymous\r\n");
    if(!checkStatus("331", strReply)) exit(0);
    cout << strReply  << endl;
    
    strReply = requestReply(sockpi, "PASS asa@asas.com\r\n");
    if(!checkStatus("230", strReply)) exit(0);
    cout << strReply  << endl;
    
    sleep();
    bool run = true;
    int choice = -1;
    string filename = "";
    
    cout << "The FTP Client is now connected and running. What would you like to do?\n";
    while(run) {
        cout << "--------------\n1) List files\n2) Read a file\n3) Quit\n";
        cin >> choice;
        switch(choice) {
            case 1:
                strReply = issueCommand(sockpi, "LIST\r\n");
                cout << "--------------\n" << strReply;
                sleep();
                break;
            case 2:
                cout << "Enter the name of the file you want to read: ";
                cin >> filename;
                
                strReply = issueCommand(sockpi, "RETR " + filename + "\r\n");
                cout << strReply;
                sleep();
                break;
            case 3:
                run = false;
                strReply = requestReply(sockpi, "QUIT\r\n");
                if(!checkStatus("221", strReply)) exit(0);
                sleep();
                break;
            default:
                cout << "Invalid menu choice" << endl;
                break;
        }
    }
    return 0;
}

int createConnection(string host, int port, bool hideOutput)
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
        if(!hideOutput) cout << "by ip";
        sockaddr.sin_addr.s_addr =  inet_addr(host.c_str());
    }
    else {
        if(!hideOutput) cout << "by name";
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
    
    sleep(); // Change based on connection strength
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
    sleep();
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
    int newConnection = createConnection(ip_address, port,true);
    sleep();
    
    return newConnection;
}

string issueCommand(int socket, string message) {
    int newConnection = pasvConnection(socket);
    
    string response = requestReply(socket, message);
    sleep();
    if(!checkStatus("150", response)) exit(0);
    
    response = reply(newConnection);
    sleep();
    
    close(newConnection);
    return response;
}

void sleep() {
    usleep(500000);
}

void sleep(int duration) {
    usleep(duration);
}
