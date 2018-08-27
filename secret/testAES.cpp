#include<openssl/aes.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
int main()
{
  //  自定义的公钥
  unsigned char userkey[AES_BLOCK_SIZE];
  // 源数据
  unsigned char* data = (unsigned char*)malloc(AES_BLOCK_SIZE*3);
  // 加密后的密文
  unsigned char* encrypt = (unsigned char*)malloc(AES_BLOCK_SIZE*3);
  // 解密后的密文
  unsigned char* plain = (unsigned char*)malloc(AES_BLOCK_SIZE*3);
  // AES_key 对象
  AES_KEY key;
  strcpy((char *)data,"helloworld");
  memset((void*)userkey,'k',AES_BLOCK_SIZE);
  //memset((void*)data,'p',AES_BLOCK_SIZE*3);
  memset((void*)encrypt,0,AES_BLOCK_SIZE*6);
  // 设置密钥key和密钥加密长度
  AES_set_encrypt_key(userkey,AES_BLOCK_SIZE*8,&key);

  int len = 0;
  while(len < AES_BLOCK_SIZE*3)
  {
    AES_encrypt(data+len,encrypt+len,&key);
    len +=AES_BLOCK_SIZE;
  }

  AES_set_decrypt_key(userkey,AES_BLOCK_SIZE*8,&key);

  len = 0;

  while(len < AES_BLOCK_SIZE*3)
  {
    AES_decrypt(encrypt+len,plain+len,&key);
    len += AES_BLOCK_SIZE;
  }
  printf("%s\n",plain);
  printf("%s\n",data);
}
