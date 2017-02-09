#include "header.h"

/**
 Program that connects to a remote server and allows for LIST, RETR, and QUIT commands to be issued to the server.
 */
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
                strReply = issueCommand(sockpi, "LIST\r\n", "150");
                cout << "--------------\n" << strReply;
                sleep();
                break;
            case 2:
                cout << "Enter the name of the file you want to read: ";
                cin >> filename;
                
                strReply = issueCommand(sockpi, "RETR " + filename + "\r\n", "150");
                cout << strReply;
                sleep();
                break;
            case 3:
                run = false;
                strReply = requestReply(sockpi, "QUIT\r\n");
                if(!checkStatus("221", strReply)) exit(0);
                sleep();
                cout << "Goodbye!" << endl;
                break;
            default:
                cout << "Invalid menu choice" << endl;
                break;
        }
    }
    return 0;
}

/**
 Funtion creates a socket connection via ip address or dns.
 
 @param host the server to connect to (ip or dns)
 @param port the port on the server to connect to
 @param hideOutput the option to hide output when creating the connection
 @return the socket of the server
 */
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

/**
 Send a command message to the server.
 
 @param sock the socket
 @param message the command to send to the server
 */
int request(int sock, string message)
{
    return send(sock, message.c_str(), message.size(), 0);
}

/**
 Function gets reply from the server.
 
 @param s the socket of the connection to the server
 */
string reply(int s)
{
    string strReply;
    int count;
    char buffer[BUFFER_LENGTH+1];
    
    sleep();
    do {
        count = recv(s, buffer, BUFFER_LENGTH, 0);
        buffer[count] = '\0';
        strReply += buffer;
    }while (count ==  BUFFER_LENGTH);
    return strReply;
}

/**
 Request a reply from the socket.
 
 @param s the socket
 @param message the command to send
 @return string containing the server reply
 */
string requestReply(int s, string message)
{
	if (request(s, message) > 0)
    {
    	return reply(s);
	}
	return "";
}

/**
 Compares the status returned from the server to a desired status.
 
 @param desiredStatus the status you expect to see returned from the server after issuing your command. Should be a number like "227"
 @param stringReply the full reply from the server, which may contain more than just a status
 @return true if the desiredStatus is found within the reply. Otherwise, false
 */
bool checkStatus(string desiredStatus, string stringReply) {
    size_t status = stringReply.find(desiredStatus);
    if (status == string::npos) {
        cout << "Error! Incorrect status returned from the server" << endl;
        return false;
    }
    return true;
}

/**
 Establishes a passive connection with the server.
 
 @param socket the socket of the main server
 @return the socket of the DTP server
 */
int pasvConnection(int socket) {
    string reply = requestReply(socket, "PASV\r\n");
    sleep();
    if(!checkStatus("227", reply)) exit(0);
    
    // Obtain substring of IP and port information from reply message
    // reply contains the ip address and 2 numbers for the port, all separated by commas
    string ip_port = reply.substr(reply.find("(") + 1, reply.find(")") - reply.find("(") - 1);
    
    // Parse ip information to create "a.b.c.d" format from "a,b,c,d,e,f"
    string ip_address = "", e = "", f = "";
    int i = 0, part = 0;
    for(; part < 4; i++) {
        if(ip_port[i] == ',') {
            part++;
            if(part != 4) ip_address += ".";
        } else {
            ip_address += ip_port[i];
        }
    }
    
    // Get the two numbers (e & f from reply) to perform (e << 8) | f
    for(; i < ip_port.length(); i++) {
        if(ip_port[i] == ',') {
            part++;
        } else if(part == 4) {
            e += ip_port[i];
        } else if(part == 5) {
            f += ip_port[i];
        }
    }
    
    int port = atoi(e.c_str()) << 8 | atoi(f.c_str());
    int newConnection = createConnection(ip_address, port, true);
    sleep();
    
    return newConnection;
}

/**
 Sends a string message to the server and returns the reply
 
 @param socket the socket established for the server
 @param message the command to send to the server. Should be formatted correctly
 @param desiredStatus the status that should be returned from the server after issuing the intended command
 @return string containing the response from the server
 */
string issueCommand(int socket, string message, string desiredStatus) {
    // Establish a passive connection. Get the socket for the DTP server
    int newConnection = pasvConnection(socket);
    
    // Get the response from the PI server after sending the user command
    string response = requestReply(socket, message);
    sleep();
    if(!checkStatus(desiredStatus, response)) exit(0);
    
    // Get the response from the DTP server
    response = reply(newConnection);
    sleep();
    
    // Close the connection to the DTP server
    close(newConnection);
    return response;
}

/**
 Default sleep function.
 */
void sleep() {
    usleep(500000); // Change based on connection strength
}

/**
 Sleep for a specific amount of time.
 @param duration the amount of sleep time in milliseconds
 */
void sleep(int duration) {
    usleep(duration);
}
