
// keymngserverop.h
#ifndef _KEYMNG_ServerOp_H_
#define _KEYMNG_ServerOp_H_

#include "keymng_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

//keymngserver ������

#define		MngSvr_OK				0		//��ȷ
#define		MngSvr_ParamErr			301		//�������ʧ��
#define		MngSvr_NoNetPointErr	302		//�����ڴ��У�û���ҵ�������Ϣ
#define		MngSvr_NodeMaxCount		303		//�����ڴ��У������������
#define		MngSvr_CheckErr			304		//�����ڴ��У������������

typedef struct _MngServer_Info
{
	char			serverId[12];	//�������˱��
	
	//���ݿ����ӳؾ��	
	char			dbuse[24]; //���ݿ��û���
	char			dbpasswd[24]; //���ݿ�����
	char			dbsid[24]; //���ݿ�sid
	int				dbpoolnum; //���ݿ�� ������
	//�ź������
	
	char			serverip[24];
	int 			serverport;
	
	//�����ڴ�������Ϣ
	int				maxnode; //��������� �ͻ���Ĭ��1��
	int 			shmkey;	 //�����ڴ�keyid ���������ڴ�ʱʹ��	 
	int 			shmhdl; //�����ڴ���	
	
}MngServer_Info;



//��ʼ�������� ȫ�ֱ���
int MngServer_InitInfo(MngServer_Info *svrInfo);


int MngServer_Quit(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen);

//����� ��ԿЭ��Ӧ������
int MngServer_Agree(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen);
   

int MngServer_Check(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen);

int MngServer_Revoke(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen);

int MngServer_view(MngServer_Info *svrInfo, MsgKey_Req *msgkeyReq, unsigned char **outData, int *datalen);




#ifdef __cplusplus
}
#endif

#endif

