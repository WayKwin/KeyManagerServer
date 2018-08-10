#include <string.h>

#include <time.h>
#include <pthread.h>
#include<signal.h>

#include "../incl/keymnglog.h"
#include "../incl/keymngserverop.h"
#include "../incl/poolsocket.h"
#include "../incl/keymng_msg.h"
#include "../incl/keymng_shmop.h"

//TODO
//#include "../incl/myipc_shm.h"
//#include "keymng_shmop.h"
//#include "icdbapi.h"
//#include "keymng_dbop.h"
//server初始化
//设计与实现 
//1 数据库信息,从配置文件中获取, secmng secmn orc1
//2 自身服务器网点信息,serverid 服务器端支持的最大网点
//3  
//初始化服务器 全局变量
int g_keyid = 0;
int MngServer_InitInfo(MngServer_Info *svrInfo)
{
  int ret;
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],0,"%s begin",__FUNCTION__);
  //TODO 配置文件
  strcpy(svrInfo->serverId,"1234");
  strcpy(svrInfo->dbuse,"root");
  strcpy(svrInfo->dbpasswd,"1234556");
  strcpy(svrInfo->dbsid,"cwasx");

  strcpy(svrInfo->serverip,"127.0.0.1");
  svrInfo->serverport = 8080;

  svrInfo->maxnode = 30;
  svrInfo->shmkey= 0x001;
  svrInfo->shmhdl = 0;

  ret = KeyMng_ShmInit(svrInfo->shmkey,svrInfo->maxnode,&svrInfo->shmhdl);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"KeyMng_ShmInit() Error");
    return ret;
  }

  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],0,"%s end",__FUNCTION__);
  return 0;
}
int  MngServer_Agree(MngServer_Info* mngServerInfo,
                     MsgKey_Req* msgKeyReq,unsigned char**pMsgKeyResData,
                     int *iMgKeyResDataLen)
{

  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],0,"MngServer_Agree()");
  int ret = 0;
  int i = 0;
  MsgKey_Res msgRes;
 
  memset(&msgRes,0,sizeof(MsgKey_Res));
 // 构建应答报文
  msgRes.rv = 0;
  strcpy(msgRes.serverId,msgKeyReq->serverId);
  strcpy(msgRes.clientId,msgKeyReq->clientId);
  for(i = 0; i < 64; i++)
  {
    msgRes.r2[i] = 'a' + i;
  }

  // 每次应答,都自增id
  g_keyid++;
  msgRes.seckeyid = g_keyid;
  // 编码应答报文
  ret = MsgEncode((void*)&msgRes,ID_MsgKey_Res,pMsgKeyResData,iMgKeyResDataLen);

  if(ret < 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],0,"MsgEncode() Error");
    goto END;
  }
  // 应答不归 这个函数管

  // 协商密钥 
  // 上面给客户端应答了之后,还要用共享内存, 写入网点信息(在数据库中)  
#if 1

  NodeSHMInfo nodeShmInfo;
  memset(&nodeShmInfo,0,sizeof(NodeSHMInfo));
  nodeShmInfo.status = 1;
  // 一个网点信息包含 客户端和服务器端ID 
  strcpy(nodeShmInfo.clientId,msgKeyReq->clientId);
  strcpy(nodeShmInfo.serverId,mngServerInfo->serverId);
  nodeShmInfo.seckeyid = g_keyid;//本来是数据库自增字段
  for(i = 0; i < 64; i++)
  {
    // 这个网点的加密规则是 A 和 B 密码的叠加
    nodeShmInfo.seckey[2*i] = msgKeyReq->r1[i];
    nodeShmInfo.seckey[2*i+1] = msgRes.r2[i];
  }
  //写入共享内存
  ret = KeyMng_ShmWrite(mngServerInfo->shmhdl,mngServerInfo->maxnode,&nodeShmInfo);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"KeyMng_ShmWrite() Error");
    goto END;
  }
#endif
END:
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],0,"MngServer_Agree() END");
  return ret;
}
int MngServer_Check(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen)
{
  int ret;
  MsgKey_Res msgkeyRes;
  NodeSHMInfo nodeShmInfo;

  memset(&nodeShmInfo,0,sizeof(nodeShmInfo));
  memset(&msgkeyRes,0,sizeof(msgkeyRes));

  /*printf("clientId->%s,serverId->%s\n",msgkeyReq->clientId,msgkeyReq->serverId);*/

  ret = KeyMng_ShmRead(svrInfo->shmhdl,msgkeyReq->clientId,msgkeyReq->serverId,svrInfo->maxnode,&nodeShmInfo); 
  if(ret != 0 && ret != -1)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"KeyMng_ShmRead()");
    return 0;
  }
  /*printf("compare :%s|%s\n",msgkeyReq->r1,nodeShmInfo.seckey);*/
  if(ret == -1)
  {
    msgkeyRes.rv = KeyMng_Check_Not_Found;
    return ret;
  }
  //  比较客户端网点和服务器网点是否相同
  if(memcmp(msgkeyReq->r1,nodeShmInfo.seckey,KeyMng_Check_Len) 
       == 0)
  {
      msgkeyRes.rv = KeyMng_Check_Same ; 
  }
  else 
  {
      msgkeyRes.rv = KeyMng_Check_Differ; 
  }
  // 组织应答报文
  /*strcpy(nodeShmInfo.clientId,msgkeyReq->clientId);*/
  /*strcpy(nodeShmInfo.serverId,msgkeyReq->serverId);*/
  /*msgkeyRes.seckeyid = 0;*/
  ret = MsgEncode((void*)&msgkeyRes,ID_MsgKey_Res,outData,datalen);
  return 0;
}

int MngServer_Revoke(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen)
{
  int ret;
  MsgKey_Res msgkeyRes;
  NodeSHMInfo nodeShmInfo;

  memset(&nodeShmInfo,0,sizeof(nodeShmInfo));
  memset(&msgkeyRes,0,sizeof(msgkeyRes));


  /*printf("clientId->%s,serverId->%s\n",msgkeyReq->clientId,msgkeyReq->serverId);*/

  ret = KeyMng_ShmDelete(svrInfo->shmhdl,svrInfo->maxnode,&nodeShmInfo); 
  if(ret != 0 && ret != -1)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"KeyMng_Delete()");
    return 0;
  }
  /*printf("compare :%s|%s\n",msgkeyReq->r1,nodeShmInfo.seckey);*/
  if(ret == -1)
  {
    msgkeyRes.rv = KeyMng_Revoke_NotFound;
    return ret;
  }
  else 
  {
    msgkeyRes.rv = KeyMng_Revoke_Suceess;
  }
  ret = MsgEncode((void*)&msgkeyRes,ID_MsgKey_Res,outData,datalen);
  return 0;
}


int MngServer_view(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen)
{
  
}
