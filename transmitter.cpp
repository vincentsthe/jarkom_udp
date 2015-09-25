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

using namespace std;

char lastChar = XON;

int socketConnection;

struct sockaddr_in server;

struct addrinfo *servAddr;

void transmitDataFromFile(const char* destHost, const char* port, const char* fileName);

void setSocketConnection(const char* destHost, const char* port);

void listenToReceiver();

void sendData(const char* fileName);

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "Wrong number of argument\n" << endl;
	} else {
		int port;
		sscanf(argv[2], "%d", &port);
		transmitDataFromFile(argv[1], argv[2], argv[3]);
	}

	return 0;
}

void transmitDataFromFile(const char* destHost, const char* port, const char* fileName) {
	setSocketConnection(destHost, port);

	thread listener (listenToReceiver);
	sendData(fileName);

	listener.join();
}

void setSocketConnection(const char* destHost, const char* port) {
	cout << "Membuat socket untuk koneksi ke " << destHost << ":" << port << endl;

	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;

	addrCriteria.ai_socktype = SOCK_DGRAM;
	addrCriteria.ai_protocol = IPPROTO_UDP;
	int rtnVal = getaddrinfo(destHost, port, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		cout << "Get address info failed" << endl;
	}
	
	socketConnection = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (socketConnection < 0) {
		cout << "Failed connecting to socket" << endl;
	} else {
		cout << "socket created" << endl;
	}
}

void listenToReceiver() {
	char buffer[256];
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr);
	do {
		recvfrom(socketConnection, buffer, 255, 0, (struct sockaddr *) &fromAddr, &fromAddrLen);
		if (buffer[0] == XON) {
			lastChar = buffer[0];
			cout << "XON diterima..." << endl;
		} else if (buffer[0] == XOFF) {
			lastChar = buffer[0];
			cout << "XOFF diterima..." << endl;
		}
		usleep(500000);
	} while (true);
}

void sendData(const char* fileName) {
	ifstream fileReader;
	fileReader.open(fileName);
	
	int i = 1;
	char input;
	char buffer[32];
	buffer[1] = 0;
	while (fileReader.get(input)) {
		do {
			sleep(1);
		} while (lastChar != XON);
		buffer[0] = input;
		cout << "Mengirim byte ke-" << i++ << ":" << "'" << buffer << "'" << endl;
		ssize_t numB = sendto(socketConnection, buffer, 1, 0, servAddr->ai_addr, servAddr->ai_addrlen);
		usleep(500000);
	}
}


