#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<openssl/evp.h>
#include<openssl/aes.h>
int main()
{
  char userKey[EVP_MAX_KEY_LENGTH];
  char iv[EVP_MAX_IV_LENGTH];
  unsigned char *data = (unsigned char*)malloc(AES_BLOCK_SIZE*3);
  unsigned char* encrypt = (unsigned char*)malloc(AES_BLOCK_SIZE*6);
  unsigned char* plain = (unsigned char*) malloc(AES_BLOCK_SIZE*6);
  EVP_CIPHER_CTX ctx;
  int ret;
  int tlen = 0;
  int mlen = 0;
  int flen = 0;
  EVP_CIPHER_CTX_cleanup(&ctx);

  ret = EVP_DecryptInit_ex(&ctx,EVP_aes_128_ecb(),NULL,(unsigned char*)userKey,(unsigned char*)iv);

  EVP_CIPHER_CTX_set_padding(&ctx,0);
  EVP_DecodeUpdate(&ctx,plain,&mlen,)

}
