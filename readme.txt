Programmer: Austin Brennan
Class:		CPSC 3600
Professor:	Remy Sekou
Due Date:	03/25/2015

HTTP Client (simget) and Server (simhttp)

Build Instructions:
$> tar -xvzf awbrenn-hw2.tar
$> make


==== simget ====

Program Description:
This program is creates a http GET
request and sends it to a specified
server.

Use Case:
simget URL [-p port] [-O filename]

URL (Required): The requested URL.

port (Optional): The web server’s port. Default value: 8080

filename (Optional): The document (web page) will be saved as a textual 
                     file instead of printing at stdout.


==== simhttp ====

Program Description:
This program is a simple implementation
of an http server. The server supports
both HEAD and GET methods.


Use Case:
simhttp [-p port] [directory]

port (Optional): Your web server’s port. Default value: 8080

directory (Optional): The directory on the server’s file system, where 
                      the web pages are stored. Default value: ./ (the 
                      same directory as ./simhttp hosted)


