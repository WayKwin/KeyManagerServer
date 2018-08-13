#include<string.h>
#include "../incl/myipc_shm.h"
#include"../incl/keymng_shmop.h"
#include"../incl/keymnglog.h"
#include"../incl/keymngclientop.h"

int  KeyMng_ShmInit(int key,int maxnodenum,int * shmhdl)
{
  int ret = 0;
  void *mapddr = NULL;
  ret = IPC_OpenShm(key,sizeof(NodeSHMInfo)*maxnodenum,shmhdl);
  // 不存在共享内存
  if(ret == MYIPC_NotEXISTErr)
  {
    printf(" 没有共享内存,开始创建...\n");
    // 创建新的共享内存
    ret = IPC_CreatShm(key,sizeof(NodeSHMInfo)*maxnodenum,shmhdl);
    if(ret != 0)
    {
        printf("func IPC_CeatShm() err: %d line: %d\n",ret,__LINE__);
        return ret;
    }
    else 
    {
      //映射内存
        printf("系统创建共享内存 ok, 开始清空共享内存\n");
        ret = IPC_MapShm(*shmhdl,&mapddr);
        if(ret != 0)
        {
        printf("func IPC_MapShm() err: %d line: %d\n",ret,__LINE__);
          // 映射共享内存失败
          return ret;
        }
        memset(mapddr,0,sizeof(NodeSHMInfo)*maxnodenum);

        ret = IPC_UnMapShm(mapddr);
        printf("系统创建共享内存 ok,清空共享内存 ok\n");
    }
  }
  // 有旧的共享内存,使用旧的
  else if (ret == 0)
  {
    printf("系统检测到有共享内存, 使用旧的共享内存 \n"); 
    /*printf("")*/
  }
  else 
  {
    printf("系统监测有旧的共享内存,但打开共享内存失败\n"); 
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func KeyMng_ShmInit() err:");
    // 失败
  }
  return ret; 
}
int KeyMng_ShmWrite(int shmhdl, int maxnodenum, NodeSHMInfo *pNodeInfo)
{
  int ret = 0, i = 0;
  NodeSHMInfo* CurNode = NULL;
  NodeSHMInfo ZeroNode;
  char* addr = NULL;
  memset(&ZeroNode,0x00,sizeof(ZeroNode));
  if(pNodeInfo == NULL)
  {
    ret = MngClt_ParamErr; 
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func KeyMng_ShmWrite() err:");
    goto END;
  }

  //获得共享内存
  ret = IPC_MapShm(shmhdl,(void**)&addr);
  if(addr == NULL)
  {
    printf("addr NULL\n");
    goto  END;
  }
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"func IPC_MapShm() err:");
    goto END;
  }
  for(i = 0; i < maxnodenum; i++)
  {
    CurNode = (NodeSHMInfo*)(addr + i * sizeof(NodeSHMInfo));
    if(CurNode->status == 1 && strcmp(pNodeInfo->serverId,CurNode->serverId) == 0 &&
        strcmp(pNodeInfo->clientId,CurNode->clientId) == 0)
    {
      // 更新密钥数据
      printf("密钥已经存在,更新密钥\n");
      /*printf("clientId->%s,serverId->%s\n",CurNode->clientId,CurNode->serverId);*/
      memcpy(CurNode,pNodeInfo,sizeof(NodeSHMInfo));
      CurNode->status = 1;
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[2],ret,"网点密钥已经存在,已重新覆盖密钥信息");
      goto END;
    }
  }
  for(i = 0; i < maxnodenum; i++)
  {
    //bug  注意第二个括号
    CurNode = (NodeSHMInfo*)(addr + i*sizeof(NodeSHMInfo));
    if(CurNode->status == 0 || memcmp(CurNode,&ZeroNode,sizeof(NodeSHMInfo)) == 0)
    {
      //新加入的网点信息
      printf("密钥不存在,新增密钥\n");
      /*printf("clientId->%s,serverId->%s\n",CurNode->clientId,CurNode->serverId);*/
      memcpy(CurNode,pNodeInfo,sizeof(NodeSHMInfo));
      CurNode->status = 1;
      KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[2],ret,"插入新的网点信息");
      goto END;
    }
  }
  if(i == maxnodenum)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"网点信息已满");
    goto END;
  }
END:
  if(addr != NULL) IPC_UnMapShm(addr);
  return ret;
}
int KeyMng_ShmRead(int shmhdl, char *clientId, char *serverId,  int maxnodenum, NodeSHMInfo *pNodeInfo)
{
  int ret = 0, i = 0;
  NodeSHMInfo* CurNode = NULL;
  char* addr = NULL;
  if(clientId == NULL || serverId == NULL ||
      pNodeInfo == NULL)
  {
    ret = MngClt_ParamErr;
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"KeyMng_ShmRead() err %d\n",ret);
    goto END;
  }
  ret = IPC_MapShm(shmhdl,(void**)&addr);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"IPC_MapShm() err %d\n",ret);
    goto END;
  }
  
  for( i = 0; i < maxnodenum; i++)
  {
    CurNode = (NodeSHMInfo*)(addr + i * sizeof(NodeSHMInfo));
    /*printf("clientId->%s,serverId->%s\n",CurNode->clientId,CurNode->serverId);*/
    if( CurNode->status == 1 && strcmp(CurNode->clientId,clientId) == 0 &&
        strcmp(CurNode->serverId,serverId) == 0 )
    {
        printf("找到网点信息\n");
        /*printf("clientId->%s,serverId->%s\n",CurNode->clientId,CurNode->serverId);*/
        memcpy(pNodeInfo,CurNode,sizeof(NodeSHMInfo));
        goto END;
    }
  }
  ret = -1;
  printf("网点信息不存在\n");
END:
  if(addr != NULL)
    IPC_UnMapShm(addr);
    return ret;
}
int KeyMng_ShmDelete(int shmhdl,int maxnodenum, NodeSHMInfo *pNodeInfo)
{
  int ret = 0, i = 0;
  NodeSHMInfo* CurNode = NULL;
  char* addr = NULL;
  char *clientId = NULL;
  char *serverId = NULL;
  if( pNodeInfo == NULL)
  {
    ret = MngClt_ParamErr;
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"KeyMng_ShmRead() err %d\n",ret);
    goto END;
  }
  /*printf("clientId->%s,serverId->%s\n",pNodeInfo->clientId,pNodeInfo->serverId);*/
  clientId = pNodeInfo->clientId ;
  serverId = pNodeInfo->serverId;
  ret = IPC_MapShm(shmhdl,(void**)&addr);
  if(ret != 0)
  {
    KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],ret,"IPC_MapShm() err %d\n",ret);
    goto END;
  }
  
  for( i = 0; i < maxnodenum; i++)
  {
    CurNode = (NodeSHMInfo*)(addr + i * sizeof(NodeSHMInfo));
    /*printf("maxnodenum:%d\n",maxnodenum);*/
    if( CurNode->status == 1 && strcmp(CurNode->clientId,clientId) == 0 &&
        strcmp(CurNode->serverId,serverId) == 0 )
    {
        memset((void*)CurNode,0,sizeof(CurNode));
        CurNode->status = 0;
        printf("找到网点信息, 注销成功\n");
        ret = 0;
        goto END;
    }
  }
  ret =  -1;
  printf("网点信息不存在\n");
END:
  if(addr != NULL)
    IPC_UnMapShm(addr);
    return ret;
}
