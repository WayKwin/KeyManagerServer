#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
 
#define PRIKEY "./privatekey.pem"
#define PUBKEY "./publickey.pem"
#define BUFFSIZE 4096
 
/************************************************************************
 * RSA加密解密函数
 *
 * file: test_rsa_encdec.c
 * gcc -Wall -O2 -o test_rsa_encdec test_rsa_encdec.c -lcrypto -lssl
 *
 * author: tonglulin@gmail.com by www.qmailer.net
 ************************************************************************/
 
char *my_encrypt(char *str, char *pubkey_path)
{
    RSA *rsa = NULL;
    FILE *fp = NULL;
    char *en = NULL;
    int len = 0;
    int rsa_len = 0;
 
    if ((fp = fopen(pubkey_path, "r")) == NULL) {
        return NULL;
    }
 
    /* 读取公钥PEM，PUBKEY格式PEM使用PEM_read_RSA_PUBKEY函数 */
    if ((rsa = PEM_read_RSAPublicKey(fp, NULL, NULL, NULL)) == NULL) {
        return NULL;
    }
 
    RSA_print_fp(stdout, rsa, 0);
 
    len = strlen(str);
    rsa_len = RSA_size(rsa);
 
    en = (char *)malloc(rsa_len + 1);
    memset(en, 0, rsa_len + 1);
 
    if (RSA_public_encrypt(rsa_len, (unsigned char *)str, (unsigned char*)en, rsa, RSA_NO_PADDING) < 0) {
        return NULL;
    }
 
    RSA_free(rsa);
    fclose(fp);
 
    return en;
}
 
char *my_decrypt(char *str, char *prikey_path)
{
    RSA *rsa = NULL;
    FILE *fp = NULL;
    char *de = NULL;
    int rsa_len = 0;
 
    if ((fp = fopen(prikey_path, "r")) == NULL) {
        return NULL;
    }
 
    if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        return NULL;
    }
 
    RSA_print_fp(stdout, rsa, 0);
 
    rsa_len = RSA_size(rsa);
    de = (char *)malloc(rsa_len + 1);
    memset(de, 0, rsa_len + 1);
 
    if (RSA_private_decrypt(rsa_len, (unsigned char *)str, (unsigned char*)de, rsa, RSA_NO_PADDING) < 0) {
        return NULL;
    }
 
    RSA_free(rsa);
    fclose(fp);
 
    return de;
}
 
int main(int argc, char *argv[])
{
    char *src = "hello, world!";
    char *en = NULL;
    char *de = NULL;
 
    printf("src is: %s\n", src);
 
    en = my_encrypt(src, PUBKEY);
    printf("enc is: %s\n", en);
 
    de= my_decrypt(en, PRIKEY);
    printf("dec is: %s\n", de);
 
    if (en != NULL) {
        free(en);
    }
 
    if (de != NULL) {
        free(de);
    }
    return 0;
}
