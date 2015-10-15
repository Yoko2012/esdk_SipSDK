#ifndef _DATATYPE_H_
#define _DATATYPE_H_

//��󳤶�
const int MAX_LENGTH = 1024;

//�������ؽ��
typedef enum _EM_SIP_RESULT
{
	SIPSDK_RET_SUCCESS				=	0,	//�ɹ�
	SIPSDK_RET_FAILURE				=	1,	//ʧ��
	SIPSDK_RET_NOT_INIT			=	2,	//δ��ʼ��
	SIPSDK_RET_HAS_INIT			=	3,	//�Ѿ���ʼ��
	SIPSDK_RET_NOT_REGISTER		=	4,	//δע��
	SIPSDK_RET_INVALID_PARAM		=	5	//��������
}EM_SIP_RESULT;

//�յ�����Ϣ����
typedef enum _EM_SIP_MSG_TYPE
{
	SIP_REQUEST_MSG		=	0,	//�յ���������Ϣ
	SIP_RESPONSE_MSG	=	1	//�յ�����Ӧ��Ϣ
}EM_SIP_MSG_TYPE;

//������Ϣ������
typedef enum _EM_SIP_REQUEST_EVENT
{
	SIP_REQ_REGISTER		=	0,	//ע�������¼�
	SIP_REQ_INVITE			=	1,	//���������¼�
	SIP_REQ_MESSAGE			=	2	//��Ϣ�����¼�
}EM_SIP_REQUEST_EVENT;

//��Ӧ��Ϣ������
typedef enum _EM_SIP_RESPONSE_EVENT
{
	SIP_RSP_SUCCESSS		=	200,	//��Ӧ�ɹ��¼�
	SIP_RSP_NOT_REG			=	403		//ע��ʧ����Ӧ�¼�
}EM_SIP_RESPONSE_EVENT;

//��Ϣ֪ͨ�ص��������
typedef int (_stdcall *SIP_CallBack)(int msgType,int eventType,const char* pPara);
#endif 