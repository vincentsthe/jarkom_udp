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

char lastChar;

int socketConnection;

struct sockaddr_in server;

void transmitDataFromFile(const char* destHost, int port, const char* fileName);

void setSocketConnection(const char* destHost, int port);

void listenToReceiver();

void sendData(const char* fileName);

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "Wrong number of argument\n" << endl;
	} else {
		int port;
		sscanf(argv[2], "%d", &port);
		transmitDataFromFile(argv[1], port, argv[3]);
	}

	return 0;
}

void transmitDataFromFile(const char* destHost, int port, const char* fileName) {
	setSocketConnection(destHost, port);

	thread listener (listenToReceiver);
	sendData(fileName);

	listener.join();
}

void setSocketConnection(const char* destHost, int port) {
	cout << "Membuat socket untuk koneksi ke " << destHost << ":" << port << endl;

	socketConnection = socket(AF_INET, SOCK_DGRAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	inet_aton(destHost, &server.sin_addr);
}

void listenToReceiver() {
	char buffer[256];
	struct sockaddr_storage fromAddr;
	socklen_t fromAddrLen = sizeof(fromAddr);
	do {
		recvfrom(socketConnection, buffer, 255, 0, (struct sockaddr *) &fromAddr, &fromAddrLen);
		lastChar = buffer[0];
		if (lastChar == XON) {
			cout << "XON diterima..." << endl;
		} else {
			cout << "XOFF diterima..." << endl;
		}
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
		/* do {
			sleep(1);
		} while (lastChar != XON); */
		buffer[0] = input;
		cout << "Mengirim byte ke-" << i++ << ":" << "'" << buffer << "'" << endl;
		sendto(socketConnection, buffer, 1, 0, (struct sockaddr *) &server, sizeof(server));
	}
}


