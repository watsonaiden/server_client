#ifndef HELPER_H
#define HELPER_H

#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib") //for dynamically linked library
#pragma warning(disable:4996) 

#define DEFAULT_BUFLEN 128
#define MAX_READ 112

int findSize(char file_name[]);

void auth();


#endif