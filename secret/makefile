all:AES RSA GenerateKey
AES:testAES.cpp
	g++ $^ -o $@ -L/usr/lib64   -lssl -lcrypto
RSA:RSA.c
	gcc $^ -o $@ -L/usr/lib64   -lssl -lcrypto
GenerateKey:GenerateKey.c
	gcc $^ -o $@ -lcrypto

