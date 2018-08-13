#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "../incl/poolsocket.h"
#include "../incl/keymngserverop.h"
#include "../incl/keymnglog.h"
//所有服务器的的配置信息  做成全局变量
MngServer_Info 			mngServerInfo;
#define INIT_DEMON \
do\
{\
  if(fork() > 0) exit(0);\
  setsid();\
  if(fork() > 0) exit(0);\
}while(0)\

// fix race condition  TODO
int g_stop = 0;
void sig_handler(int signo)
{
    g_stop = 1;    
}
struct sigaction  s;


void* process(void* arg)
{
  pthread_detach(pthread_self());
  int ret = 0;
  MsgKey_Req* msgKeyReq = NULL;
  int type = 0;
  int mytime = 0;
  unsigned char* outData;
  int outDatalen;
  int64_t connfd = (int64_t) arg;
  
  unsigned char *pMsgKeyResData = NULL;
  int iMsgKeyResDataLen = 0;


  while(1)
  {
    ret = sckClient_rev(connfd,mytime,(unsigned char**)&outData,&outDatalen);
    /*printf("outData :%s\n",outData);*/
    /*return 0;*/
    if(ret == Sck_ErrTimeOut)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],ret,"sckClient_rev() TimeOut");
      goto END;
    }
    else if(ret == Sck_ErrPeerClosed)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],ret,"sckClient_rev() ClientsckServer_close");
      goto END;
    }
    else if(ret != 0)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],ret,"sckClient_rev() Error");
      goto END;
    }

    ret = MsgDecode(outData,outDatalen,\
          (void**)&msgKeyReq,&type);
    /*MsgMemFree((void**)&outData,0);*/
    /*MsgMemFree((void**)&msgKeyReq,ID_MsgKey_Req);*/
    if(type != ID_MsgKey_Req)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"MsgDecode UnKownType");
      goto END;
    }
    if(ret != 0)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"MsgDecode() error");
      goto END;
    }
    switch(msgKeyReq->cmdType)
    {
      case KeyMng_NEWorUPDATE:
        //协商报文
        //通过req构造 res   Agree 要完成 keyid r2 的协商
        /*printf("in UPDATA\n");*/
        ret = MngServer_Agree(&mngServerInfo,msgKeyReq,&pMsgKeyResData,&iMsgKeyResDataLen);
        break;
      case KeyMng_Check:
        ret = MngServer_Check(&mngServerInfo,msgKeyReq,&pMsgKeyResData,&iMsgKeyResDataLen);
        break;
      case KeyMng_Revoke:
        /*printf("im Revoke %s,%s\n");*/
        ret = MngServer_Revoke(&mngServerInfo,msgKeyReq,&pMsgKeyResData,&iMsgKeyResDataLen);
        break;
    }
    // 此处的 ret 是switch 错误处理  swtich 最好不要 goto end
    // ret 要向客户端 发送错误信息
    if(ret != 0)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"MsgDecode() error");
    }
    ret = sckClient_send(connfd,mytime,pMsgKeyResData,iMsgKeyResDataLen);
    
    if(ret == Sck_ErrTimeOut)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"sckClient_send() error");
      goto END;
    }
    else if(ret == Sck_ErrPeerClosed)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"sckClient_send() ClientClosed");
      goto END;
    }
    else if(ret != 0)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"sckClient_send() Error");
      goto END;
    }
  }

END:
  if(pMsgKeyResData != NULL)
    MsgMemFree((void**)&pMsgKeyResData,0);
  if(connfd > 0)
    sckServer_close(connfd);
  if(msgKeyReq != NULL)
    MsgMemFree((void**)&msgKeyReq,ID_MsgKey_Req);
  if(outData != NULL)
    MsgMemFree((void**)&outData,0);
  return NULL;

}

int main()
{
  /*INIT_DEMON;*/
  int ret = 0;
  int port = 8080;
  int connfd = -1;
  int mytime = 3;
  int listenfd = -1;
  pthread_t pid = 0;


  s.sa_handler = sig_handler;
  sigaction(SIGUSR1,&s,NULL);
  
 ret =  MngServer_InitInfo(&mngServerInfo);
 if(ret != 0)
 {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"MngServer_Info() ");
    goto ENDMAIN;
 }

 ret = sckServer_init(port,&listenfd);

 if(ret != 0)
 {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"sckServer_init() ");
    goto ENDMAIN;
 }
 while(!g_stop)
 {
    
    ret = sckServer_accept(listenfd,mytime,&connfd); 
    if(ret == Sck_ErrTimeOut)
    {
      /*KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[2],ret,"sckServer_accept() TimeOut");*/
      continue;
    }
    else if(ret != 0)
    {
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"sckServer_accept() Error");
      goto ENDMAIN;
    }
    /*printf(" pthread_create\n");*/
    pthread_create(&pid,NULL,process,(void*)(int64_t)connfd);

 }


ENDMAIN:
 if(listenfd > 0)
  sckServer_close(listenfd);
  // connfd本来是线程关闭,但是 doublecheck一下
  sckServer_destroy();
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[2],ret,"ServerStop()");
  printf("Server Stop\n");
 return 0;
}
