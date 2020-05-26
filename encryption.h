#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include<openssl\evp.h>
#include<openssl\err.h>
#include <openssl\dh.h>


#pragma comment(lib, "crypt32")


void handleErrors(void);
void init_Key(FILE* FP);
void init_IV(FILE* FP);
int encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* ciphertext);
int decrypt(unsigned char* ciphertext, int ciphertext_len, unsigned char* plaintext);
int trail();

#endif //ENCRYPTION_H