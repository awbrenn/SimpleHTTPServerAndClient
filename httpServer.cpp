#include "httpSim.h"
using namespace std;


/* function declarations */
void getHTTPRequest(int clientSock);
void processHTTPRequest();
void getHTTPMethod();
void getPath();
void getHTTPResponeBody();
void buildHTTPResponse();
void buildGoodResponse();
void buildBadResponse();
void checkHostIsPresent();
void getServerDateTime();
void getLastModifiedTime();
void getContentType();
void cleanup();


/* global variable declarations */
unsigned short PORT = 8080;	// default port value
string SERVER_PATH("./");
string HTTP_REQUEST("");
string HTTP_RESPONSE("");
string HTTP_RESPONSE_BODY("");
string HTTP_RESPONSE_HEADER("");
int HTTP_METHOD;
int RESPONSE_NUM = 0;
string REQUEST_PATH("");
string SERVER_DATETIME("");
string LAST_MODIFIED_DATETIME("");
string RESPONSE_MESSAGE_BODY("");
string CONTENT_TYPE("");
int CONTENT_LENGTH;


int main (int argc, char *argv[]) {
	int serverSock;
    struct sockaddr_in serverAddr; /* Local address */
    struct sockaddr_in clientAddr; /* Client address */
    int clientSock;
    unsigned int clientLen;

    if (argc == 1) // zero optional flags set
    	; // do nothing
   	else if (argc == 2)
   		SERVER_PATH = argv[1];
    else if (argc == 3 && strcmp("-p", argv[1]) == 0)
    	PORT = (unsigned short) atoi(argv[2]);
    else if (argc == 4 && strcmp("-p", argv[1]) == 0) {
		PORT = (unsigned short) atoi(argv[2]);
		SERVER_PATH = argv[3];
    }
    else
       stateProperUsageAndDie();

   /* Create socket for sending/receiving datagrams */
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        dieWithError((char *)"socket() failed");

    /* Construct local address structure */
    memset(&serverAddr, 0, sizeof(serverAddr));   /* Zero out structure */
    serverAddr.sin_family = AF_INET;                /* Internet address family */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    serverAddr.sin_port = htons(PORT);      /* Local port */

    /* Bind to the local address */
    if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        dieWithError((char *)"bind() failed");

    if (listen(serverSock, MAX_PENDING) < 0)
        dieWithError((char *)"listen() failed");

    while(true) {
        /* setting the size of the in-out parameter */
        clientLen = sizeof(clientAddr);
        
        /* waiting for the client to connect */
        if ((clientSock = accept(serverSock, (struct sockaddr *) &clientAddr, 
		               &clientLen)) < 0)
            dieWithError((char *)"accept() failed");

        // printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));

        getHTTPRequest(clientSock);
        processHTTPRequest();
        checkHostIsPresent();
        getHTTPResponeBody();
        buildHTTPResponse();
        cleanup();
    }
}


void getHTTPResponeBody() {
    ifstream fs(SERVER_PATH);
    if (fs) {
        string body((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());
        HTTP_RESPONSE_BODY = body;
    }
    else {
        RESPONSE_NUM = 404;
    }
}


void checkHostIsPresent() {
    if (HTTP_REQUEST.find("\r\nHost:") != string::npos)
        RESPONSE_NUM = 200;
    else
        RESPONSE_NUM = 400;
}


void getHTTPRequest(int clientSock) {
    char httpRequestBuffer[RECV_BUFF_SIZE];
    int messageLen;

    if ((messageLen = recv(clientSock, httpRequestBuffer, RECV_BUFF_SIZE, 0)) < 0)
        dieWithError((char *)"recv() failed");

    while (messageLen > 0) {
        /* build the HTTP_REQUEST as a char vector */
        for (int i = 0; i < messageLen; ++i) {
            HTTP_REQUEST += httpRequestBuffer[i];
        }

        if ((messageLen = recv(clientSock, httpRequestBuffer, RECV_BUFF_SIZE, 0)) < 0)
            dieWithError((char *)"recv() failed"); 
    }
    close(clientSock);

    /* print out the message recieved */

}


void processHTTPRequest() {
    getHTTPMethod();
    getPath();
}


void getPath() {
    int begin_of_path_index;
    int end_of_path_index;

    begin_of_path_index = HTTP_REQUEST.find_first_of("/");
    end_of_path_index = HTTP_REQUEST.find_first_of(" ", begin_of_path_index);

    if (end_of_path_index == begin_of_path_index + 1)
        return;
    else
        SERVER_PATH += HTTP_REQUEST.substr(begin_of_path_index + 1, (end_of_path_index - begin_of_path_index) - 1);

    cout << SERVER_PATH << endl;
}


void getHTTPMethod() {
    int http_function_end_index;
    string method;

    http_function_end_index = HTTP_REQUEST.find_first_of(" "); // find the index of the first space

    /* check for bad http request */
    if (http_function_end_index == HTTP_REQUEST.npos) {
        RESPONSE_NUM = 400; // malformed http request
        return;
    }

    /* set the http method based on the REQUEST method. GET & HEAD only */
    method = HTTP_REQUEST.substr(0, http_function_end_index);
    if (method.compare("GET") == 0) {
        HTTP_METHOD = 1;
        RESPONSE_NUM = 200;
    }
    else if (method.compare("HEAD") == 0) {
        HTTP_METHOD = 2;
        RESPONSE_NUM = 200;
    }
    else if (method.compare("PUT")        == 0  or
             method.compare("DELETE")     == 0  or
             method.compare("POST")       == 0  or
             method.compare("CHECKOUT")   == 0  or
             method.compare("SHOWMETHOD") == 0  or
             method.compare("LINK")       == 0  or
             method.compare("UNLINK")     == 0  or
             method.compare("CHECKIN")    == 0  or
             method.compare("TEXTSEARCH") == 0  or
             method.compare("SPACEJUMP")  == 0  or
             method.compare("SEARCH")     == 0) {
             /*
                basically if you use any other valid http method
                just set the HTTP_METHOD to -1
             */
        HTTP_METHOD = -1;
        RESPONSE_NUM = 405;
    }
    else {
        RESPONSE_NUM = 400;
    }
}


void buildHTTPResponse() {
    if (RESPONSE_NUM == 200)
        buildGoodResponse();
    else
        buildBadResponse();
}


void buildGoodResponse() {
    getServerDateTime();
    getLastModifiedTime();
    getContentType();

    HTTP_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\n";
    HTTP_RESPONSE_HEADER += "Connection: close\r\n";
    HTTP_RESPONSE_HEADER += "Date: " + SERVER_DATETIME + "\r\n";
    HTTP_RESPONSE_HEADER += "Last-Modified: " + LAST_MODIFIED_DATETIME + "\r\n";
    HTTP_RESPONSE_HEADER += "Content -Length: " + to_string(HTTP_RESPONSE_BODY.length()) + "\r\n";
    HTTP_RESPONSE_HEADER += "Server: MyLittleHTTPD/1.2\r\n";
    HTTP_RESPONSE_HEADER += "\r\n";

    HTTP_RESPONSE = HTTP_RESPONSE_HEADER + HTTP_RESPONSE_BODY;
    cout << "\n" + HTTP_RESPONSE << endl;
}

void buildBadResponse() {
    //stubbed
    cout << "\nBAD RESPONSE " << RESPONSE_NUM << endl;
}

void getContentType() {
    begin_file_name_index;
}

void getServerDateTime() {
    char buffer[200];
    time_t current_time;
    struct tm *time_info;

    current_time = time(NULL);
    time_info = localtime(&current_time);
    if (time_info == NULL)
        dieWithError((char *)"localtime() failed: unable to get time of server");


    strftime (buffer,200,"%a, %d %b %Y %T",time_info);
    SERVER_DATETIME = buffer;
}


void getLastModifiedTime() {
    char buffer[200];
    struct stat stat_buffer;
    struct tm *time_info;

    stat(SERVER_PATH.c_str(), &stat_buffer);
    // get last modified time by stat_buffer.st_mtime
    time_info = localtime(&stat_buffer.st_mtime);
    if (time_info == NULL)
        dieWithError((char *)"localtime() failed: unable to get time of server");

    strftime (buffer,200,"%a, %d %b %Y %T",time_info);

    LAST_MODIFIED_DATETIME = buffer;
}


void cleanup() {
    SERVER_PATH = "./";
    HTTP_REQUEST = "";
    REQUEST_PATH = "";
    SERVER_DATETIME = "";
    LAST_MODIFIED_DATETIME = "";
    RESPONSE_MESSAGE_BODY = "";
    CONTENT_TYPE = "";
    HTTP_RESPONSE = "";
    HTTP_RESPONSE_HEADER = "";
    HTTP_RESPONSE_BODY = "";
}






















