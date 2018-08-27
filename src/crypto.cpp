#include"../incl/crypto.h"
#include<string.h>
#include<assert.h>
DEFINE_string(PriKeyPath, "./secret/privatekey.pem", "the privatekey file path");
DEFINE_string(PubKeyPath, "./secret/publickey.pem", "the publickey file path");
DEFINE_int64(KeyBits,1024,"RSA length");
DEFINE_int64(expoend,1024,"RSA expoend");
char* RSA_encrypt(char* str)
{
  
  RSA* rsa = NULL; 
  FILE* f = NULL;
  int data_len = 0;
  int rsa_len = 0;
  char* encode_str = NULL;
  int ret = 0;
  if(!( f = fopen(fLS::FLAGS_PriKeyPath.c_str(),"r")))
  {
    LOG(WARNING) << "PriKeyPath Error File Open Fail";
    goto END;
  }
  rsa = PEM_read_RSAPublicKey(f,NULL,NULL,NULL);

  if(rsa == NULL)
  {
    LOG(WARNING) << "PEM_read_RSAPublicKey Error";
    goto END;
  }

  data_len = strlen(str);
  rsa_len = RSA_size(rsa);

  encode_str = (char*)malloc(rsa_len+1);

  ret =RSA_public_encrypt(rsa_len, (unsigned char *)str, (unsigned char*)encode_str, rsa, RSA_NO_PADDING);
   if(ret < 0)
   {
    LOG(WARNING) << "RSA_public_encrypt()";
    goto END; 
   }
END:
  if(rsa != NULL)
    free(rsa);
  if(f != NULL)
  {
    fclose(f);
  }
  LOG(INFO)<<"RSA_encrypt() success";
  return encode_str;
}
char* RSA_decrypt(unsigned char* str)
{
  RSA* rsa = NULL;
  FILE* f = NULL;
  int rsa_len = 0;
  char* plain_str = NULL;
  int ret;

  f = fopen(fLS::FLAGS_PriKeyPath.c_str(),"r");
  if(f == NULL)
  {
    LOG(FATAL) << "PriKeyPath Error File Open File Fail";
    goto END;
  }

  rsa = PEM_read_RSAPrivateKey(f,NULL,NULL,NULL);

  if(rsa == NULL)
  {
    LOG(FATAL) << "PEM_read_RSAPrivateKey";
    goto END;
  }
  rsa_len = RSA_size(rsa);
  plain_str = (char*)malloc(rsa_len+1);
  ret = RSA_private_decrypt(rsa_len,(unsigned char*)str,(unsigned char*)plain_str,rsa,RSA_NO_PADDING);
  if(ret < 0)
  {
    LOG(FATAL) << "RSA_private_dncrypt()";
    goto END;
  }
END:
  if(rsa != NULL)
    free(rsa);
  if(f != NULL)
  {
    fclose(f);
  }
  LOG(INFO)<<"RSA_decrypt() success";
  return plain_str;
}
int RSA_Generate_Key( )
{
 FILE* PubKey = fopen(fLS::FLAGS_PubKeyPath.c_str(),"w");  
 FILE* PriKey = fopen(fLS::FLAGS_PriKeyPath.c_str(),"w");  
 assert(PubKey);
 assert(PriKey);
 // 提取私钥

 RSA *rsa = RSA_generate_key(fLI64::FLAGS_KeyBits,fLI64::FLAGS_expoend,NULL,NULL);
 assert(rsa);

  PEM_write_RSAPrivateKey(PubKey,rsa,NULL,NULL,0,NULL,NULL);

  // 提取公钥
  unsigned char*n_b = (unsigned char*)calloc(RSA_size(rsa), sizeof(unsigned char));

  unsigned char*e_b = (unsigned char*)calloc(RSA_size(rsa), sizeof(unsigned char));

  int n_size = BN_bn2bin(rsa->n, n_b);
  int b_size = BN_bn2bin(rsa->e, e_b);

  RSA *pubrsa = RSA_new();

  pubrsa->n = BN_bin2bn(n_b, n_size, NULL);
  pubrsa->e = BN_bin2bn(e_b, b_size, NULL);

  PEM_write_RSAPublicKey(PubKey, pubrsa);

  if(rsa != NULL)
  {
    RSA_free(rsa);
  }
  if(pubrsa != NULL)
  {
    RSA_free(pubrsa);
  }
  if(PriKey != NULL)
  {
    fclose(PriKey);
  }
  if(PubKey != NULL)
  {
    fclose(PubKey);
  }

  LOG(INFO)<<"RSA_Genrate_KEY() success";
  return 0;
}
