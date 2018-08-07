.PHONY:clean all

WORKDIR=.
VPATH = ./src

CC=gcc
CFLGS= -Wall -g -I$(WORKDIR)/inc/
LIBFLAG = -L$(WORKDIR)/lib


BIN = keymngclient  keymngserver #myipc
#testdbapi keymngserver

all:$(BIN)
# myipc_shm.o keymng_shmop.o


myipc: myipc_shm.o  keymng_shmop.o  keymnglog.o #keymng_dbop.o 
	$(CC) $^ -o $@

keymngserver:keymngserver.o  keymngserverop.o  keymnglog.o  myipc_shm.o  keymng_shmop.o
	$(CC) $(LIBFLAG) $^ -o $@ -lpthread -litcastsocket -lmessagereal # -lclntsh  -licdbapi

keymngclient:keymngclient.o  keymnglog.o  keymngclientop.o   myipc_shm.o  keymng_shmop.o
	$(CC) $(LIBFLAG) -lpthread -litcastsocket -lmessagereal $^ -o $@ 
	export LD_LIBRARY_PATH=$(WORKDIR)/lib


 
#testdbapi:testdbapi.o  
#	$(CC) $(LIBFLAG) $^ -o $@ -lpthread  -lclntsh  -licdbapi
		
%.o:%.c
	$(CC) $(CFLGS) -c $< -o $@	

clean:
	rm -f *.o $(BIN)
	
	

