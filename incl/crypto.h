#pragma once 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#include<openssl/aes.h>
#include<gflags/gflags.h>
#include<glog/logging.h>
 
#define BUFFSIZE 4096
int key_RSA_GenerateKey();
char* key_RSA_encrypt(char* str);
char* key_RSA_decrypt(char* str);
char* key_AES_GenerateKey();
int key_AES_encrypt(char* data,int data_len,char** pp_encrypt,int* p_encrypt_len ,char* userkey);
int  key_AES_decrypt(char* p_encrypt,int encrypt_len,char** pp_plain,char* usrkey);
