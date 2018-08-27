#pragma once 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#include<gflags/gflags.h>
#include<glog/logging.h>
 
#define PRIKEY "./privatekey.pem"
#define PUBKEY "./publickey.pem"
#define BUFFSIZE 4096
char* RSA_encrypt(unsigned char* str);
char* RSA_decrypt(unsigned char* str);
int RSA_Generate_Key( );



