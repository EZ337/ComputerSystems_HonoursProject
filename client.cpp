// Example driver/solution for Lab 2.

#include "HTTP_Request.h"
#include "HTTP_Response.h"
#include "sys/socket.h"

#include "URI.h"
#include <unistd.h>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <netdb.h>
#include <string>


using namespace std;

int main(int argc, char* argv[])
{
	// Collect info from command line
	// Expects 1 argument: the URI address
	if (argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " <URI>" << std::endl;
		return 1;
	}

	URI *uri = URI::Parse(argv[1]);
	// Check that we received a valid URI
	if (uri == nullptr)
	{
		std::cerr << "Invalid address: " << argv[1] << std::endl;
		return 2;
	}

	// If port is not defined, set port to 80
	if (!uri->Is_port_defined())
		uri->Set_port(80);

	// Create our tcp port to communicate
	//TODO Review: socket takes int protocol. Passing 0 lets the system choose but is this right?
	int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_sock < 0)
	{
		std::cerr << "Unable to create TCP socket." << std::endl;
		return 3;
	}

	// Create server and try to establish connection
	sockaddr_in server;
	socklen_t slen = sizeof(server);

	server.sin_family = AF_INET;
	hostent *hostp = gethostbyname(uri->Get_host().c_str());
	memcpy(&server.sin_addr, hostp->h_addr,hostp->h_length);

	// Try to connect
	server.sin_port = htons(uri->Get_port());
	if (connect(tcp_sock, (sockaddr*)&server, slen) < 0)
	{
		std::cerr << "Unable to connect to server via TCP." << endl;
		close(tcp_sock);
		return 4;
	}

	/*
		Build an HTTP Request for GET-ing from the specified server and path. Print that request to a
		character buffer, and send the resulting data to the server. Wait for a reply.
	*/

	HTTP_Request *request = HTTP_Request::Create_GET_request(uri->Get_path());
	std::string request_str;
	request->Print(request_str);
	std::cout << "Request string: " << request_str;

	send(tcp_sock, &request, sizeof(request), 0);

	// recv and then parse that info to check if its a valid response


	// std::string response;
	// if (!request->Get_header_value("HTTP/1.1", response))
	// {
	// 	std::cout << "Failed to read response from server"
	// }
	




	
	close(tcp_sock);
	return 0;
}
