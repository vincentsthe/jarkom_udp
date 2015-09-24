#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <thread>
#include <cstdlib>
#include <unistd.h>
#include "pthread.h"

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dcomm.h"

#define QSIZE 8
#define LOWERLIMIT 4
#define DATASIZE 256

using namespace std;

char buffer[DATASIZE];

char lastChar;

char *localHost = "127.0.0.1";

int socketConnection;

int byteCount = 0,
dataCount = 0,
head = 0,
tail = 0;

char on[256];
char off[256];

bool send_xoff = false,
send_xon = true;

struct sockaddr_in server;

void receiveDataFromClient(int port);
void setSocketConnection(int port);
void listenToTransmitter();
void consumeData();

int main(int argc, char* argv[]) {
	on[0] = XON;
	off[0] = XOFF;

	if (argc != 2) {
		cout << "Wrong number of argument\n" << endl;
	} else {
		int port;
		sscanf(argv[1], "%d", &port);
		receiveDataFromClient(port);
	}

	return 0;
}

void receiveDataFromClient(int port) {
	setSocketConnection(port);
	//sendto(socketConnection, on, 1, 0, (struct sockaddr *) &server, sizeof(server));			
			
	thread listener (listenToTransmitter);
	consumeData();

	listener.join();
	
	close(socketConnection);
}

void setSocketConnection(int port) {
	cout << "Binding pada " << localHost << ":" << port << endl;

	socketConnection = socket(AF_INET, SOCK_DGRAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	// inet_aton(localHost, &server.sin_addr);
	
	if (bind(socketConnection, (const sockaddr *)&server, sizeof(server)) < 0) {
		cout << "error binding to socket" << endl;
	} else {
		cout << "binding success" << endl; // test aja
	}
	
}

/*
int bindToLocalhost(int port) {
	struct sockaddr_in server, cli_addr;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	inet_aton("127.0.0.1", &server.sin_addr);

	cout << "Binding pada " << ":" << port << "..." << endl;

	socketConnection = socket(AF_INET, SOCK_DGRAM, 0);
    	
	if (bind(socketConnection, (const sockaddr *)&server, sizeof(server)) < 0) {
		cout << "error connecting to socket" << endl;
	}

	listen(socketConnection, 1000);

    return socketConnection;
}
*/

void listenToTransmitter() {
	char input[256];
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr);
	
	do {
			recvfrom(socketConnection, input, 1, 0, (struct sockaddr *) &fromAddr, &fromAddrLen);
			
			if (head == QSIZE)
				head = 0;	
			else
				head++;
			
			byteCount++;
			buffer[head] = input[0];
			memset(input, 0, sizeof(input));
			
			cout << "Menerima byte ke-" << byteCount << ":" << "'" << buffer[head] << "'" << endl;			
	
			if ((abs(head-tail)) >= QSIZE) {
				sendto(socketConnection, off, 1, 0, (struct sockaddr *) &server, sizeof(server));
				send_xon = false;
				send_xoff = true;
			}
	} while (true);
}

void consumeData() {	
	do {
		if (head != tail) {
			tail++;
			if ((buffer[tail] > 32) || (buffer[tail] > CR) || (buffer[tail] > LF) || (buffer[tail] > Endfile)) {
				dataCount++; 
				cout << "Mengkonsumsi byte ke-" << dataCount <<	": " << "'" << buffer[tail] << "'" << endl;			
			}

			if ((abs(head-tail)) <= LOWERLIMIT) {
				sendto(socketConnection, on, 1, 0, (struct sockaddr *) &server, sizeof(server));
				send_xon = true;
				send_xoff = false;
			}
		}			
	}  while (true);		
}
