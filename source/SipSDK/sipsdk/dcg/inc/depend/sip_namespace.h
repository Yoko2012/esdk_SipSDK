/**
* @file  sip_namespace.h
* @brief SIP�����ռ䶨��  
*
* Copyright (c) 2010
* Huawei Tech.Co.,Ltd
*
* @author   Li GengQiang/l00124479
* @date     2010/12/01
* @version  NVS V100R002C01
*
*/

#ifndef IVSCBB_SS_SIP_NAME_SPACE_H
#define IVSCBB_SS_SIP_NAME_SPACE_H

#ifndef BOOL
#define BOOL  bool
#endif

#ifdef WIN32
    //ȥ��map�澯
#define STDCALL __stdcall
    #pragma warning(disable : 4786 4503)

    #pragma comment(lib, "coresip.lib")
    #pragma comment(lib, "sipext.lib")
    #pragma comment(lib, "sipsdp.lib")
    #pragma comment(lib, "cbb.lib")
#else

#define STDCALL
#define SOCKET_ERROR (-1)

#endif

//��׼ͷ�ļ�
#include <string>
#include <map>
#include <vector>
#include <list>
using namespace std;

//VOSͷ�ļ�
#include "vos.h"
#include "vos_common.h"
#include "CConnMgr.h"
#include "CSyncQueue.h"
#include "CNVSTimer.h"
#include "MiniSip.h"

//������ӹ�����ʵ������
#define CONN_MGR CConnMgr::instance()

//���ȫ�ֶ�ʱ��ʵ������
#define NVS_TIMER SipStack::CNVSTimer::instance()

#pragma pack(push,1)

namespace SipStack{
//��������
namespace SIP
{
const unsigned long MAX_NUM_AUTH_HEADER     = 1; 

const unsigned long MAX_NUM_TRANSACTIONS    = 3000; //���������
const unsigned long MAX_FORWARDS            = 70;   //SIP��Ϣ�����ת����

const unsigned long MAX_LEN_UDP_PACKET      = 1024 * 8; //UDP������󳤶�

const unsigned long MAX_LEN_LOGIN_DOMAIN    = 128 + 4;  //��¼�����󳤶�
const unsigned long MAX_LEN_LOGIN_NAME      = 32  + 4;  //��¼������󳤶�
const unsigned long MAX_LEN_LOGIN_PWD       = 64  + 4;  //��¼�������󳤶�
const unsigned long MAX_LEN_PRODUCT_NAME    = 128 + 4;  //��Ʒ���Ƶ���󳤶�
const unsigned long MAX_LEN_PRODUCT_VERSION = 64  + 4;  //��Ʒ�汾����󳤶�
const unsigned long MAX_LEN_CALL_ID         = 64  + 4;  //Call-ID����󳤶�
const unsigned long MAX_LEN_REQUEST_URI     = 256;      //Request-URI����󳤶�
const unsigned long MAX_LEN_SUBJECT         = 256;      //Subjectͷ�����󳤶�
const unsigned long MAX_LEN_OPAQUE          = 256;      //Opaque����󳤶�

const unsigned long MD5_HEX_LEN             = 32  + 4;  //MD5ֵ�ĳ���

const unsigned long MAX_SIP_SERVER_ID       = 32;   //SIP������ID����󳤶� 

const unsigned long MAX_LEN_IP              = 16;   //IP����󳤶�
const unsigned long MAX_LEN_PORT            = 5;
const unsigned long MAX_LEN_FILE_PATH       = 256;  //�ļ�·����󳤶�

const unsigned long REGISTER_EXPIRES_TIME   = 365 * 24 * 3600; //ע�ᳬʱʱ��: һ��

const char SIP_URI_SCHEME[]             = "sip:";   //SIP��URI�ķ����ַ���
const char MEDIA_TYPE_APPLICATION[]     = "application";    //ý������
const char SUB_MEDIA_TYPE_SDP[]         = "sdp";    //SDP��ý������
const char SUB_MEDIA_TYPE_XML[]         = "cv+xml"; //XML��ý������
const char SUB_MEDIA_TYPE_T28181_SUBSCRIBE_XML[]         = "MANSCDP+xml"; //XML��ý������
const char SUB_MEDIA_TYPE_T28181_XML[]  = "xml";

const unsigned long MAX_LEN_AUTH_STRING = 8 + 1;    //wwwժҪ��Ȩ��qop
const unsigned long MAX_NUM_AUTH_INFO_CHANGE    = 3;    //ͬһ�μ�Ȩ��Ϣ���������

const unsigned long TIMER_INTERVAL          = 1;    //100����
const unsigned long EVENT_SYNC_QUEUE_NUM    = 1024; //�¼�ͬ��������

const unsigned long TIMER_USER_RIGHT_SHIFT  = 28;   //��ʱ���������û���Ϣ����λ��

const unsigned long INVALID_DIALOG_ID = 0;  //��Ч��ҵ��ID
}

//ö�����Ͷ���
namespace SIP
{
//������
enum RET_CODE
{
    //ͨ�ô�����
    RET_CODE_OK = 0x0000,           //�ɹ�
    RET_CODE_FAIL,                  //ϵͳ����
    RET_CODE_PARA_INVALIDATE,       //��Ч����
    RET_CODE_ALLOC_MEM_FAIL,        //�����ڴ�ʧ��
    RET_CODE_MEMORY_NULL,           //ָ��Ϊ��
    RET_CODE_INIT_SYS_ERROR,        //��ʼ��ʱϵͳ����
    RET_CODE_AUTHENTICATE_ERROR,    //��Ȩʧ��
    RET_CODE_CREATE_SIP_MSG_FAIL,   //����SIP��Ϣʧ��
    RET_CODE_SEND_SIP_MSG_FAIL,     //����SIP��Ϣʧ��
    RET_CODE_REQUEST_TIME_OUT,      //����ʱ
    RET_CODE_NO_TO_TAG,             //û��TO��Tag
    RET_CODE_NO_REGISTER,           //û��ע�ᣬ����ע��
    RET_CODE_REGISTER_UNAUTH,       // ....401..
    RET_CODE_ERROR_ONLINE,          // ..............
};

//�Զ����SIP״̬��
enum SIP_REASON_CODE
{
    SIP_REASON_CODE_NO_REGISTER = 1101,  //û��ע�ᣬ����ע��
};

//�¼�����
enum EVENT_TYPE
{
    EVENT_TYPE_EXIT_THREAD,             ///<�˳��߳�
        //��Ӧ��pEventInfoΪ: NULL

    EVENT_TYPE_STACK_TIMER_REGISTER,    ///<��ʱ��ע���¼�
        //��Ӧ��pEventInfoΪ: EVENT_INFO_TIMER_REGISTER
    EVENT_TYPE_STACK_TIMER_CANCEL,      ///<��ʱ��ȡ���¼�
        //��Ӧ��pEventInfoΪ: EVENT_INFO_TIMER_CANCEL
    EVENT_TYPE_STACK_TIMER_TRIGGER,     ///<��ʱ�������¼�
        //��Ӧ��pEventInfoΪ: NULL
    EVENT_TYPE_STACK_REDIRECT,          ///<�ض����¼�
        //��Ӧ��pEventInfoΪ: SIP_MSG_INFO

    EVENT_TYPE_STACK_INVALID,   //��Ч�¼�
};

//�¼����͵���ʾ�ַ�����󳤶�
const unsigned long STRING_LEN_EVENT_TYPE = sizeof("Event Type is Timer Register");
//�¼����͵���ʾ�ַ������鶨��
const char STR_ARR_EVENT_TYPE[][STRING_LEN_EVENT_TYPE] =
{
    "exit thread event",
    "registe timer event", //STRING_LEN_EVENT_TYPEΪ���ַ����ĳ���
    "cancel timer event",
    "trigger timer event",
    "redirect event",
    "invalid event",
};

const unsigned int MAX_STR_ARR_EVENT_TYPE_COUNT = 5;

//�ı���Ϣ����
typedef enum
{
    //����λ����
    TEXT_MSG_TYPE_MASK_MASK_BIT     = 0xF000,   ///<����λ������
    TEXT_MSG_TYPE_MASK_SIP          = 0x1000,   ///<SIP����
    TEXT_MSG_TYPE_MASK_HTTP         = 0x2000,   ///<HTTP����

    //SIP�ı���Ϣ����
    TEXT_MSG_TYPE_SIP_MIN           = 0x0000 | TEXT_MSG_TYPE_MASK_SIP,  //SIP��Ϣ������Сֵ
    TEXT_MSG_TYPE_SIP_REGISTER      = 0x0001 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP REGISTER
        //��Ӧ��pMsgHeaderΪ: RESP_HEADER_REGISTER
    TEXT_MSG_TYPE_SIP_UNREGISTER    = 0x0002 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP REGISTER ����ע��
    TEXT_MSG_TYPE_SIP_OPTIONS       = 0x0003 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP OPTIONS
    TEXT_MSG_TYPE_SIP_MESSAGE       = 0x0004 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP MESSAGE
        //������Ϣ�У���Ӧ��pMsgHeaderΪ: Request-URI��Name���֣����ַ���
        //������Ϣ�У���Ӧ��ulMsgHeaderLenΪ: Request-URI��Name���֣�����������
    TEXT_MSG_TYPE_SIP_INVITE        = 0x0005 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP INVITE
        //������Ϣ�У���Ӧ��pMsgHeaderΪ: @ref SIP::REQ_HEADER_INVITE
        //������Ϣ�У���Ӧ��ulMsgHeaderLenΪ: @SIP::REQ_HEADER_INVITE�ĳ���
        //������Ϣ�У���Ӧ��pMsgHeaderΪ: Subject�ַ������ȣ�����������
        //������Ϣ�У���Ӧ��ulMsgHeaderLenΪ: Subject�ַ������ȣ�����������
    TEXT_MSG_TYPE_SIP_BYE           = 0x0006 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP BYE
    TEXT_MSG_TYPE_SIP_CANCEL        = 0x0007 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP CANCEL
    TEXT_MSG_TYPE_SIP_ACK           = 0x0008 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP ACK

    TEXT_MSG_TYPE_SIP_SUBSCRIBE   = 0x0009 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP SUBSCRIBE
    TEXT_MSG_TYPE_SIP_UNSUBSCRIBE   = 0x0010 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP UNSUBSCRIBE
    TEXT_MSG_TYPE_SIP_INVITE_RSP   = 0x0011 | TEXT_MSG_TYPE_MASK_SIP,  ///<SIP 

    TEXT_MSG_TYPE_SIP_MAX,                                              //SIP��Ϣ�������ֵ

    //HTTP�ı���Ϣ����
    TEXT_MSG_TYPE_HTTP_POST         = 0x0001 | TEXT_MSG_TYPE_MASK_HTTP, ///<HTTP POST
}TEXT_MSG_TYPE;

//�ı����͵�SIP�������͵�ת��
const EN_SIP_METHOD ARR_TEXT_TYPE_TO_SIP_METHOD[] =
{
    SIP_METHOD_EXT,
    SIP_METHOD_REGISTER,
    SIP_METHOD_REGISTER,
    SIP_METHOD_OPTIONS,
    SIP_METHOD_MESSAGE,
    SIP_METHOD_INVITE,
    SIP_METHOD_BYE,
    SIP_METHOD_CANCEL,
    SIP_METHOD_ACK,

    SIP_METHOD_SUBSCRIBE,
    SIP_METHOD_UNSUBSCRIBE,
};

//SIP�������͵��ı�����ת��
const TEXT_MSG_TYPE ARR_SIP_METHOD_TO_TEXT_TYPE[] =
{
    TEXT_MSG_TYPE_SIP_MIN,
    TEXT_MSG_TYPE_SIP_INVITE,
    TEXT_MSG_TYPE_SIP_ACK,
    TEXT_MSG_TYPE_SIP_OPTIONS,
    TEXT_MSG_TYPE_SIP_BYE,
    TEXT_MSG_TYPE_SIP_CANCEL,
    TEXT_MSG_TYPE_SIP_REGISTER,
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_INFO
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_PRACK
    TEXT_MSG_TYPE_SIP_SUBSCRIBE,      //SIP_METHOD_SUBSCRIBE
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_NOTIFY
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_UPDATE
    TEXT_MSG_TYPE_SIP_MESSAGE,         
    TEXT_MSG_TYPE_SIP_UNSUBSCRIBE,      //SIP_METHOD_UNSUBSCRIBE
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_REFER
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_PUBLISH
    TEXT_MSG_TYPE_SIP_MAX,      //SIP_METHOD_BUTT
};
}

//�ṹ�嶨��
namespace SIP
{
//��ʱ����Ϣ
typedef struct 
{
    unsigned long ulTimerId;        //��ʱ��ID
    unsigned long ulTimerLength;    //��ʱ��ʱ��
    unsigned long ulTimerPara;      //��ʱ������
    unsigned long ulStartTimeTick;  //��ʼʱ���
        //BEGIN R002C01SPC100 ADD 2011-08-15 ligengqiang 00124479 for ��ʱ��SipTxnTimeoutHandler��������⣬��ֹ�ظ�ֹͣ��ʱ��
    BOOL          bStop;            //�Ƿ���ֹͣ
        //END   R002C01SPC100 ADD 2011-08-15 ligengqiang 00124479 for ��ʱ��SipTxnTimeoutHandler��������⣬��ֹ�ظ�ֹͣ��ʱ��
}TIMER_INFO;

//��ʱ��ע���¼���Ϣ
    //��Ӧ��ulEventTypeΪ: EVENT_TYPE_STACK_TIMER_REGISTER
typedef TIMER_INFO EVENT_INFO_TIMER_REGISTER;

//��ʱ��ȡ���¼���Ϣ
    //��Ӧ��ulEventTypeΪ: EVENT_INFO_TIMER_CANCEL
typedef struct
{
    unsigned long ulTimerId;    //��ʱ��ID
}EVENT_INFO_TIMER_CANCEL;

//�¼���Ϣ
typedef struct _EVENT_INFO
{
    EVENT_TYPE      enEventType;        //�¼�����
    void*           pEventInfo;         //�¼���Ϣָ�룬������ȡ����ulEventType
    unsigned long   ulEventInfoLen;     //�¼���Ϣ���ݳ��ȣ�У����

    //���캯��
    _EVENT_INFO(EVENT_TYPE enType = EVENT_TYPE_STACK_INVALID, void* pInfo = NULL, unsigned long ulInfoLen = 0)
        {
            enEventType     = enType;
            pEventInfo      = pInfo;
            ulEventInfoLen  = ulInfoLen;
        }
}EVENT_INFO;

//�¼�ͬ������
typedef CSyncQueue<EVENT_INFO> CEventSyncQueue;

//��������Ϣ
typedef struct _SERVER_INFO
{
    char            szServerIP[MAX_LEN_IP+1]; //������IP
    unsigned short  usServerPort;           //�������˿�
    unsigned short  usReserve;              //�Ƿ�ʹ�ù�

    _SERVER_INFO()
        {
            memset(szServerIP, 0, sizeof(szServerIP));
            usServerPort    = 0;
            usReserve       = 0;
        }
}SERVER_INFO;

//��������Ϣ��������
typedef vector<SERVER_INFO> VECTOR_SERVER_INFO;

//ע����Ϣ
typedef struct
{
    char            szLoginDomain[MAX_LEN_LOGIN_DOMAIN];        //��¼��
    char            szLoginName[MAX_LEN_LOGIN_NAME];            //��¼��
    char            szPassword[MAX_LEN_LOGIN_PWD];              //��¼����
    char            szLocalIp[MAX_LEN_IP];                      //����IP
    char            szLocalPort[MAX_LEN_PORT];                  //���ض˿�
    char            szUriDomain[MAX_LEN_LOGIN_DOMAIN];          //����ͨ��ʱ���UriDomain��
    char            szSipServerId[MAX_SIP_SERVER_ID];           //SIP������ID
	char            szRemoteLoginName[MAX_LEN_LOGIN_NAME];      //Զ��SIP��¼��
	char            szRemotePassword[MAX_LEN_LOGIN_PWD];        //Զ��SIP��¼����
	char            szLocalId[MAX_LEN_LOGIN_NAME];				//����ID
    char            szProductName[MAX_LEN_PRODUCT_NAME];        //��Ʒ����
    char            szProductVersion[MAX_LEN_PRODUCT_VERSION];  //��Ʒ�汾
    unsigned long   ulExpireTime;                               //��ʱʱ�䳤��
    unsigned long   ulServerInfoNum;                            //��������Ϣ����
    SERVER_INFO     stServerInfoArray[1];                       //��������Ϣ�ɱ����飬��Ԫ�ظ���ΪulServerInfoNum
}REGISTER_INFO;


//�ı���Ϣ
typedef struct
{
    unsigned long   ulMsgSeq;       //�ı���Ϣ���
    TEXT_MSG_TYPE   enTextMsgType;  //�ı���Ϣ����
    unsigned long   ulDialogId;     //Invite�����ĶԻ�Id��Invite��200 OK�ﷵ�ء�BYEʱ��Я��
    unsigned long   ulMsgBodyLen;   //�ı���Ϣ�峤��
    char*           pszMsgBody;     //�ı���Ϣ��
    void*           pMsgHeader;     //��Ϣͷ��Ľṹ��ָ�룬������ȡ����enMsgType
    unsigned long   ulMsgHeaderLen; //��Ϣͷ�򳤶�
    void*           pUserData;      //�û����ݣ�ԭֵ����
}TEXT_MSG;

//ע����Ӧ��Ϣ��ͷ��
typedef struct
{
    unsigned long   ulExpires;                  //��ʱʱ��
    VOS_BOOLEAN     bRedirect;                  //�Ƿ��ض���
    char            szOpaque[MAX_LEN_OPAQUE];   //ע��ɹ��ļ�Ȩ��ս��Ϣ�е�Opaque
    unsigned long   ulServerInfoNum;            //��������Ϣ����
    SERVER_INFO     stServerInfoArray[1];       //��������Ϣ�ɱ����飬��Ԫ�ظ���ΪulServerInfoNum
}RESP_HEADER_REGISTER;

///////////////////////////////////////////////////////////////////////////
    /// @brief INVITE������Ϣ��ͷ��
    ///
    /// ��Ӧ��enTextMsgTypeΪ: @ref SIP::TEXT_MSG_TYPE_SIP_INVITE
    ///////////////////////////////////////////////////////////////////////////
struct REQ_HEADER_INVITE
{
    char szRequestUri[MAX_LEN_REQUEST_URI];  //Request-URI�����ݲ�ͬҵ�񣬿������豸ID��������ID
    char szSubject[MAX_LEN_SUBJECT];         //Subjectͷ���ֵ
};

///////////////////////////////////////////////////////////////////////////
    /// @brief  �Զ˵�URI��Ϣ����������Ϣ�е�Fromͷ����ȡ
    ///
    /// ��Ӧ��enTextMsgTypeΪ: @ref SIP::TEXT_MSG_TYPE_SIP_MESSAGE
    ///////////////////////////////////////////////////////////////////////////
struct PEER_URI_INFO
{
    char szUriUserName[MAX_LEN_REQUEST_URI];    //URI�����ֲ���
    char szUriHostName[MAX_LEN_REQUEST_URI];    //URI����������
    char szUserAgentName[MAX_LEN_REQUEST_URI];  //userAgent����������
    char szCallID[MAX_LEN_REQUEST_URI];			//userAgent����������
};

//ƽ̨��Ȩ��Ϣ
typedef struct _PLAT_AUTH_INFO
{
    string strRealm;    //�û���
    string strNonce;    //Nonce
    string strOpaque;   //Opaque
    string stAlgorithm; //Algorithm
	string strQop;		//Qop="auth","auth-sess"
}PLAT_AUTH_INFO;

//�Ի���Ϣ
typedef struct _DIALOG_INFO
{
    unsigned long   ulCSeq;               // Invite��CSeq��ACK��Cancel��ʹ��
    VOS_BOOLEAN     bRecvOK;              //�Ƿ��Ѿ��յ�OK
    string          strLocalTag;
    string          strRemoteTag;
    string          strCallId;            //����Inviteʱ��Ч
    string          strRequestUriName;    //Invite RequestUri
    string          strRequestUriDomain;  //Invite RequestDomain

    _DIALOG_INFO()
        {
            ulCSeq  = 0;
            bRecvOK = VOS_FALSE;
        }

    bool operator < ( _DIALOG_INFO& other )
        {
            return (strCallId < other.strCallId);
        }

}DIALOG_INFO;
}

//�ص���������
namespace SIP
{
//֪ͨ���Ͷ���
typedef enum
{
    NOTIFY_TYPE_INTERFACE_MIN   = 0x8000,   //SIP������Сֵ

    NOTIFY_TYPE_TEXT_MSG_RESP   = 0x8001,   //�ı���Ϣ��Ӧ֪ͨ
    //��ӦpNotifyInfo�ṹ��Ϊ: NOTIFY_TEXT_MSG_RESP
    NOTIFY_TYPE_TEXT_MSG_REQ    = 0x8002,   //�ı���Ϣ����֪ͨ
    //��ӦpNotifyInfo�ṹ��Ϊ: NOTIFY_TEXT_MSG_REQ

    NOTIFY_TYPE_SIP_MAX         = 0x9000,   ///<SIP�������ֵ
}NOTIFY_TYPE;

//�ı���Ϣ��Ӧ֪ͨ
//��Ӧ��NOTIFY_TYPEΪ: NOTIFY_TYPE_TEXT_MSG_RESP
typedef struct
{
    unsigned long   ulRetCode;  //������
    unsigned long   ulReason;
    TEXT_MSG        stTextMsg;  //���ص��ı���Ϣ
}NOTIFY_TEXT_MSG_RESP;

// added by dhong
typedef struct 
{
char *pRcvData;
char *pSendBody;
}Sip_Rcv_Data;

//�ı���Ϣ����֪ͨ
//��Ӧ��NOTIFY_TYPEΪ: NOTIFY_TYPE_TEXT_MSG_REQ
typedef TEXT_MSG NOTIFY_TEXT_MSG_REQ;

//֪ͨ�ص�������֪ͨ��Ϣ�ṹ��
typedef struct
{
    unsigned long   ulNotifyType;       //֪ͨ���ͣ�ȡֵ��NOTIFY_TYPE
    void*           pNotifyInfo;        //֪ͨ��Ϣ�������������ulNotifyType��ȡֵ��NOTIFY_TYPE
    unsigned long   ulNotifyInfoLen;    //pNotifyInfo���ݵ�ʵ�ʳ��ȣ�������ȷ
}NOTIFY_INFO;

//֪ͨ�ص���������
//����pstNotifyInfoΪ֪ͨ��Ϣ�ṹ��ָ��
//����pUserDataΪ�û�����ָ��
typedef long (STDCALL *PCALLBACK_NOTIFY)(NOTIFY_INFO* pstNotifyInfo, void* pUserData);
}

}//end namespace
#pragma pack(pop)

#endif //_SIP_NAME_SPACE_H_HEADER_INCLUDED_

