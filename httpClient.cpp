/*********************************************************
File Name:  httpClient.cpp
Author:     Austin Brennan
Course:     CPSC 3600
Instructor: Sekou Remy
Due Date:   03/25/2015


File Description:
This file contains an implementation of a simple
http client. See readme.txt for more details.

*********************************************************/


#include "httpSim.h"
using namespace std;


/* function declarations */
void checkFlag(int index, char *argv[]);
void parseURL(string URL);


/* global variable declarations */
unsigned short PORT = 8080; 		// default port value
string FILE_PATH("/");
string SERVER_NAME("");
string OUTPUT_FILENAME("");
string HTTP_RESPONSE("");
string REQUEST_MESSAGE("GET ");
void getHTTPResponse(int);


int main (int argc, char *argv[]) {
	int sock;
    struct sockaddr_in serverAddr; /* Local address */
	struct hostent *thehost;         /* Hostent from gethostbyname() */


    if (argc == 2) // zero optional flags set
        ;// do nothing
    else if (argc == 4) // one optional flags set
        checkFlag(2, argv);
    else if (argc == 6) { // two optional flags set
        checkFlag(2, argv);
        checkFlag(4, argv);
    }
    else
       stateProperUsageAndDie();

   	string URL(argv[1]);
	parseURL(URL);

    REQUEST_MESSAGE += FILE_PATH;
    REQUEST_MESSAGE += " HTTP/1.1\r\n";
    REQUEST_MESSAGE += "Host: " + SERVER_NAME + "\r\n";
    REQUEST_MESSAGE += "Connection: close\r\n";
    REQUEST_MESSAGE += "\r\n";

    /* Create a TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        dieWithError((char *)"socket() failed");

    /* Construct the server address structure */
    memset(&serverAddr, 0, sizeof(serverAddr));    /* Zero out structure */
    serverAddr.sin_family = AF_INET;                 /* Internet addr family */
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_NAME.c_str());  /* Server IP address */
    serverAddr.sin_port   = htons(PORT);     /* Server port */

    /* If user gave a dotted decimal address, we need to resolve it  */
    if ((int)serverAddr.sin_addr.s_addr == -1) {
        thehost = gethostbyname(SERVER_NAME.c_str());
            serverAddr.sin_addr.s_addr = *((unsigned long *) thehost->h_addr_list[0]);
    }

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        dieWithError((char *)"connect() failed");

    /* Send the string to the server */
    if (send(sock, REQUEST_MESSAGE.c_str(), REQUEST_MESSAGE.length(), 0) != (int)REQUEST_MESSAGE.length())
        dieWithError((char *)"send() sent a different number of bytes than expected");

    getHTTPResponse(sock);
    close(sock);

    if (OUTPUT_FILENAME.compare("") == 0) {
    	fprintf(stdout, "%s", HTTP_RESPONSE.c_str());
    }
	else {
		ofstream output_file;
		output_file.open(OUTPUT_FILENAME.c_str());
		output_file << HTTP_RESPONSE;
		output_file.close();
	}

    return 0;
}


void getHTTPResponse(int sock) {
    char httpRequestBuffer[RECV_BUFF_SIZE];
    int messageLen;

    if ((messageLen = read(sock, httpRequestBuffer, RECV_BUFF_SIZE)) < 0)
        dieWithError((char *)"recv() failed");

    /* build the HTTP_RESPONSE as a char vector */
    for (int i = 0; i < messageLen; ++i) {
        HTTP_RESPONSE += httpRequestBuffer[i];
    }
}


void parseURL(string URL) {
	size_t serverName_endIndex;

	// basic checks for valid URL
   	if (URL.length() < HTTP_URL_SECTION + 1) // check if the url is long enough
	    dieWithError((char *)"parseURL() failed: URL is too short");
	else if (URL.substr(0, HTTP_URL_SECTION).compare("http://") != 0) // check if the url starts with "http://"
	    dieWithError((char *)"parseURL() failed: URL does not start with \"http://\"");

    serverName_endIndex = URL.find_first_of("/", HTTP_URL_SECTION);

    if (serverName_endIndex == URL.npos) { // no path 
        SERVER_NAME = URL.substr(HTTP_URL_SECTION, URL.length() - HTTP_URL_SECTION);
    }
    else {
        SERVER_NAME = URL.substr(HTTP_URL_SECTION, serverName_endIndex - HTTP_URL_SECTION);
        FILE_PATH += URL.substr(serverName_endIndex + 1, URL.length() - serverName_endIndex);
    }
}


/*  checkFlag
    input       - index and the arguments
    output      - none
    description - checks the optional flag and sets global variables accordingly
*/
void checkFlag(int index, char *argv[]) {
            if (strcmp("-p", argv[index]) == 0)
                PORT = (unsigned short) atoi(argv[index+1]);
            else if (strcmp("-o", argv[index]) == 0)
                OUTPUT_FILENAME.assign(argv[index+1]);
            else
                stateProperUsageAndDie();
}

