/**
* @file  MiniSip.h
* @brief ����MiniSip����ͷ�ļ�  
*
* Copyright (c) 2010
* Huawei Tech.Co.,Ltd
*
* @author   Li GengQiang/l00124479
* @date     2010/12/06
* @version  NVS V100R002C01
*
*/

#ifndef IVSCBB_SS_MINISIP_H
#define IVSCBB_SS_MINISIP_H

#define SS_MAINTAIN

#ifdef CUMW_SIP_NAMESPACE

//ȥ��map�澯
#pragma warning(disable : 4786 4503)
#endif

#include "ssincludeall.h"
#include "CConnMgr.h"
#include "sip_namespace.h"
//SIP��UDP��Socket
extern unsigned long g_ulSipUdpSocket;
namespace SipStack{

namespace SIP
{
    //SIP����ֵ����ʾ�ַ�����󳤶�
    const unsigned long STRING_LEN_SIP_RET_RESULT = sizeof("SIP_RET_ERR_MANDATORY_HDR");

    //SIP����ֵ����ʾ�ַ������鶨��
    const char STR_ARR_EN_SIP_RESULT[][STRING_LEN_SIP_RET_RESULT]
    = {
        "SIP_RET_SUCCESS",
        "SIP_RET_FAILURE",
        "SIP_RET_MALLOC_FAILED",
        "SIP_RET_INVALID_PARAM",
        "SIP_RET_INVALID_STATE",
        "SIP_RET_ERR_MANDATORY_HDR",    //STRING_LEN_SIP_RET_RESULTΪ���ַ����ĳ���
        "SIP_RET_ERR_BRANCH_CLASH",
        "SIP_RET_NOT_HEADER_LIST",
        "SIP_RET_RPORT_ERROR"
    };

    //SIP���󷽷����ַ�����󳤶�
    const unsigned long STRING_LEN_SIP_METHOD = sizeof("UNSUBSCRIBE");
    //SIP���󷽷����ַ������鶨��
    const char STR_ARR_SIP_METHOD[][STRING_LEN_SIP_METHOD]
    = {
        "EXT",
        "INVITE",
        "ACK",
        "OPTIONS",
        "BYE",
        "CANCEL",
        "REGISTER",
        "INFO",
        "PRACK",
        "SUBSCRIBE",
        "NOTIFY",
        "UPDATE",
        "MESSAGE",
        "UNSUBSCRIBE",
        "REFER",
        "PUBLISH"
    };

    typedef enum
    {
        EX_HDR_ID_MAX_FORWARDS  = SIP_BASIC_HDR_ID_BUTT,    //Max-Forwordsͷ��
        EX_HDR_ID_USER_AGENT,                               //User-Agentͷ��
        EX_HDR_ID_AUTHORIZATION,                            //Authorizationͷ��
        EX_HDR_ID_EXPIRES,                                  //Expiresͷ��
        EX_HDR_ID_CONTENT_TYPE,                             //Content-Typeͷ��
        EX_HDR_ID_WWW_AUTHENTICATE,                         //WWW-Authenticateͷ��
        EX_HDR_ID_AUTHENTICATION_INFO,                      //Authentication-Infoͷ��
        EX_HDR_ID_SUBJECT,                                  //Subjectͷ��
        EX_HDR_ID_REASON,                                   //
        EX_HDR_ID_DATE,                                     //
        EX_HDR_ID_SERVER,                                   //add by cwx148380
        EX_HDR_ID_MAX,                                      //��չͷ�����ֵ
    }EX_HDR_ID;

    const unsigned long MAX_NUM_EX_HEADER = EX_HDR_ID_MAX - EX_HDR_ID_MAX_FORWARDS;   //��չͷ�����
	
	typedef enum
	{
		INIT  = 0,                   // 
		OCCUPIED,            // ʹ����
	}REQ_MSG_STATE;

    //������Ϣ��Ϣ
    typedef struct
    {
        SipMsgS*    pstSipReqMsg;   //SIP������Ϣָ��
        SS_UINT32   ulTuObjId;      //��Ӧ��TU����ID
        SS_UINT32   ulTxnObjId;     //��Ӧ���������ID
        unsigned long ulDialogId;
        // added by dhong
        unsigned long secs;        // added by dhong
		REQ_MSG_STATE state;
    }REQ_MSG_INFO;
}

namespace SIP
{
    /**
    * Description:  CreateReqMsg(). ����SIP������Ϣ����������CSeqͷ��
    * @param  [in]  enSipMethod SIP�������� 
    * @param  [out] pstSipMsg   SIP��Ϣָ��
    * @return       long.       ������
    */
    long CreateReqMsg
    (
        EN_SIP_METHOD   enSipMethod, 
        SipMsgS*&       pstSipMsg
    );

    /**
    * Description:  InitMiniSip().  ��ʼ��MiniSip
    * @return       long.       ������
    */
    long InitMiniSip();

    /**
    * Description:  RegExHeaders(). ע����չͷ��
    * @return       long.       ������
    */
    long RegExHeaders();

    /**
    * Description:  SetSipTptIpPort().   ����SipTptIpPort
    * @param  [out] stTptIpPort �����õ�SipTptIpPortS����
    * @param  [in]  objAddr     ��·��ַ
    * @return       void.
    */
    void SetSipTptIpPort
    (
        SipTptIpPortS&      stTptIpPort, 
        const CNetworkAddr& objAddr
    );

    /**
    * Description:  SetSipTptNwInfo().  ����SipTptNwInfo
    * @param  [out] stTptNwInfo �����õ�SipTptNwInfoS����
    * @param  [in]  objDstAddr  Ŀ����·��ַ
    * @param  [in]  objSrcAddr  Դ��·��ַ
    * @return       long.       ������
    */
    void SetSipTptNwInfo
    (
        SipTptNwInfoS&      stTptNwInfo, 
        const CNetworkAddr& objDstAddr, 
        const CNetworkAddr& objSrcAddr
    );

    /**
    * Description:  CreateSipString().  ����SipString
    * @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
    * @param  [out] pstSipString    ��������SipString
    * @param  [in]  szContent       ����
    * @return       long.           ������
    */
    long CreateSipString
    (
        SipMemCpHdl     hdlMemCp, 
        VppStringS*&    pstSipString,
        const char*     szContent
    );

    /**
    * Description:  CreateQuoteString().  ���������ŵ�SipString
    * @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
    * @param  [out] pstSipString    ��������SipString
    * @param  [in]  szContent       ����
    * @return       long.           ������
    */
    long CreateQuoteString
    (
        SipMemCpHdl     hdlMemCp, 
        VppStringS*&    pstSipString,
        const char*     szContent
    );

    /**
    * Description:  CreateSipStringData().  ����SipString������
    * @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
    * @param  [out] stSipString     ��������SipString������
    * @param  [in]  szContent       ����
    * @return       long.           ������
    */
    long CreateSipStringData
    (
        SipMemCpHdl     hdlMemCp, 
        VppStringS&     stSipString, 
        const char*     szContent
    );

    /**
    * Description:  SetSipString(). ����SipString
    * @param  [in]  stSipString     ��������SipString
    * @param  [in]  szContent       ����
    * @return       void.
    */
    void SetSipString
    (
        SipString&  stSipString, 
        const char* szContent
    );

    /**
    * Description:  SetSipHostPort().   ����SipHostPort
    * @param  [out] stHostPort  �����õ�HostPort����
    * @param  [in]  objAddr     ��·��ַ
    * @return       void.
    */
    void SetSipHostPort
    (
        SipHostPort&        stHostPort, 
        const CNetworkAddr& objAddr
    );

    void SetSipHostPort_New
        (
        SipHostPort&         sip_host_port_st_r, 
        const string&   ip_str_r,
        const unsigned short port_us
        );

    /**
    * Description:  SetSipHostName().    ����HostName
    * @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
    * @param  [out] stHostPort      �����õ�HostPort����
    * @param  [in]  pszHostName     ������
    * @return       long.   ����ֵ
    */
    long SetSipHostName
    (
        SipMemCpHdl     hdlMemCp, 
        SipHostPort&    stHostPort, 
        const char*     pszHostName
    );

    /**
    * Description:  SetURI().   ����URI
    * @param  [in]  hdlMemCp    �ڴ���ƿ�ָ��
    * @param  [out] stURI       �����õ�URI����
    * @param  [in]  pszUriName  URI��
    * @return       long.       ������
    */
    long SetURI
    (
        SipMemCpHdl hdlMemCp, 
        URI&        stURI, 
        const char* pszUriName
    );

    /**
    * Description:  SetUriByDomain().   ��������URI
    * @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
    * @param  [out] stURI           �����õ�URI����
    * @param  [in]  pszUriName      URI��
    * @param  [in]  pszUriDomain    URI��
    * @return       long.       ������
    */
    long SetUriByDomain
    (
        SipMemCpHdl     hdlMemCp, 
        URI&            stURI, 
        const char*     pszUriName, 
        const char*     pszUriDomain
    );

    /**
    * Description:  SetUriByAddr(). ���õ�ַ��URI
    * @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
    * @param  [out] stURI           �����õ�URI����
    * @param  [in]  pszUriName      URI��
    * @param  [in]  objAddr         ��·��ַ
    * @return       long.       ������
    */
    long SetUriByAddr
    (
        SipMemCpHdl         hdlMemCp, 
        URI&                stURI, 
        const char*         pszUriName, 
        const CNetworkAddr& objAddr
    );

    /**
    * Description:  SetRequestUri().    ����Request-URI
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszUriName      URI��
    * @param  [in]  pszUriDomain    URI��
    * @return       long.       ������
    */
    long SetRequestUri
    (
        SipMsgS&        stSipMsg,
        const char*     pszUriName, 
        const char*     pszUriDomain
    );

    /**
    * Description:  SetCallId().  ����Call-IDͷ��
    * @param  [in]          stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in/out]  strCallId       CallID�ַ�����ֵ��Ϊ��ʱ���������
    * @return       long.       ������
    */
    long SetCallId
    (
        SipMsgS&    stSipMsg,
        string&     strCallId
    );

    /**
    * Description:  SetFrom().  ����Fromͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszUriName      URI��
    * @param  [in]  pszUriDomain    URI��
    * @return       long.       ������
    */
    long SetFrom
    (
        SipMsgS&        stSipMsg,
        const char*     pszUriName, 
        const char*     pszUriDomain
    );

    /**
    * Description:  SetTo().    ����Toͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszUriName      URI��
    * @param  [in]  pszUriDomain    URI��
    * @return       long.       ������
    */
    long SetTo
    (
        SipMsgS&        stSipMsg,
        const char*     pszUriName, 
        const char*     pszUriDomain
    );

    /**
    * Description:  SetToTag().    ����To��Tag
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @return       long.       ������
    */
    long SetToTag
    (
        SipMsgS& stSipMsg
    );

    /**
    * Description:  SetVia().   ����Viaͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  objAddr         ��·��ַ
    * @return       long.       ������
    */
    long SetVia
    (
        SipMsgS&             sip_msg_st_r,
        const string&   ip_str_r,
        const unsigned short port_us
    );


    /**
    * Description:  AddExHeader().  �����չͷ��
    * @param  [in]  stSipMsg    Sip��Ϣ�ṹ������
    * @param  [in]  usHdrId     ��չͷ��ID
    * @param  [in]  pvHdrVal    ��չͷ��ֵָ��
    * @return       long.       ������
    */
    long AddExHeader
    (
        SipMsgS&        stSipMsg,
        unsigned short  usHdrId,
        void*           pvHdrVal
    );

    /**
    * Description:  AddAuthorizationHeader().  �����չͷ��
    * @param  [in]  stSipMsg                    Sip��Ϣ�ṹ������
    * @param  [in]  stSipAuthorization  Sip��Ȩ��Ϣ
    * @return       long.       ������
    */
    long AddAuthorizationHeader
    (
        SipMsgS&            stSipMsg,
        SipAuthorization&   stSipAuthorization
    );

   /**
    * Description:  AddWWWAuthorizationHeader().  �����չͷ��
    * @param  [in]  stSipMsg                    Sip��Ϣ�ṹ������
    * @param  [in]  stSipAuthorization  Sip��Ȩ��Ϣ
    * @return       long.       ������
    */
    long AddWWWAuthorizationHeader
    (
        SipMsgS&            stSipMsg,
        SipWWWAuthenticate&   stSipAuthorization
    );

    /**
    * Description:  SetMaxForwords().   ����Max-Forwardsͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  nMaxForwards    Max-Forwardsֵ
    * @return       long.       ������
    */
    long SetMaxForwards
    (
        SipMsgS&    stSipMsg,
        SS_UINT32   nMaxForwards
    );

    /**
    * Description:  SetUserAgent(). ����User-Agentͷ��
    * @param  [in]  stSipMsg            Sip��Ϣ�ṹ������
    * @param  [in]  pszProductName      ��Ʒ����
    * @param  [in]  pszProductVersion   ��Ʒ�汾
    * @return       long.       ������
    */
    long SetUserAgent
    (
        SipMsgS&    stSipMsg,
        const char* pszProductName,
        const char* pszProductVersion
    );

    /**
    * Description:  SetRoute(). ����Routeͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  objRouteAddr    ·�ɵ�ַ
    * @param  [in]  pszDisplayName  ��ʾ����
    * @return       long.       ������
    */
    long SetRoute
    (
        SipMsgS&            stSipMsg,
        const CNetworkAddr& objRouteAddr,
        const char*         pszDisplayName
    );

    /**
    * Description:  SetAuthorization(). ����Authorizationͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszUserName     �û���
    * @param  [in]  pszPassword     ����
    * @param  [in]  stPlatAuthInfo  ƽ̨��Ȩ��Ϣ
    * @return       long.       ������
    */
    long SetAuthorization
    (
        SipMsgS&                stSipMsg,
        char*                   pszUserName,
        char*                   pszPassword,
        const PLAT_AUTH_INFO&   stPlatAuthInfo
    );

    /**
    * Description:  CreateAuthDigestUri().  ����Authorization��Digest��URI
    * @param  [in]      stReqUri        SipURI�ṹ������
    * @param  [out]     pszDigestUri    DigestUri�ַ������
    * @return       long.       ������
    */
    long CreateAuthDigestUri
    (
        const SipURI&   stReqUri,
        char*&          pszDigestUri
    );

    /**
    * Description:  SetContact(). ����Contactͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  objContactAddr  Contact��ַ
    * @param  [in]  pszDisplayName  ��ʾ����
    * @return       long.       ������
    */
    long SetContact
    (
        SipMsgS&            stSipMsg,
        const CNetworkAddr& objContactAddr,
        const char*         pszDisplayName
    );

    /**
    * Description:  SetExpires().   ����Expiresͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  nExpireTime     ��ʱʱ��
    * @return       long.       ������
    */
    long SetExpires
    (
        SipMsgS&    stSipMsg,
        SS_UINT32   nExpireTime
    );

    /**
    * Description:  SetDate(). ����Dateͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszDate          ʱ��
    * @return       long.       ������
    */
    long SetDate
    (
        SipMsgS&            stSipMsg,
        const char*         pszDate
    );

    long SetContentEncoding
        (
        SipMsgS&            stSipMsg,
        const char*         pszEncoding
        );

    /**
    * Description:  SetDate()       ����Dateͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszServer       �������
    * @return       long.           ������
    */
    long SetServer
    (
        SipMsgS&            stSipMsg,
        const char*         pszServer
    );

    /**
    * Description:  SetEvent(). �����Զ���Eventͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  pszEvent         �¼�
    * @return       long.       ������
    */
    long SetEvent
        (
        SipMsgS&         stSipMsg,
        const char*         pszEvent
        );

    /**
    * Description:  SetExpires().   ����Expiresͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  szSubMediaType  ��ý������
    * @return       long.       ������
    */
    long SetContentType
    (
        SipMsgS&    stSipMsg,
        const char* szSubMediaType
    );

    /**
    * Description:  SetSubject().   ����Subjectͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  strSubject      Subject�����ַ���
    * @return       long.       ������
    */
    long SetSubject
    (
        SipMsgS&        stSipMsg,
        const string&   strSubject
    );

    /**
    * Description:  ResetVarHdr().  ���ÿɱ�ͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  ulSequence      CSeqֵ
    * @param  [in]  pszFromTag      From��Tag
    * @param  [in]  pszToTag        To��Tag
    * @return       long.       ������
    */
    long ResetVarHdr
    (
        SipMsgS&        stSipMsg,   
        unsigned long   ulSequence,
        const std::string& strFromTag,
        const char*     pszToTag    = VOS_NULL
    );

    /**
    * Description:  SetMsgBody().   ������Ϣ��
    * @param  [in]  stSipMsg            ip��Ϣ�ṹ������
    * @param  [in]  szSubMediaType      ��ý������
    * @param  [in]  ulMsgBodyLen        ��Ϣ�峤��
    * @param  [in]  szMsgBody           ��Ϣ���ַ���
    * @return       long.       ������
    */
    long SetMsgBody
    (
        SipMsgS&        stSipMsg,
        const char*     szSubMediaType,
        unsigned long   ulMsgBodyLen,
        char*           szMsgBody
    );

    /**
    * Description:  GetPlatAuthInfo().   ��ȡƽ̨��Ȩ��Ϣ
    * @param  [in]  stSipMsg        SIP��Ϣ����
    * @param  [out] stPaltAuthInfo  ��ȡ��ƽ̨��Ȩ��Ϣ
    * @return       long.       ������
    */
    long GetPlatAuthInfo
    (
        SipMsgS&        stSipMsg,
        PLAT_AUTH_INFO& stPaltAuthInfo
    );

    /**
    * Description:  GetRedirectInfo().  ��ȡ�ض�����Ϣ
    * @param  [in]  stSipMsg            SIP��Ϣ����
    * @param  [out] vectorServerInfo    ��������Ϣ����
    * @return       long.       ������
    */
    long GetRedirectInfo
    (
        SipMsgS&            stSipMsg,
        VECTOR_SERVER_INFO& vectorServerInfo
    );

    /**
    * Description:  GetExpires().   ��ȡע�ᳬʱʱ��
    * @param  [in]  stSipMsg        SIP��Ϣ����
    * @param  [out] ulExpires       ��ȡ��ע�ᳬʱʱ��
    * @return       long.       ������
    */
    long GetExpires
    (
        SipMsgS&        stSipMsg,
        unsigned long&  ulExpires
    );

	/**
	* Description:  GetContact().   ��ȡ�豸�����ַ��Ϣ
	* @param  [in]  stSipMsg        SIP��Ϣ����
	* @param  [out] devIP              ��ȡ�豸IP
	* @param  [out] devPort          ��ȡ�豸Port
	* @return       long.       ������
	*/
	long GetContact
	(
	    SipMsgS&        stSipMsg,
	    string&          devIP,
	    unsigned short&  devPort
	);

   /**
    * Description:  GetDate().   ��ȡDateʱ��
    * @param  [in]  stSipMsg        SIP��Ϣ����
    * @param  [out] dateTime          ʱ��
    * @return       long.       ������
    */
    long GetDate
    (
        SipMsgS&        stSipMsg,
		string&       dateTime
    );

   /**
    * Description:  GetCurrentTime().   ��ȡϵͳ��ǰʱ��
    * @return       string.       ����ϵͳ��ǰʱ��
    */
	string GetCurrentTime();

    /**
    * Description:  GetNextNonce().     ��ȡNextNonce
    * @param  [in]  stSipMsg        SIP��Ϣ����
    * @param  [out] strNextNonce       ��ȡ��NextNonce
    * @return       long.       ������
    */
    long GetNextNonce
    (
        SipMsgS&    stSipMsg,
        string&     strNextNonce
    );

    /**
    * Description:  GetMsgBody().   ��ȡ��Ϣ������
    * @param  [in]  stSipMsg        SIP��Ϣ����
    * @param  [out] pMsgBody       ��Ϣ��ָ��
    * @param  [out] ulMsgBodyLen       ��Ϣ�����ݳ���
    * @return       long.       ������
    */
    long GetMsgBody
    (
        SipMsgS&        stSipMsg,
        char*&          pMsgBody,
        unsigned long&  ulMsgBodyLen
    );

    /**
    * Description:  GetSubject().   ��ȡSubjectͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  szSubject       Subject�����ַ�����������ַ���������
    * @param  [in]  ulSubjectLen    Subject�����ַ������ȣ������ַ���������
    * @return       long.       ������
    */
    long GetSubject
    (
        SipMsgS&        stSipMsg,
        char*&          pSubject,
        unsigned long&  ulSubjectLen
    );

    /**
    * Description:  GetReason().   ��ȡReasonͷ��
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [in]  ulReason        ulReason ����text�Ĵ�����
    * @return       long.       ������
    */
    long GetReason
    (
        SipMsgS&        stSipMsg,
        unsigned long&  ulReason
    );

    /**
    * Description:  CopyHeader().   ��SIP��Ϣ�п���ͷ��
    * @param  [in]  sHeaderId       ͷ��ID
    * @param  [in]  stInSipMsg      ������SIP��Ϣ����
    * @param  [out] stOutSipMsg     �����SIP��Ϣ����
    * @return       long.       ������
    */
    long CopyHeader
    (
        unsigned short  usHeaderId,
        SipMsgS&        stInSipMsg,
        SipMsgS&        stOutSipMsg
    );

    /**
    * Description:  GetDialogInfo().  ��ȡ�Ի���Ϣ
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [out]  stDiagInfo     ����ĶԻ���Ϣ
    * @param  [in]  bFromLocal      ��Ϣ�Ƿ����Ա���
    * @return       long.       ������
    */
    long GetDialogInfo
    (
        SipMsgS&        stSipMsg,
        DIALOG_INFO&    stDiagInfo,
        VOS_BOOLEAN     bFromLocal
    );

    /**
    * Description:  GetFromTag().  ��ȡFrom��Tag
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [out]  strFromTag     �����From��Tag
    * @return       long.       ������
    */
    long GetFromTag
    (
        SipMsgS&    stSipMsg,
        string&     strFromTag
    );

    /**
    * Description:  GetFromUriInfo().  ��ȡFrom��Uri��Ϣ
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [out]  stFromUri     �����From��Uri��Ϣ
    * @return       long.       ������
    */
    long GetFromUriInfo
    (
        SipMsgS&        stSipMsg,
        PEER_URI_INFO&  stFromUri
    );

    /**
    * Description:  GetFromValue().  ��ȡFrom��ֵ
    * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
    * @param  [out]  pszFromValue   �����From��Value������������ڴ�
    * @return       long.       ������
    */
    /*
    long GetFromValue
    (
        SipMsgS&    stSipMsg,
        char*&      pszFromValue
    );
    */
}
}//end namespace
#endif //_MINI_SIP_H_HEADER_INCLUDED_

