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

#define QSIZE 24
#define UPPERLIMIT 16
#define LOWERLIMIT 8
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

struct sockaddr_in clntAddr;
socklen_t clntAddrLen = sizeof(clntAddr);

void receiveDataFromClient(const char* port);
void setSocketConnection(const char* port);
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
		receiveDataFromClient(argv[1]);
	}

	return 0;
}

void receiveDataFromClient(const char* port) {
	setSocketConnection(port);
	//sendto(socketConnection, on, 1, 0, (struct sockaddr *) &server, sizeof(server));			
			
	bzero(&clntAddr, sizeof(clntAddr));
	thread listener (listenToTransmitter);
	consumeData();

	listener.join();
	
	close(socketConnection);
}

void setSocketConnection(const char* port) {
	cout << "Binding pada " << localHost << ":" << port << endl;

	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;
	addrCriteria.ai_flags = AI_PASSIVE;
	addrCriteria.ai_socktype = SOCK_DGRAM;
	addrCriteria.ai_protocol = IPPROTO_UDP;
	
	
	struct addrinfo *servAddr; // List of server addresses
	int rtnVal = getaddrinfo(NULL, port, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		cout << "get address info failed" << endl;
	}
	
	// create socket
	socketConnection = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (socketConnection < 0) {
		cout << "failed create socket" << endl;
	}
	
	// Bind to local address
	if (bind(socketConnection, servAddr->ai_addr, servAddr->ai_addrlen) < 0) {
		cout << "error binding" << endl;
	} else {
		cout << "binding success" << endl;
	}

	freeaddrinfo(servAddr);
}

void listenToTransmitter() {
	char input[256];
	
	do {
		recvfrom(socketConnection, input, 1, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
		
		byteCount++;
		buffer[head % QSIZE] = input[0];
		memset(input, 0, sizeof(input));
		
		cout << "Menerima byte ke-" << byteCount << ":" << "'" << buffer[head % QSIZE] << "'" << endl;
		head++;

		if ((abs(head-tail)) >= UPPERLIMIT) {
			cout << "sending XOFF signal" << endl;
			sendto(socketConnection, off, 1, 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr));
			send_xon = false;
			send_xoff = true;
			usleep(500000);
		}
	} while (true);
}

void consumeData() {	
	do {
		if (head != tail) {
			if ((buffer[tail % QSIZE] > 32) || (buffer[tail % QSIZE] > CR) || (buffer[tail % QSIZE] > LF) || (buffer[tail % QSIZE] > Endfile)) {
				dataCount++; 
				cout << "Mengkonsumsi byte ke-" << dataCount <<	": " << "'" << buffer[tail % QSIZE] << "'" << endl;
			}
			tail++;

			if ((abs(head-tail)) <= LOWERLIMIT) {
				cout << "sending XON signal" << endl;
				sendto(socketConnection, on, 1, 0, (struct sockaddr *) &clntAddr, sizeof(clntAddr));
				send_xon = true;
				send_xoff = false;
			}
		}			
		sleep(1);
	}  while (true);		
}
