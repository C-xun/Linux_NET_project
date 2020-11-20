#ifndef GET_INTERFACE_H
#define GET_INTERFACE_H

#define MAXINTERFACES 16    /* ���ӿ��� */

typedef struct interface{
	char name[20];		//�ӿ�����
	unsigned char ip[4];		//IP��ַ
	unsigned char mac[6];		//MAC��ַ
	unsigned char netmask[4];	//��������
	unsigned char br_ip[4];		//�㲥��ַ
	int  flag;			//״̬
}INTERFACE;
extern INTERFACE net_interface[MAXINTERFACES];//�ӿ�����

/******************************************************************
��	��:	int getinterface()
��	��:	��ȡ�ӿ���Ϣ
��	��:	��
*******************************************************************/
extern void getinterface();

/******************************************************************
��	��:	int get_interface_num()
��	��:	��ȡʵ�ʽӿ�����
��	��:	�ӿ�����
*******************************************************************/
int get_interface_num();


#endif
