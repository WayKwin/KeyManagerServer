#include <stdio.h>

#include <string.h>

#include <stdlib.h>

#include <openssl/bn.h>

#include <openssl/rsa.h>

#include <openssl/pem.h>

#include<unistd.h>
#define PubKeyPath "./publickey.pem"
#define PriKeyPath "./privatekey.pem"

 

/************************************************************************

 * RSA密钥生成函数

 *

 * file: test_rsa_genkey.c

 * gcc -Wall -O2 -o test_rsa_genkey test_rsa_genkey.c -lcrypto

 *

 * author: tonglulin@gmail.com by www.qmailer.net

 ************************************************************************/

int main(int argc, char *argv[])
{
    FILE* PubKey = fopen(PubKeyPath,"w");
    FILE* PriKey = fopen(PriKeyPath,"w");

    /* 产生RSA密钥 */

    RSA *rsa = RSA_generate_key(1024, 65537, NULL, NULL);
    RSA_print_fp(stdout,rsa,0);

		//printf("RSA_size:%d\n",RSA_size(rsa));


    printf("BIGNUM: %s\n", BN_bn2hex(rsa->n));

 

    /* 提取私钥 */

    printf("PRIKEY:\n");
    PEM_write_RSAPrivateKey(stdout, rsa, NULL, NULL, 0, NULL, NULL);
    PEM_write_RSAPrivateKey(PriKey, rsa, NULL, NULL, 0, NULL, NULL);

 

    /* 提取公钥 */

    unsigned char*n_b = (unsigned char*)calloc(RSA_size(rsa), sizeof(unsigned char));

    unsigned char*e_b = (unsigned char*)calloc(RSA_size(rsa), sizeof(unsigned char));

 

    int n_size = BN_bn2bin(rsa->n, n_b);

    int b_size = BN_bn2bin(rsa->e, e_b);

 

    RSA *pubrsa = RSA_new();

    pubrsa->n = BN_bin2bn(n_b, n_size, NULL);

    pubrsa->e = BN_bin2bn(e_b, b_size, NULL);

 

    printf("PUBKEY: \n");

    PEM_write_RSAPublicKey(stdout, pubrsa);
    PEM_write_RSAPublicKey(PubKey, pubrsa);

 

    RSA_free(rsa);

    RSA_free(pubrsa);
    fclose(PriKey);
    fclose(PubKey);

 
    return 0;

}

