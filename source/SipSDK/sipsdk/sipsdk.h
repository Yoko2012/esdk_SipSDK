//lint -e1784
#ifndef _ESDK_SIPSDK_H
#define _ESDK_SIPSDK_H

// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� ESDK_UCSERVICE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// ESDK_UCSERVICE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#ifdef SIPSDK_EXPORTS
#define ESDK_SIPSDK_API __declspec(dllexport)
#else
#define ESDK_SIPSDK_API __declspec(dllimport)
#endif

#ifdef WIN32
#define __SIP_SDK_CALL __stdcall
#else
#define __SIP_SDK_CALL
#endif

#include "DataType.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 *��ʼ��
	 * 
	 *�ú������ڳ�ʼ��SIPЭ��ջ
	 *	 
	 *@param[in]	pLocalSipAccount	����sip��Ȩ�û���	 
	 *@param[in]	pLocalSipPasswd		����sip��Ȩ����
	 *@param[in]	pLocalID		����ID	
	 *@param[in]	pLocalIP		����IP
	 *@param[in]	iLocalPort		���ض˿�
	 *@param[in]	pServerSipAccount	�Զ�sip��Ȩ�û���	 
	 *@param[in]	pServerSipPasswd	�Զ�sip��Ȩ����
	 *@param[in]	pServerID		�Է�ID
	 *@param[in]	pServerIP		�Է�IP
	 *@param[in]	iServerPort		�Է��˿�
	 *@param[in]	pCallBackFunc	�ص�����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	���ȿ�ʼ���õĺ���,pSipAccount��pSipPasswd����Ϊ���ַ���������������Ϊ��
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_Init(const char* pLocalSipAccount
		,const char* pLocalSipPasswd
		,const char* pLocalID
		,const char* pLocalIP
		,int iLocalPort
		,const char* pServerSipAccount
		,const char* pServerSipPasswd
		,const char* pServerID
		,const char* pServerIP
		,int iServerPort
		,SIP_CallBack pCallBackFunc);

	/**
	 *ȥ��ʼ��
	 * 
	 *�ú�������ȥ��ʼ��SIPЭ��ջ
	 *
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_UnInit(void);

	/**
	 *SIPע��
	 * 
	 *�ú������ڷ���SIP Register��Ϣ
	 *
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_REGISTER(void);

	/**
	 *SIPעע��
	 * 
	 *�ú������ڷ���SIP Register��Ϣ����Ч��Ϊ0
	 *
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_UNREGISTER(void);

	/**
	 *SIP Invite
	 * 
	 *�ú������ڷ���SIP Invite��Ϣ
	 *
	 *@param[in]	pDevCode		�豸���룬�������������04598710001311861862
	 *@param[in]	pLocalSdpBody	����SDP�����ַ���
	 *@param[out]	iResponseID		��ӦID
	 *@param[out]	strRemoteBody	�Է�SDP����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_INVITE(const char* pDevCode
		,const char* pLocalSdpBody
		,int& iResponseID
		,char strRemoteBody[MAX_LENGTH]);

	/**
	 *SIP Message
	 * 
	 *@param[in]	pDevCode		�豸���룬�������������04598710001311861862
	 *@param[in]	pBody			���͵�xml�ֶ�
	 *�ú������ڷ���SIP Message��Ϣ
	 *
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_MESSAGE(const char* pDevCode,const char* pBody);

	/**
	 *SIP BYE
	 * 
	 *�ú������ڷ���SIP BYE��Ϣ
	 *
	 *@param[in]	iResponseID		Invite�ɹ������ӦID
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��������Ҫ����SIP_SDK_INVITE���ص�ֵ
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_BYE(int iResponseID);

	/**
	 *SIP ACK
	 * 
	 *�ú������ڷ���SIP ACK��Ϣ
	 *
	 *@param[in]	iResponseID		Invite�ɹ������ӦID
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��������Ҫ����SIP_SDK_INVITE���ص�ֵ
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_ACK(int iResponseID);

	/**
	 *����
	 * 
	 *�ú������ڷ���subscribe��Ϣ
	 *
	 *@param[in]	pRemoteURI		���Ķ���URI
	 *@param[in]	pBody			body��
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	���ȿ�ʼ���õĺ���
	 *@par			��
	**/
	ESDK_SIPSDK_API int __SIP_SDK_CALL SIP_SDK_Subscribe(const char* pRemoteURI,const char* pBody);



#ifdef __cplusplus
}
#endif 


#endif

