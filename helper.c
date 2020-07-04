#include "helper.h"
#include "start.h"
#pragma warning(disable:4996)
SOCKET sockt;
void auth() {
	char response = 'K';
	send(sockt, &response, 1, 0);
}
int findSize(char file_name[]) {
	FILE* fp = fopen(file_name, "r");

	if (fp == NULL) {
		printf("file not found");
		return -1;
	}
	fseek(fp, 0L, SEEK_END); //moves to end of file

	long int size = ftell(fp);

	fclose(fp); 

	return size;
}