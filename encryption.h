#ifndef ENCRYPTION_H
#define ENCRYPTION_H
#include<openssl\evp.h>
#include<openssl\err.h>
#include <openssl\dh.h>


#pragma comment(lib, "crypt32")


void handleErrors(void);

int encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* key,
    unsigned char* iv, unsigned char* ciphertext);
int decrypt(unsigned char* ciphertext, int ciphertext_len, unsigned char* key,
    unsigned char* iv, unsigned char* plaintext);
int trail();

#endif //ENCRYPTION_H