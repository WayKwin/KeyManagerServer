#include <unistd.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <pthread.h>
#include "../incl/keymnglog.h"

#include "../incl/poolsocket.h"
#include "../incl/keymngclientop.h"
#include "../incl/keymng_msg.h"
#include "../incl/crypto.h"
int usage()
{
  int nSel = -1;
    system("clear");    
    printf("\n  /*************************************************************/");
    printf("\n  /*************************************************************/");
    printf("\n  /*     1.密钥协商                                            */");
    printf("\n  /*     2.密钥校验                                            */");
    printf("\n  /*     3.密钥注销                                            */");
    printf("\n  /*     4.密钥查看                                            */");
    printf("\n  /*     0.退出系统                                            */");
    printf("\n  /*                                                           */");
    printf("\n  /*************************************************************/");
    printf("\n  /*************************************************************/");
    printf("\n\n  选择:");
    scanf("%d", &nSel);
    while(getchar() != '\n'); //把应用程序io缓冲器的所有的数据 都读走,避免影响下一次 输入
    return nSel;
}
int main(int argc,char* argv[] )
{
 //读配置文件, clientid authocode,serverid serverip serverprot clientshmkey
 int ret = 0;
 MngClient_Info MC_info;
 memset(&MC_info,0,sizeof(MngClient_Info));

 google::InitGoogleLogging(argv[0]);
 FLAGS_log_dir = "./log";

 //初始化
  ret  = MngClient_InitInfo(&MC_info);
  while(1)
  {
    int nSel = 0;
    nSel = usage();
    switch(nSel)
    {
      case 0:
        // exit
        return nSel;
      case 1:
        //密钥协商
        ret = MngClient_Agree(&MC_info);
        printf("密钥协商\n");
        break;
      case 2:
        printf("密钥校验\n");
        ret = MngClient_Check(&MC_info);
        break;
      case 3:
        printf("密钥注销\n");
        ret = MngClient_Revoke(&MC_info);
        break;
      case 4:
        printf("密钥查看\n");
        break;
   }
    if(ret)
    {
      printf("\n !!!!error\n");
    }
    else 
    {
      printf("\n !!!!!! SUCESS\n");
    }
    getchar();
  }

}


