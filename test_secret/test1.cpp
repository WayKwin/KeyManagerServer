#include"../incl/crypto.h"

int main(int argc,char* argv[])
{
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir="./log";

  char* message = "askdsldajdksakajdksa1"; 
  char* encrypt = NULL;
  char* plain = NULL;
  char* usrkey = key_AES_GenerateKey();
  key_RSA_GenerateKey();
  char* s =  (char*)key_RSA_encrypt(usrkey);
  usrkey =(char*)key_RSA_decrypt(s);
  int encrypt_len = 0;
  key_AES_encrypt(message,strlen(message),&encrypt,&encrypt_len,usrkey);
  key_AES_decrypt(encrypt,encrypt_len,&plain,usrkey);
  printf("%s\n",plain);
  return 0;
}
