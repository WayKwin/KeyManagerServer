#include"../incl/keymngclientop.h"
#include "../incl/keymnglog.h"
#include  "../incl/keymng_msg.h"
#include "../incl/poolsocket.h"
#include "../incl/myipc_shm.h"
#include"../incl/keymng_shmop.h"
#include<stdio.h>
#include<string.h>
#include<unistd.h>

/*  MngClint_Info 是一个客户端的信息,应该从配置文件读取
 *  MsgKey_Req 是向服务器发送请求报文的结构体,其中信息由
 *  MngClient_Info 来填充
 *  
 *  
 *  NodeSHMInfo 是网点信息,写入网点用
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

int MngClient_InitInfo(MngClient_Info *pCltInfo)
{
  int ret = 0;
  if(pCltInfo == 0)
  {
    ret = MngClt_ParamErr;
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func main() MngClient_InitInfo() err (pCltInfo == NULL)");
  }
  // 本来是要在配置文件中读取的
  strcpy(pCltInfo->clientId,"1111");
  strcpy(pCltInfo->serverId,"1234");
  strcpy(pCltInfo->AuthCode,"1111");
  strcpy(pCltInfo->serverip,"127.0.0.1");
  pCltInfo->serverport = 8080;
  //? ????????????????????????????
  /*pCltInfo->maxnode = 1;*/
  pCltInfo->maxnode = 30;
  pCltInfo->shmkey = 0x1111;

  ret = KeyMng_ShmInit(pCltInfo->shmkey,pCltInfo->maxnode,&pCltInfo->shmhdl);
  if(ret != 0)
  {
    return ret;
  }
  //pCltInfo->shmkey = 0x;
  return ret;
}
int MngClient_Agree(MngClient_Info *pCltInfo)
{
  int ret = 0;
  int i = 0; 

  //编解码
  MsgKey_Req msgReq;
  unsigned char* outData = NULL;
  int outLen= 0;
  MsgKey_Res* msgRes = NULL;
  int type = ID_MsgKey_Res;

  //连接用
  char* server_ip = pCltInfo->serverip;
  /*printf("server_ip:%s\n",server_ip);*/
  int server_port = pCltInfo->serverport;
  /*printf("server_port:%d\n",server_port);*/
  int connfd = -1;
  int sendtime = 3;
  int connect_time = 3;
  int revtime = 3;

  memset(&msgReq, 0, sizeof(MsgKey_Req));

  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],ret,"func MngClient_Agree() begin");
  //更新
  msgReq.cmdType = KeyMng_NEWorUPDATE;
  strcpy(msgReq.clientId,pCltInfo->clientId);
  strcpy(msgReq.serverId,pCltInfo->serverId);
  /*printf("msgReq: %s,%s\n",msgReq.clientId,msgReq.serverId);*/
  strcpy(msgReq.AuthCode,pCltInfo->AuthCode);
  for(; i < 65;i++)
  {
    msgReq.r1[i] = 'a'+i;
  }
  ret = MsgEncode((void*)&msgReq,ID_MsgKey_Req,&outData,&outLen);
  /*printf("client outData:%s\n",outData);*/
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgEncode()");
    goto END;
  }

  sckClient_init();
  
  ret = sckClient_connect(server_ip,server_port,connect_time,&connfd);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() connect()");
    goto END;
  }
  
  ret = sckClient_send(connfd,sendtime,outData,outLen);
  if(ret == Sck_ErrTimeOut)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_send() Sck_ErrTimeOut");
    goto END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_send()");
    goto END;
  }

  MsgMemFree((void**)&outData,0);
  outLen = 0;

  ret = sckClient_rev(connfd,revtime,&outData,&outLen);
  if(ret == Sck_ErrTimeOut)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sc kClient_rev() Sck_ErrTimeOut");
    goto END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_rev()");
    goto END;
  }

  // 进去msgReq 会将outData第一层解码 判断类型,然后用输出型参数传递给你
  ret = MsgDecode(outData,outLen,(void**)&msgRes,&type);
  if(type != ID_MsgKey_Res)
  {
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgDecode() TypeError");
    //打日志,类型出错 
    goto END;
  }
  // rv是 编解码的返回值
  if(msgRes->rv == 0)
  {
    printf("编解码成功\n");
  }
  else 
  {
    printf("编解码失败\n");
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgDecode() DecodeError");
  }
  // 将协商内容写入共享内存
  NodeSHMInfo nodeShmInfo;
  nodeShmInfo.status = 1;
  strcpy(nodeShmInfo.serverId,pCltInfo->serverId);
  strcpy(nodeShmInfo.clientId,pCltInfo->clientId);
  nodeShmInfo.seckeyid =msgRes->seckeyid;

  // 协商密钥 abc 123 a1b2c3
  for(i = 0; i < 64; i++)
  {
    nodeShmInfo.seckey[2*i] = msgReq.r1[i];
    nodeShmInfo.seckey[2*i + 1] = msgRes->r2[i];
  }
  // bug maxnode是 1
  // 写网点密钥
  ret = KeyMng_ShmWrite(pCltInfo->shmhdl,pCltInfo->maxnode,&nodeShmInfo);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func KeyMng_ShmWrite() ");
    goto END;
  }

END:
  if(outData != NULL)
    MsgMemFree((void**)&msgRes,0);
  if(connfd > 0)
    close(connfd);
  if(msgRes != NULL)
    MsgMemFree((void**)msgRes,ID_MsgKey_Res);
  sckClient_destroy();

  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],ret,"func MngClient_Agree() end ");

  return ret;
}
//客户端 共享内存 内前8个字节读出来
//服务器 也把共享内存前8个字节读出来 比较和客户端的8个字节是否相同
//  memcmp
int MngClient_Check(MngClient_Info *pCltInfo)
{
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[0],0,"func MngClient_Check() Begin");
  MsgKey_Req msgReq;

  int ret = 0;
  int i = 0;
  int shmhdl = pCltInfo->shmhdl;   
  NodeSHMInfo nodeShmInfo;

  char* server_ip = pCltInfo->serverip;
  int  server_port = pCltInfo->serverport;
  int connect_time = 3;
  int sendtime = 3;
  int recvtime = 3;
  int connfd = -1;

  unsigned char* outData = NULL;
  int outLen= 0;
  MsgKey_Res* msgRes = NULL;
  int type = ID_MsgKey_Res;
  
  
  memset(&nodeShmInfo,0,sizeof(nodeShmInfo));
  memset(&msgReq,0,sizeof(msgReq));
  msgReq.cmdType = KeyMng_Check;
  /*printf("msgReq: %s,%s\n",pCltInfo->clientId,pCltInfo->serverId);*/
  ret = KeyMng_ShmRead(shmhdl,pCltInfo->clientId,pCltInfo->serverId,pCltInfo->maxnode,&nodeShmInfo);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func KeyMng_ShmRead()");
    goto END;
  }
  strcpy(msgReq.clientId,pCltInfo->clientId);
  strcpy(msgReq.serverId,pCltInfo->serverId);
  strcpy(msgReq.AuthCode,pCltInfo->AuthCode);
  memcpy(msgReq.r1,nodeShmInfo.seckey,KeyMng_Check_Len);
  /*printf("%d msgReq.r1%s\n",__LINE__,msgReq.r1);*/


  ret = MsgEncode((void*)&msgReq,ID_MsgKey_Req,&outData,&outLen);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgEncode()");
    goto END;
  }
  
  sckClient_init();
  
  ret = sckClient_connect(server_ip,server_port,connect_time,&connfd);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() connect()");
    goto END;
  }
   
  ret = sckClient_send(connfd,sendtime,outData,outLen);
  if(ret == Sck_ErrTimeOut)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_send() Sck_ErrTimeOut");
    goto END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_send()");
    goto END;
  }

  MsgMemFree((void**)&outData,0);
  outLen = 0;

  ret = sckClient_rev(connfd,recvtime,&outData,&outLen);
  if(ret == Sck_ErrTimeOut)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sc kClient_rev() Sck_ErrTimeOut");
    goto END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_rev()");
    goto END;
  }

  ret = MsgDecode(outData,outLen,(void**)&msgRes,&type);
  if(type != ID_MsgKey_Res)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgDecode() TypeError");
    //打日志,类型出错 
    goto END;
  }

   /*rv是 编解码的返回值*/
  if(msgRes->rv == KeyMng_Check_Same)
  {
    printf("校验成功,网点信息一致\n");
  }
  else 
  {
    /*printf("msgRes->rv:%d",msgRes->rv);*/
    printf("校验发现不一样\n");
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgDecode() DecodeError");
  }
  
END:
  if(outData != NULL)
    MsgMemFree((void**)&msgRes,0);
  if(connfd > 0)
    close(connfd);
  if(msgRes != NULL)
    MsgMemFree((void**)msgRes,ID_MsgKey_Res);
  sckClient_destroy();

  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],ret,"func MngClient_Check() end ");
  return ret;
}
int MngClient_Revoke(MngClient_Info *pCltInfo)
{
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],0,"func KeyMng_Revoke() Begin");
  // 向服务器发送 请求报文
  MsgKey_Req msgReq;

  int ret = 0;
  int i = 0;
  int shmhdl = pCltInfo->shmhdl;   
  NodeSHMInfo nodeShmInfo;

  char* server_ip = pCltInfo->serverip;
  int  server_port = pCltInfo->serverport;
  int connect_time = 3;
  int sendtime = 3;
  int recvtime = 3;
  int connfd = -1;

  unsigned char* outData = NULL;
  int outLen= 0;
  MsgKey_Res* msgRes = NULL;
  int type = ID_MsgKey_Res;
  
  
  memset(&nodeShmInfo,0,sizeof(nodeShmInfo));
  memset(&msgReq,0,sizeof(msgReq));
  msgReq.cmdType = KeyMng_Revoke;
  /*printf("msgReq: %s,%s\n",pCltInfo->clientId,pCltInfo->serverId);*/
  ret = KeyMng_ShmRead(shmhdl,pCltInfo->clientId,pCltInfo->serverId,pCltInfo->maxnode,&nodeShmInfo);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func KeyMng_ShmRead()");
    goto END;
  }
  strcpy(msgReq.clientId,pCltInfo->clientId);
  strcpy(msgReq.serverId,pCltInfo->serverId);
  strcpy(msgReq.AuthCode,pCltInfo->AuthCode);
  memcpy(msgReq.r1,nodeShmInfo.seckey,KeyMng_Check_Len);
  /*printf("%d msgReq.r1%s\n",__LINE__,msgReq.r1);*/


  ret = MsgEncode((void*)&msgReq,ID_MsgKey_Req,&outData,&outLen);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgEncode()");
    goto END;
  }
  
  sckClient_init();
  
  ret = sckClient_connect(server_ip,server_port,connect_time,&connfd);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() connect()");
    goto END;
  }
   
  ret = sckClient_send(connfd,sendtime,outData,outLen);
  if(ret == Sck_ErrTimeOut)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_send() Sck_ErrTimeOut");
    goto END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_send()");
    goto END;
  }

  MsgMemFree((void**)&outData,0);
  outLen = 0;

  ret = sckClient_rev(connfd,recvtime,&outData,&outLen);
  if(ret == Sck_ErrTimeOut)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sc kClient_rev() Sck_ErrTimeOut");
    goto END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() sckClient_rev()");
    goto END;
  }

  ret = MsgDecode(outData,outLen,(void**)&msgRes,&type);
  if(type != ID_MsgKey_Res)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgDecode() TypeError");
    //打日志,类型出错 
    goto END;
  }

   /*rv是 编解码的返回值*/
  if(msgRes->rv == KeyMng_Revoke_Suceess)
  {
    printf("网点注销成功\n");
  }
  else if(msgRes->rv == KeyMng_Revoke_NotFound)
  {
    /*printf("msgRes->rv:%d",msgRes->rv);*/
    printf("网点不存在");
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Agree() MsgDecode() DecodeError");
  }
  ret = KeyMng_ShmDelete(pCltInfo->shmhdl,pCltInfo->maxnode,&nodeShmInfo); 
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func MngClient_Revoke() KeyMng_ShmDelete()");
  }

END:
  if(outData != NULL)
    MsgMemFree((void**)&msgRes,0);
  if(connfd > 0)
    close(connfd);
  if(msgRes != NULL)
    MsgMemFree((void**)msgRes,ID_MsgKey_Res);
  sckClient_destroy();
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],ret,"func MngClient_Revoke() end ");
  return ret;
  
}

