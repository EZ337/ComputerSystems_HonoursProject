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
#include <fstream>


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
	request->Set_host(uri->Get_host());

	// HTTP Request string
	std::string request_str;
	request->Print(request_str);
	std::cout << "Request string: " << request_str;

	send(tcp_sock, request_str.c_str(), request_str.length(), 0);
	std::cout << "Sent request" << std::endl;


	// Setup receiving
	const unsigned int BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int received_total = 0, received = 0;
    bool headers_parsed = false;

	// Output location
	HTTP_Response* response;
	ofstream file;
	string filename = "index.html"; //default filename

	// recv and then parse that info to check if its a valid response
    while ((received = recv(tcp_sock, buffer + received_total, BUFFER_SIZE - received_total - 1, 0)) > 0) 
	{
        received_total += received;
		std::cout << received_total << std::endl;

        if (!headers_parsed) 
		{
            std::string headers(buffer, received_total);
            size_t pos = headers.find("\r\n\r\n"); // Find the 2 carriages that signify the end of the headers
            if (pos != string::npos) 
			{
                headers_parsed = true;
                response = HTTP_Response::Parse(buffer, pos + 4); // Start processing the info

				// Failed to parse the response from server
                if (response == nullptr) 
				{
                    cerr << "Failed to parse HTTP response headers." << endl;
                    return 6;
                }

                cout << "HTTP Status Code: " << response->Get_code() << endl;

				// Status code received. We only care about status code 200. Anything else is treated as an error
                if (response->Get_code() != 200) 
				{
                    cerr << "Request failed with status " << response->Get_code() << endl;
                    return 7;
                }

                // Determine filename from URI
                string path = uri->Get_path();
                size_t last_slash_pos = path.find_last_of('/');

				// filename = the last string after the last '/'
                if (last_slash_pos != string::npos && last_slash_pos + 1 < path.length()) 
				{
                    filename = path.substr(last_slash_pos + 1);
                }

				// Open file for writing
                file.open(filename, ios::binary);

				// Error check for failing to open the file
                if (!file.is_open()) 
				{
                    cerr << "Cannot open file: " << filename << endl;
                    return 8;
                }

                // Write the initial part of the body to the file
                file.write(buffer + pos + 4, received_total - pos - 4);
            }
        } 

		// Continue writing tot he file
		else if (file.is_open()) 
		{
            file.write(buffer, received);
        }


        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer for the next read
    }

    if (received < 0) 
	{
        cerr << "Failed to receive data from server." << endl;
        return 9;
    }

	// Close file and socket
    file.close();
    close(tcp_sock);

	// SUCCESS
    cout << "File " << filename << " has been saved." << endl;
    return 0;
}
