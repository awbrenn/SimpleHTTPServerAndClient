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
void getHTTPErrorMessage(int);
void sendHTTPResponse(int);
void cleanup();


/* global variable declarations */
unsigned short PORT = 8080;	// default port value
string SERVER_PATH("./");
string SERVER_PATH_ORIGINAL("./");
string HTTP_REQUEST("");
string HTTP_RESPONSE("");
string HTTP_RESPONSE_BODY("");
string HTTP_RESPONSE_HEADER("");
int HTTP_METHOD;
string HTTP_METHOD_STR("");
int RESPONSE_NUM = 200;
string RESPONSE_NUM_MSG("");
string REQUEST_PATH("");
string SERVER_DATETIME("");
string SERVER_DATETIME_SHORT("");
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
		SERVER_PATH_ORIGINAL = argv[3];
        if (SERVER_PATH_ORIGINAL.back() != '/')
            SERVER_PATH_ORIGINAL.push_back('/');
        SERVER_PATH = SERVER_PATH_ORIGINAL;
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

        getHTTPRequest(clientSock);
        processHTTPRequest();
        checkHostIsPresent();
        getHTTPResponeBody();
        buildHTTPResponse();
        sendHTTPResponse(clientSock);
        close(clientSock);

        // print the final statement
        fprintf(stdout, "%s\t%s\t%s\t%d %s\n", HTTP_METHOD_STR.c_str(), SERVER_PATH.c_str(),
                SERVER_DATETIME_SHORT.c_str(), RESPONSE_NUM, RESPONSE_NUM_MSG.c_str());
        cleanup();
    }
}


// gets the httpRequest
void getHTTPRequest(int clientSock) {
    char httpRequestBuffer[RECV_BUFF_SIZE];
    int messageLen;

    if ((messageLen = read(clientSock, httpRequestBuffer, RECV_BUFF_SIZE)) < 0)
        dieWithError((char *)"recv() failed");

    for (int i = 0; i < messageLen; ++i) {
        HTTP_REQUEST += httpRequestBuffer[i];
    }
}


// sends the response back to the client
void sendHTTPResponse(int sock) {
    /* Send the string to the server */
    if (write(sock, HTTP_RESPONSE.c_str(), HTTP_RESPONSE.length()) != (int)HTTP_RESPONSE.length())
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
        int check = access(SERVER_PATH.c_str(), F_OK);

        if (check == 0)
            RESPONSE_NUM = 403; // do nothing
        else if (SERVER_PATH.find(" ") != string::npos) {
            RESPONSE_NUM = 400;
        }
        else
            RESPONSE_NUM = 404; // FORBIDDEN
    }
}


/* checks to see if the "Host" header field is present
   in the http request */
void checkHostIsPresent() {
    if (HTTP_REQUEST.find("\r\nHost:") != string::npos)
        ; // do nothing
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
}


// get the http method of the http request
void getHTTPMethod() {
    int http_function_end_index;

    http_function_end_index = HTTP_REQUEST.find_first_of(" "); // find the index of the first space

    /* check for bad http request */
    if (http_function_end_index == (int)HTTP_REQUEST.npos) {
        RESPONSE_NUM = 400; // malformed http request
        return;
    }

    /* set the http method based on the REQUEST method. GET & HEAD only */
    HTTP_METHOD_STR = HTTP_REQUEST.substr(0, http_function_end_index);
    if (HTTP_METHOD_STR.compare("GET") == 0) {
        HTTP_METHOD = 1;
        RESPONSE_NUM = 200;
    }
    else if (HTTP_METHOD_STR.compare("HEAD") == 0) {
        HTTP_METHOD = 2;
        RESPONSE_NUM = 200;
    }
    else if (HTTP_METHOD_STR.compare("PUT")        == 0  or
             HTTP_METHOD_STR.compare("DELETE")     == 0  or
             HTTP_METHOD_STR.compare("POST")       == 0  or
             HTTP_METHOD_STR.compare("CHECKOUT")   == 0  or
             HTTP_METHOD_STR.compare("SHOWMETHOD") == 0  or
             HTTP_METHOD_STR.compare("LINK")       == 0  or
             HTTP_METHOD_STR.compare("UNLINK")     == 0  or
             HTTP_METHOD_STR.compare("CHECKIN")    == 0  or
             HTTP_METHOD_STR.compare("TEXTSEARCH") == 0  or
             HTTP_METHOD_STR.compare("SPACEJUMP")  == 0  or
             HTTP_METHOD_STR.compare("SEARCH")     == 0) {
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
    RESPONSE_NUM_MSG = "OK";

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
}


// build the response of a message with error notification
void buildBadResponse() {
    getHTTPErrorMessage(RESPONSE_NUM);
    HTTP_RESPONSE_HEADER = "HTTP/1.1 " + to_string(RESPONSE_NUM) + " " +
                           RESPONSE_NUM_MSG + "\r\n";
    HTTP_RESPONSE_HEADER += "\r\n";

   HTTP_RESPONSE = HTTP_RESPONSE_HEADER;
}


// get the http error message based on the response number
void getHTTPErrorMessage(int RESPONSE_NUM) {
    if (RESPONSE_NUM == 405)
        RESPONSE_NUM_MSG = "METHOD NOT FOUND";
    else if (RESPONSE_NUM == 403)
        RESPONSE_NUM_MSG = "FORBIDDEN";
    else if (RESPONSE_NUM == 404)
        RESPONSE_NUM_MSG = "NOT FOUND";
    else if (RESPONSE_NUM == 400)
        RESPONSE_NUM_MSG = "BAD REQUEST";
    else
        RESPONSE_NUM_MSG = "";
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
    else if (extension.compare("css")  == 0)  CONTENT_TYPE = "text/css";
    else if (extension.compare("js")   == 0)  CONTENT_TYPE = "application/javascript";
    else if (extension.compare("txt")  == 0)  CONTENT_TYPE = "text/plain";
    else if (extension.compare("jpg")  == 0)  CONTENT_TYPE = "image/jpeg";
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

    // get time for client
    strftime (buffer,200,"%a, %d %b %Y %T",time_info);
    SERVER_DATETIME = buffer;

    // get time for server print statement
    strftime (buffer,200, "%d %b %Y %H:%M",time_info);
    SERVER_DATETIME_SHORT = buffer;
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
    SERVER_PATH = SERVER_PATH_ORIGINAL;
    HTTP_REQUEST = "";
    REQUEST_PATH = "";
    SERVER_DATETIME = "";
    LAST_MODIFIED_DATETIME = "";
    RESPONSE_MESSAGE_BODY = "";
    CONTENT_TYPE = "";
    HTTP_RESPONSE = "";
    HTTP_RESPONSE_HEADER = "";
    HTTP_RESPONSE_BODY = "";
    HTTP_METHOD_STR = "";
    SERVER_DATETIME_SHORT = "";
    RESPONSE_NUM_MSG = "";
    RESPONSE_NUM = 200;
}






















