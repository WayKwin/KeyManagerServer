

// keymng_shmop.h

#ifndef _KEYMNG_SHMOP_H_
#define _KEYMNG_SHMOP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus 
extern "C" {
#endif


//��������Կ��Ϣд�����ڴ棬 ���㹲���ڴ�ṹ��
typedef struct _NodeSHMInfo
{	
	int 			status;			//��Կ״̬ 0-��Ч 1��Ч
	char			clientId[12];	//�ͻ���id
	char			serverId[12];	//��������id	
	int				seckeyid;		//�Գ���Կid
	unsigned char	seckey[128];	//�Գ���Կ //hash1 hash256 md5
}NodeSHMInfo;


//int KeyMng_ShmInit(int keyid, int keysize, void *shmid )
//�򿪹����ڴ� �����ڴ������ʹ�� �������򴴽�
int KeyMng_ShmInit(int key, int maxnodenum, int *shmhdl);

	
/*
1 д��������Կʱ,���ж�������Կ�Ƿ����
	���������޸�
:w
*/

int KeyMng_ShmWrite(int shmhdl, int maxnodenum, NodeSHMInfo *pNodeInfo);



int KeyMng_ShmRead(int shmhdl, char *clientId, char *serverId,  int maxnodenum, NodeSHMInfo *pNodeInfo);


#ifdef __cplusplus
}
#endif
#endif





