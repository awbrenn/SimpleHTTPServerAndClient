/*********************************************************
File Name:  httpServer.cpp
Author:     Austin Brennan
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   03/25/2015


File Description:
This file contains an implementation of a simple
http server. See readme.txt for more details.

*********************************************************/


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
string getHTTPErrorMessage(int);
void sendHTTPResponse(int);
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

   /* Create socket for sending/receiving data */
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
        cout << "Got Here 1" << endl;
        processHTTPRequest();
        checkHostIsPresent();
        getHTTPResponeBody();
        buildHTTPResponse();
        sendHTTPResponse(clientSock);
        cout << "Got Here 2" << endl;
        cleanup();
        close(clientSock);
    }
}


// gets the httpRequest
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
}


// sends the response back to the client
void sendHTTPResponse(int sock) {
    cout << "Got Here" << endl;
    /* Send the string to the server */
    if (send(sock, HTTP_RESPONSE.c_str(), HTTP_RESPONSE.length(), 0) != (int)HTTP_RESPONSE.length())
        dieWithError((char *)"send() sent a different number of bytes than expected");
}


// gets the body of the response message
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


/* checks to see if the "Host" header field is present
   in the http request */
void checkHostIsPresent() {
    if (HTTP_REQUEST.find("\r\nHost:") != string::npos)
        RESPONSE_NUM = 200;
    else
        RESPONSE_NUM = 400;
}


// processes the http request
void processHTTPRequest() {
    getHTTPMethod();
    getPath();
}


// get the path to the desired file 
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


// get the http method of the http request
void getHTTPMethod() {
    int http_function_end_index;
    string method;

    http_function_end_index = HTTP_REQUEST.find_first_of(" "); // find the index of the first space

    /* check for bad http request */
    if (http_function_end_index == (int)HTTP_REQUEST.npos) {
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


// build the http Reesponse
void buildHTTPResponse() {
    if (RESPONSE_NUM == 200)
        buildGoodResponse();
    else
        buildBadResponse();
}


// build the response of a 200 OK response
void buildGoodResponse() {
    getServerDateTime();
    getLastModifiedTime();
    getContentType();

    HTTP_RESPONSE_HEADER = "HTTP/1.1 200 OK\r\n";
    HTTP_RESPONSE_HEADER += "Connection: close\r\n";
    HTTP_RESPONSE_HEADER += "Date: " + SERVER_DATETIME + "\r\n";
    HTTP_RESPONSE_HEADER += "Last-Modified: " + LAST_MODIFIED_DATETIME + "\r\n";
    HTTP_RESPONSE_HEADER += "Content -Type: " + CONTENT_TYPE + "\r\n";
    HTTP_RESPONSE_HEADER += "Content -Length: " + to_string(HTTP_RESPONSE_BODY.length()) + "\r\n";
    HTTP_RESPONSE_HEADER += "Server: MyLittleHTTPD/1.2\r\n";
    HTTP_RESPONSE_HEADER += "\r\n";

    if (HTTP_METHOD == 1)
        HTTP_RESPONSE = HTTP_RESPONSE_HEADER + HTTP_RESPONSE_BODY;
    else
        HTTP_RESPONSE = HTTP_RESPONSE_HEADER;
    cout << "\n" + HTTP_RESPONSE << endl;
}


// build the response of a message with error notification
void buildBadResponse() {
    HTTP_RESPONSE_HEADER = "HTTP/1.1 " + to_string(RESPONSE_NUM) +
                           getHTTPErrorMessage(RESPONSE_NUM) + "\r\n";

   HTTP_RESPONSE = HTTP_RESPONSE_HEADER;
    cout << "\n" + HTTP_RESPONSE << endl;
}


// get the http error message based on the response number
string getHTTPErrorMessage(int RESPONSE_NUM) {
    string return_string("");

    if (RESPONSE_NUM == 405)
        return_string = " METHOD NOT FOUND";
    else if (RESPONSE_NUM == 403)
        return_string = " FORBIDDEN";
    else if (RESPONSE_NUM == 404)
        return_string = " NOT FOUND";
    else if (RESPONSE_NUM == 400)
        return_string = " BAD REQUEST";

    return return_string;
}


// get the content type to be returned
void getContentType() {
    int begin_file_extension_index;
    string extension;

    begin_file_extension_index = SERVER_PATH.find_last_of(".");

    if (begin_file_extension_index == (int)string::npos)
        CONTENT_TYPE = "application/octet-stream";

    extension = SERVER_PATH.substr(begin_file_extension_index+1,
                                    SERVER_PATH.length() - begin_file_extension_index);

    if      (extension.compare("html") == 0)  CONTENT_TYPE = "text/html";
    else if (extension.compare("htm")  == 0)  CONTENT_TYPE = "text/html";
    else if (extension.compare("js")   == 0)  CONTENT_TYPE = "application/javascript";
    else if (extension.compare("txt")  == 0)  CONTENT_TYPE = "text/plain";
    else if (extension.compare("jpg")  == 0)  CONTENT_TYPE = "image/jpg";
    else if (extension.compare("pdf")  == 0)  CONTENT_TYPE = "application/pdf";
    else                                      CONTENT_TYPE = "application/octet-stream";
}


// get the datetime of the server
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


// get the last time the desired file was modified
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


// cleanup the strings for the next time the server runs
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






















