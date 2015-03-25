#include "httpSim.h"
using namespace std;


/* function declarations */
void getHTTPRequest(int clientSock);
void processHTTPRequest();
void getHTTPMethod();
void buildHTTPResponse();


/* global variable declarations */
unsigned short PORT = 8080;	// default port value
char *DIRECTORY;
vector<char> HTTP_REQUEST;
vector<char> HTTP_RESPONSE;
int HTTP_METHOD;
int RESPONSE_NUM = 0;
string SERVER_DATETIME;
string LAST_MODIFIED_DATETIME;
string RESPONSE_MESSAGE_BODY;
string CONTENT_TYPE;
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
   		DIRECTORY = argv[1];
    else if (argc == 3 && strcmp("-p", argv[1]) == 0)
    	PORT = (unsigned short) atoi(argv[2]);
    else if (argc == 4 && strcmp("-p", argv[1]) == 0) {
		PORT = (unsigned short) atoi(argv[2]);
		DIRECTORY = argv[3];
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

        printf("Handling client %s\n", inet_ntoa(clientAddr.sin_addr));

        getHTTPRequest(clientSock);
        processHTTPRequest();
        buildHTTPResponse();
    }
}

void getHTTPRequest(int clientSock) {
    char httpRequestBuffer[RECV_BUFF_SIZE];
    int messageLen;

    if ((messageLen = recv(clientSock, httpRequestBuffer, RECV_BUFF_SIZE, 0)) < 0)
        dieWithError((char *)"recv() failed");

    while (messageLen > 0) {
        /* build the HTTP_REQUEST as a char vector */
        for (int i = 0; i < messageLen; ++i) {
            HTTP_REQUEST.push_back(httpRequestBuffer[i]);
        }

        if ((messageLen = recv(clientSock, httpRequestBuffer, RECV_BUFF_SIZE, 0)) < 0)
            dieWithError((char *)"recv() failed"); 
    }
    close(clientSock);

    /* print out the message recieved */
    for (int i = 0; i < (int)HTTP_REQUEST.size(); ++i) {
        printf("%c", HTTP_REQUEST[i]);
    }
}


void processHTTPRequest() {
    getHTTPMethod();
    /* check for  */
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

}
























