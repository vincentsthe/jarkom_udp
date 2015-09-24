#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <thread>
#include <cstdlib>
#include <pthread.h>

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

char buffer[QSIZE];

Byte *data;

char lastChar;

int byteCount = 0;
int dataCount = 0;
int head = 0;
int tail = 0;
int socketConnection;

char on[1];
char off[1];

int bindToLocalhost(int port);
void listenToReceiver();
void consumeByte();

int main(int argc, char* argv[]) {
	on[0] = XON;
	off[0] = XOFF;

	if (argc != 2) {
		cout << "Wrong number of argument\n" << endl;
	} else {
		int port;
		sscanf(argv[1], "%d", &port);
		socketConnection = bindToLocalhost(port);
		data = (Byte*) malloc (DATASIZE*sizeof(Byte));
		write(socketConnection, on, 1);
		while (true) {		
			listenToReceiver();
			sleep(1);
		}
		while (true) {
			thread consume (consumeByte);	
			sleep(1);	
		}
	}

	return 0;
}


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

void listenToReceiver() {
	char input[256];
		
	read(socketConnection, input, 255);
	if (head == QSIZE)
		head = 0;	
	else
		head++;
	byteCount++;
	buffer[head] = input[0];
	cout << "Menerima byte ke-" << byteCount;
	if ((abs(head-tail)) >= QSIZE) {
		write(socketConnection, off , 1);
	} 
}

void consumeByte() {
	bool valid = false;
	
	if (head != tail) {
		tail++;
		do {
			if ((buffer[tail] > 32) || (buffer[tail] > CR) || (buffer[tail] > LF) || (buffer[tail] > Endfile)) {
				valid = true;
				dataCount++; 
				cout << "Mengkonsumsi byte ke-" << dataCount <<	": " << "'" << buffer[tail] << "'";			
			}
		} while (valid);
	}

	if ((abs(head-tail)) <= LOWERLIMIT)
		write(socketConnection, on, 1);
}