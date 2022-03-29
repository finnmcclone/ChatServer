#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <Windows.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main() {
	// Initialze winsock (adapted from LM5 socket.h)
	//socket.h 
	//Originally developed by Ed Walker 
	//Adapted by Kent Jones, Scott Griffith, and Qian Mao
	//////////////////////////////////////////////////////////////////////////////////////
	WSADATA wsaData;s
	WORD sockVersion = MAKEWORD(2, 2);// changed to version 2.2 vs 2.0 for LM

	int winSock = WSAStartup(sockVersion, &wsaData);
	if (winSock != 0) {
		cerr << "Failed to Initiale WinSock" << endl;
		return;
	}
	//////////////////////////////////////////////////////////////////////////////////////////

	//The AF_INET address family 
	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);//create listening socket
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create Socket" << endl;//when socket is unable to be made
		return;
	}

	//Get IP Address and the port number
	sockaddr_in loc;//loc for location
	loc.sin_family = AF_INET;//This field contains the address family, which is always AF_INET when TCP or User Datagram Protocol (UDP) is used
	loc.sin_port = htons(54000);//This field contains the port number
	loc.sin_addr.S_un.S_addr = INADDR_ANY;//This field contains the IP address

	bind(listening, (sockaddr*)&loc, sizeof(loc));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Create the master file descriptor set and zero it
	fd_set master;
	FD_ZERO(&master);//Fills a block of memory with zeros.

	//add listening socket to master
	FD_SET(listening, &master);//Sets the bit for the file descriptor fd in the file descriptor set fdset.

	bool running = true;//keep running the program

	while (running) {
		//need to creat a copy of master file descriptor
		fd_set copy = master;

		//The select function determines the status of one or more sockets, waiting if necessary, to perform synchronous I/O.
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		//get the current socket connections
		for (int i = 0; i < socketCount; i++) {
			SOCKET sock = copy.fd_array[i];

			// Is it an new client communication
			if (sock == listening) {
				//accept connection from new client
				SOCKET client = accept(listening, nullptr, nullptr);
				//add new client to master
				FD_SET(client, &master);//Sets the bit for the file descriptor fd in the file descriptor set fdset.

				//welcome messafe
				string welcomeMsg = "Welcome to the CS313 Networks Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
			}
			else {
				// message received (adapted from LM5 socket.h)
				//socket.h 
				//Originally developed by Ed Walker 
				//Adapted by Kent Jones, Scott Griffith, and Qian Mao
				//////////////////////////////////////////////////////////////////////////////////////
				char buf[4096];//buffer
				////https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366920(v=vs.85)
				ZeroMemory(buf, 4096);//Fills a block of memory with zeros
				///////////////////////////////////////////////////////////////////////////////////////////////
				// Receive message
				int rc = recv(sock, buf, 4096, 0);
				if (rc <= 0) {
					closesocket(sock);//close the socket
					FD_CLR(sock, &master);//Clears the bit for the file descriptor fd in the file descriptor set fdset.
				}
				else
				{
					//send clinet messages
					for (int i = 0; i < master.fd_count; i++) {
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock) {
							ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";//output the socket number of client chatting
							string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);//sends message
						}
					}
				}
			}
		}
	}

	//clear listening socket from master
	FD_CLR(listening, &master);
	closesocket(listening);//close listening socket
	//no more clients can connect now

	//goodbye message
	//string msg = "Server is shutting down. Have a good winter break!!!\r\n";

	while (master.fd_count > 0) {
		//socket number
		SOCKET sock = master.fd_array[0];

		//goodbye message sends
		//send(sock, msg.c_str(), msg.size() + 1, 0);

		//clear master
		FD_CLR(sock, &master);//Clears the bit for the file descriptor fd in the file descriptor set fdset.
		closesocket(sock);//close socket
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");
}
