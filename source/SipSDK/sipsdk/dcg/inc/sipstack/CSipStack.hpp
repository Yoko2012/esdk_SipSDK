/**
* @file  CSipStack.h
* @brief SipЭ��ջ�ඨ��  
*
* Copyright (c) 2010
* Huawei Tech.Co.,Ltd
*
* @author   Li GengQiang/l00124479
* @date     2010/12/06
* @version  NVS V100R002C01
*
*/

#ifndef IVSCBB_SS_SIPSTACK_HPP
#define IVSCBB_SS_SIPSTACK_HPP

#include <map>
#include "MiniSip.h"
#include "ace_header.h"
#include "sipstack_agent_header.hpp"

namespace SipStack{
class CSipStack;

//ȫ��SIPЭ��ջָ��
extern CSipStack* g_pSipStack;

/**
* Class name:   CSipTimerTrigger
* @brief        SIP��ʱ������
*
* Description:  
*/
class CSipStackTimerTrigger : public ITrigger
{
public:
    /**
    * Description:    CSipTimerTrigger(). Default constructor
    */
    CSipStackTimerTrigger()
    {
    };

    /**
    * Description:    CSipTimerTrigger(). Destructor
    */
    virtual ~CSipStackTimerTrigger()
    {
    };

    /**
    * Description:  ������ʱ�¼�����
    * @param  [in]  pArg        ���崦����
    * @param  [in]  ullScales   ��ʱ��ָ��
    * @param  [in]  enStyle     ��������
    */
    void OnTrigger
    (
        void *pArg, 
        ULONGLONG ullScales, 
        TriggerStyle enStyle
    );
};

class CSipStackMsgTimerTrigger : public ITrigger
{
public:
    /**
    * Description:    CSipTimerTrigger(). Default constructor
    */
    CSipStackMsgTimerTrigger()
    {
    };

    /**
    * Description:    CSipTimerTrigger(). Destructor
    */
    virtual ~CSipStackMsgTimerTrigger()
    {
    };

    /**
    * Description:  ������ʱ�¼�����
    * @param  [in]  pArg        ���崦����
    * @param  [in]  ullScales   ��ʱ��ָ��
    * @param  [in]  enStyle     ��������
    */
    void OnTrigger
    (
        void *pArg, 
        ULONGLONG ullScales, 
        TriggerStyle enStyle
    );
};

/**
* Class name:   CSipUdpHandle
* @brief        Sip��UDP������
*
* Description:  
*/
class CSipUdpHandle : public CUdpSockHandle
{
public:
    /**
    * Description:    CSipUdpHandle(). Default constructor
    */
    CSipUdpHandle();

    /**
    * Description:    ~CSipUdpHandle(). Destructor
    */
    ~CSipUdpHandle();

    /**
    * Description:  handle_recv(). �������ݵ���ʱ���ô˺���
    * @return       void.
    */
    void handle_recv(void);

    /**
    * Description:  handle_send(). �����Է�������ʱ���ô˺���
    * @return       void.
    */
    void handle_send(void);
};

/**
* Class name:   CSipStack
* @brief        SipЭ��ջ��
*
* Description:
*/
class CSipStack
{
    friend class CSipStackMsgWrap;
    friend class CSipStackMsgWrapRigister;
    friend class CSipStackMsgWrapUnRigister;
    friend class CSipStackMsgWrapMessage;
    friend class CSipStackMsgWrapInvite;
    friend class CSipStackMsgWrapBye;
    friend class CSipStackMsgWrapRedirect;
    friend class CSipStackMsgWrapAck;
    friend class CSipStackMsgWrapResp;
    friend class CSipStackMsgWrapInviteResp;
    friend class CSipStackMsgWrapSubscribe;

public:
    /**
    * Description:    CSipStack(). Default constructor
    */
    CSipStack();

    /**
    * Description:    ~CSipStack(). Destructor
    */
    virtual ~CSipStack();

    /**
    * Description:  InitSipStack(). ��ʼ��SipStack
    * @param  [in]  pFunNotifyCallBack  ֪ͨ�ص�����ָ��
    * @param  [in]  pCallBackUserData   ֪ͨ�ص��������û�����ָ��
    * @return       long.       ������
    */
    long Init(SIP::PCALLBACK_NOTIFY pfnNotify, void* pNotifyFnParams, const unsigned short local_port );

    /**
    * Description:  ReleaseSipStack(). �ͷ�SipStack
    * @return       long.       ������
    */
    long Fini();

    /**
    * Description:  SetRegisterInfo().  ����ע����Ϣ
    * @param  [in]  stRegisterInfo  ע����Ϣ
    * @return       long.           ������
    */
    long SetRegisterInfo(const SIP::REGISTER_INFO& stRegisterInfo);

    ///////////////////////////////////////////  add by w00207012 2012-11-02 Begin //////////////////////////////////////////////////
    /**
    * Description:  SetRegisterInfo().  ����ע����Ϣ
    * @param  [in]  bServerAddrEx  �Ƿ�ʹ�÷�������ַ
    * @param  [in]  serverIP              ������IP
    * @param  [in]  serverPort          ������Port
    * @return       long.           ������
    */
    long SetServerAddrEx(bool bServerAddrEx, const string& serverIP, unsigned short serverPort);

    /**
    * Description:  isServerAddrEx().  �ж��Ƿ���ǰ�˽��������ڷ�װ��Ϣ
    * @return       bool.
    */
    bool IsServerAddrEx(void);

    /**
    * Description:  SendRegisterResp().  ����ע������Ϣ
    * @param  [in]  seq              ���к�
    * @param  [in]  retCode      �����
    * @return       long.           ������
    */
    long SendRegisterResp(const unsigned long seq, const unsigned long retCode);

    /**
    * Description:  SendKeepaliveResp().  �������������Ϣ
    * @param  [in]  seq              ���к�
    * @param  [in]  retCode      �����
    * @return       long.           ������
    */
    long SendKeepaliveResp(const unsigned long seq, const unsigned long retCode);

    /**
    * Description:  SendOptionResp().  ����option ��Ϣ��ǧ���������ذ���
    * @param  [in]  seq              ���к�
    * @param  [in]  retCode      �����
    * @return       long.           ������
    */
    long SendOptionResp(const unsigned long seq, const unsigned long retCode);

    /**
    * Description:  SendInviteAck().  ����Invite Ack��Ϣ
    * @param  [in]  seq              ���к�
    * @return       long.           ������
    */
    long SendInviteAck(const unsigned long seq);

    /**
    * Description:  SendInviteBye().  ����Invite Bye��Ϣ
    * @param  [in]  seq              ���к�
    * @return       long.           ������
    */
    long SendInviteBye(const unsigned long seq);

    /**
    * Description:  SendInviteCancel().  ����Invite Cancel��Ϣ
    * @param  [in]  seq              ���к�
    * @return       long.           ������
    */
    long SendInviteCancel(const unsigned long seq);	
    ///////////////////////////////////////////  add by w00207012 2012-11-02 End ////////////////////////////////////////////////////

    /**
    * Description:  GetTuObjId().   ��ȡTU����ID
    * @return       unsigned long.           TU����ID
    */
    unsigned long GetTuObjId();

    /**
    * Description:  SendSipMsg().   ����Sip��Ϣ
    * @param  [in]  stTextMsg   �ı���Ϣ
    * @return       long.       ������
    */
    long SendSipMsg(const SIP::TEXT_MSG& stTextMsg, const string& callID = "");

    /**
    * Description:  HandleSipTxnHiSfRspInd().   ������״̬Sip��Ӧ��Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]  stSipMsg    �յ���SIP��Ӧ��Ϣ
    * @return       void.
    */
    void HandleSipTxnHiSfRspInd
    (
        SS_UINT32            ulTuObjId,
        SS_UINT32            ulTxnObjId,
        const SipTptNwInfoS& stTptNwInf,
        SipMsgS&             stSipMsg
    );

    /**
    * Description:  HandleSipTxnHiSfReqInd().   ������״̬Sip������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]  stSipMsg    �յ���SIP������Ϣ
    * @return       void.
    */
    void HandleSipTxnHiSfReqInd
    (
        SS_UINT32            ulTuObjId,
        SS_UINT32            ulTxnObjId,
        const SipTptNwInfoS& stTptNwInf,
        SipMsgS&             stSipMsg
    );

    /**
    * Description:  HandleSipTxnHiSlReqInd().   ������״̬Sip������Ϣ
    * @param  [in]  stSipMsg    �յ���SIP������Ϣ
    * @return       void.
    */
    void HandleSipTxnHiSlReqInd
    (
        const SipTptNwInfoS& stTptNwInf,
        SipMsgS&             stSipMsg
    );

    /**
    * Description:  AddEvent(). ����¼�
    * @param  [in]  stEventInfo     �¼���Ϣ����
    * @return       long.   ������
    */
    long AddEvent
    (
        const SIP::EVENT_INFO& stEventInfo
    );

    /**
    * Description:  GetTimerId().   ��ȡ��ʱ��ID
    * @return       unsigned long.  ��ʱ��ID
    */
    unsigned long GetTimerID();

    //BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
    /**
    * Description:  GetThreadMutex().   ��ȡ�߳���
    * @return       VOS_Mutex*.     ��ָ��
    */
    VOS_Mutex* GetThreadMutex();
    //END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�

    /**
    * Description:  GetCSeq().   ��ȡSIP��Ϣ��CSeq
    * @return       unsigned long.  SIP��Ϣ��CSeq
    */
    unsigned long GetCSeq();

    long SendInviteResp(const string &msg, const unsigned long seq, const unsigned long retCode);
    long SendResp(const unsigned long seq);

    long AnsMsg( const string& ans_code, const string& data);
private:
    /**
    * Description:  ProcEventThreadEntry().  �����¼��߳����
    * @param  [in]  pPara   �߳���ڲ���
    * @return       long.   ������
    */
    static unsigned long STDCALL ProcEventThreadEntry(void* pPara);

    /**
    * Description:  ProcEventThread().  �����¼��߳�
    * @return       void.
    */
    void ProcEventThread();

    /**
    * Description:  ClearAllMsgInfo().  ����������Ϣ��Ϣ
    * @return       void.
    */
    void ClearAllMsgInfo();

    /**
    * Description:  NotifyCallBack(). ���ûص�����
    * @param  [in]  stInfo  ֪ͨ��Ϣ�ṹ��
    * @return       long.   ������
    */
    long NotifyCallBack(SIP::NOTIFY_INFO& stInfo);

    /**
    * Description:  NotifyTextMsgResp(). ֪ͨ�ı���Ӧ��Ϣ
    * @param  [in]  stNotifyInfo    �ı���Ӧ��Ϣ֪ͨ��Ϣ�ṹ��
    * @return       long.   ������
    */
    long NotifyTextMsgResp(SIP::NOTIFY_TEXT_MSG_RESP& stNotifyInfo);

    /**
    * Description:  NotifyTextMsgReq(). ֪ͨ�ı�������Ϣ
    * @param  [in]  stNotifyInfo        �ı�������Ϣ֪ͨ��Ϣ�ṹ��
    * @return       long.   ������
    */
    long NotifyTextMsgReq(SIP::NOTIFY_TEXT_MSG_REQ& stNotifyInfo);

    /**
    * Description:  SetLocalAddr().  ���ñ��ص�ַ
    * @return       void.
    */
    void SetLocalAddr(const unsigned short local_port);

    /**
    * Description:  HandleEventTimerTrigger().  ����ʱ�������¼�
    * @return       void.
    */
    void HandleEventTimerTrigger();

    /**
    * Description:  HandleEventTimerRegister(). ����ʱ��ע���¼�
    * @param  [in]  pEventInfo      �¼���Ϣָ��
    * @param  [in]  ulEventInfoLen  �¼���Ϣ���ݳ���
    * @return       long.       ������
    */
    long HandleEventTimerRegister
    (
        void*           pEventInfo, 
        unsigned long   ulEventInfoLen
    );

    /**
    * Description:  HandleEventTimerCancel().   ����ʱ��ȡ���¼�
    * @param  [in]  pEventInfo      �¼���Ϣָ��
    * @param  [in]  ulEventInfoLen  �¼���Ϣ���ݳ���
    * @return       long.       ������
    */
    long HandleEventTimerCancel
    (
        void*           pEventInfo, 
        unsigned long   ulEventInfoLen
    );

    /**
    * Description:  HandleEventRedirect().  �����ض����¼�
    * @param  [in]  pEventInfo      �¼���Ϣָ��
    * @param  [in]  ulEventInfoLen  �¼���Ϣ���ݳ���
    * @return       void.
    */
    void HandleEventRedirect
    (
        void*           pEventInfo, 
        unsigned long   ulEventInfoLen
    );

    /**
    * Description:  GetDialogIdBySipMsg().    ����SIP��Ϣ�ҵ��Ի�ID
    * @param  [in]      stSipMsg    SIP��Ϣ�ṹ������
    * @param  [out] ulDialogId      �Ի�ID
    * @param  [in]      bErase      �Ƿ�ɾ��
    * @return       long.       ������
    */
    long GetDialogIdBySipMsg
    (
        SipMsgS&        stSipMsg,
        unsigned long&  ulDialogId,
        VOS_BOOLEAN     bErase = VOS_FALSE
    );

    /**
    * Description:  GetDialogIdBySipMsg().    ����Ի���Ϣ�ҵ��Ի�ID
    * @param  [in]      stDialogInfo    �Ի���Ϣ
    * @param  [out] ulDialogId      �Ի�ID
    * @param  [in]      bErase      �Ƿ�ɾ��
    * @return       long.       ������
    */
    long GetDialogIdByInfo
    (
        const SIP::DIALOG_INFO& stDialogInfo,
        unsigned long&          ulDialogId,
        VOS_BOOLEAN             bErase = VOS_FALSE
    );

    /**
    * Description:  GetDialogInfoById().      ���ݶԻ�ID�ҵ��Ի���Ϣ
    * @param  [in]      ulDialogId      �Ի�ID
    * @param  [out]     stDialogInfo    �Ի���Ϣ
    * @return       long.       ������
    */
    long GetDialogInfoById
    (
        unsigned long       ulDialogId,
        SIP::DIALOG_INFO&   stDialogInfo
    );

    ////////////////////////////////// add by wx153027 2012-12-18 Begin /////////////////////////////////
    /**
    * Description:  HandleReqOption().  ����Option������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
    * @return       long.       ������
    */
    long HandleReqOption
    (
        SS_UINT32                   ulTuObjId,
        SS_UINT32                   ulTxnObjId,
        const SipTptNwInfoS&        stTptNwInf,
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
    );
    ////////////////////////////////// add by wx153027 2012-12-18 end /////////////////////////////////

    ////////////////////////////////// add by w00207027 2012-10-27 Begin /////////////////////////////////
    /**
    * Description:  HandleReqRegister().  ����Register������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
    * @return       long.       ������
    */
    long HandleReqRegister
    (
        SS_UINT32                   ulTuObjId,
        SS_UINT32                   ulTxnObjId,
        const SipTptNwInfoS&        stTptNwInf,
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
    );

    /**
    * Description:  CheckRegistAuthInfo().  У��Regist�����Authenticate��Ϣ
    * @param  [in]      stSipMsg                    SIP��Ϣ����
    * @param  [out]    pstSipAuthorization   Regist�����Authenticate��Ϣ
    * @return       long.       ������
    */
    long CheckRegistAuthInfo
    (
        SipMsgS&                    stSipMsg,
        SipAuthorization* &         pstSipAuthorization
    );

    /**
    * Description:  getStringFromQuoteString().  У��Regist�����Authenticate��Ϣ
    * @param  [in]      sipString           SIP��Ϣ����
    * @param  [out]    str                      �����Quote��string
    * @return    long  .       ������
    */
    long GetStringFromQuoteString
    (
        const SipString &sipString, 
        string &str
    );

    /**
    * Description:  HandleReqNotify().  ����Notify������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
    * @return       long.       ������
    */
    long HandleReqNotify
    (
        SS_UINT32                   ulTuObjId,
        SS_UINT32                   ulTxnObjId,
        const SipTptNwInfoS&        stTptNwInf,
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
    );

    /**
    * Description:  isRespDirect().     �ж��Ƿ�ֱ�ӻ���Ӧ��Ϣ
    * @param  [in]      pMsgBody            SIP��Ϣ��
    * @return   bool.       ������
    */
    long IsRespDirect
    ( 
        const char* pMsgBody
    );
    ////////////////////////////////// add by w00207027 2012-10-27 End /////////////////////////////////

    /**
    * Description:  HandleReqInvite().  ����Invite������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
    * @return       long.       ������
    */
    long HandleReqInvite
    (
        SS_UINT32                   ulTuObjId,
        SS_UINT32                   ulTxnObjId,
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
    );

    /**
    * Description:  HandleReqMessage().     ����Message������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
    * @return       long.       ������
    */
    long HandleReqMessage
    (
        SS_UINT32                   ulTuObjId,
        SS_UINT32                   ulTxnObjId,
        const SipTptNwInfoS&        stTptNwInf,
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
    );

    /**
    * Description:  HandleReqBye().     ����Bye������Ϣ
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
    * @return       long.       ������
    */
    long HandleReqBye
    (
        SS_UINT32                   ulTuObjId,
        SS_UINT32                   ulTxnObjId,
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
    );

private:
      int GetAddrWithSipTptIpPortS( IN  const SipTptIpPortS& stTptAddr,
                                    OUT std::string&         strIPv4,
                                    OUT unsigned short&      usPort
                                  );
private:
    //SIP��Ϣ��Ϣ�ṹ��
    typedef struct _SIP_MSG_INFO
    {
        unsigned long       ulMsgSeq;       //�ϲ㴫��������Ϣ��ţ��ص�֪ͨʱ��Ҫ
        SIP::TEXT_MSG_TYPE  enTextMsgType;  //�ı���Ϣ����
        SS_UINT32           ulTxnObjId;     //�������ID
        SS_UINT32           ulTuObjId;      //TU����ID
        unsigned long       ulDialogId;     //INVITE����ĶԻ�ID
        SipMsgS*            pstSipMsg;      //SIP��Ϣָ��

        _SIP_MSG_INFO()
        {
            ulMsgSeq        = 0;
            enTextMsgType   = SIP::TEXT_MSG_TYPE_SIP_REGISTER;
            ulTxnObjId      = 0;
            ulTuObjId       = 0;
            ulDialogId      = 0;
            pstSipMsg       = NULL;
        }
    }SIP_MSG_INFO;

    class NoCmpDialogInfo
    {
    public:
        bool operator()
            (
            const SIP::DIALOG_INFO&, 
            const SIP::DIALOG_INFO&
            ) const
        {
            return true;
        }
    };

    //CSeq����SIP��Ϣ��Ϣ��ӳ�������
    typedef map<unsigned long, SIP_MSG_INFO*> MAP_C_SEQ_TO_SIP_MSG_INFO;

    //�Ի�ID��Ի���Ϣ��ӳ�������
    typedef map<unsigned long, SIP::DIALOG_INFO> MAP_DIALOG_ID_TO_INFO;

    //�Ի���Ϣ��Ի�ID��ӳ�������
    typedef map<SIP::DIALOG_INFO, unsigned long, NoCmpDialogInfo> MAP_DIALOG_INFO_TO_ID;

    //��ʱ��ID�붨ʱ����Ϣ��ӳ�������
    typedef map<unsigned long, SIP::TIMER_INFO> MAP_TIMER_ID_TO_INFO;

    //��������Ϣ�б������
    typedef list<SIP::SERVER_INFO> LIST_SVR_INFO;

    //��������Ϣ���ұȽϷ�����
    class FindServerInfo
    {
    public:
        FindServerInfo (const SIP::SERVER_INFO& stSvrInfo) : m_stSvrInfo(stSvrInfo)
        {
        }

        bool operator()(const SIP::SERVER_INFO& stSvrInfo)      
        {
            if (0 == strncmp(m_stSvrInfo.szServerIP, stSvrInfo.szServerIP, sizeof(m_stSvrInfo.szServerIP))
                && m_stSvrInfo.usServerPort == stSvrInfo.usServerPort)
            {
                return true;
            }

            return false;
        }

    private:
        const SIP::SERVER_INFO m_stSvrInfo;
    };

    //��������Ի���Ϣ���ұȽϷ�����
    class FindDialogInfo
    {
    public:
        FindDialogInfo (const SIP::DIALOG_INFO& stDialogInfo) : m_stDialogInfo(stDialogInfo)
        {
        }

        bool operator()(const MAP_DIALOG_INFO_TO_ID::value_type& objValueType)      
        {
            if ((objValueType.first.strCallId     == m_stDialogInfo.strCallId)
                && (objValueType.first.strLocalTag == m_stDialogInfo.strLocalTag)
                && (objValueType.first.strRemoteTag   == m_stDialogInfo.strRemoteTag))
            {
                return true;
            }

            return false;
        }

    private:
        const SIP::DIALOG_INFO m_stDialogInfo;
    };

private:
    /**
    * Description:  GetSipMsgInfoByCSeq().      ����CSeq�ҵ�SIP��Ϣ����Ϣ
    * @param  [in]      ulCSeq              SIP��Ϣ��CSeq
    * @param  [out] pstSipMsgInfo    SIP��Ϣ��Ϣָ��
    * @param  [in]  bErase      �Ƿ�ɾ��
    * @return       long.       ������
    */
    long GetSipMsgInfoByCSeq
    (
        unsigned long   ulCSeq,
        SIP_MSG_INFO*&  pstSipMsgInfo,
        VOS_BOOLEAN     bErase = VOS_FALSE
    );

    /**
    * Description:  HasRedirect().  ����Ƿ����ض����¼�
    * @param  [in]  stSipMsgInfo    SIP��Ϣ��Ϣ����
    * @return       VOS_BOOLEAN.    �Ƿ�  ���ض���
    */
    VOS_BOOLEAN HasRedirectEvent
    (
        const SIP_MSG_INFO& stSipMsgInfo
    );

    /**
    * Description:  HandleUnAuthResp().     ����δ��Ȩ��Ӧ
    * @param  [in]  stSipMsg            SIP��Ϣ����
    * @param  [in]  stReqMsgInfo  ������Ϣ��Ϣ����
    * @return       long.       ������
    */
    long HandleUnAuthResp
    (
        SipMsgS&        stSipMsg,
        SIP_MSG_INFO&   stReqMsgInfo,
        bool            bFirst=true
    );

public:
    /**
    * Description:  HandleUnAuthResp().     ����ע�ᣬadded by dhong
    * @return       long.       ������
    */
    unsigned long InsertReqMsg(SipMsgS &sipMsg, const SS_UINT32 &ulTuObjId, const SS_UINT32 &ulTxnObjId, const unsigned long &ulDialogId);
    void CheckReqMsg(void);
    long HandleReRegister(void);
    SIP::REQ_MSG_INFO* GetReqMsg(unsigned long seq, bool bGetAndDelete = true);

protected:
    SIP_HEADER *GetSipHeader(const SipMsgS &sipMsg, bool bReq);
    void       GetFromSipString(const SipString &sipString, string &str);

private:
    /**
    * Description:  HandleUnAuthResp().     ����δ��Ȩ��Ӧ
    * @param  [in]  stSipMsg            SIP��Ϣ����
    * @param  [in]  stReqMsgInfo  ������Ϣ��Ϣ����
    * @return       long.       ������
    */
    long HandleAuthOnLineResp
    (
        const SipMsgS&        stSipMsg,
        SIP_MSG_INFO&   stReqMsgInfo
    );

    /**
    * Description:  HandleSuccessResp().     ����ɹ���Ӧ
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [in]  stReqMsgInfo  ������Ϣ��Ϣ����
    * @param  [out] stNotifyResp    ֪ͨ�ı���Ӧ�ṹ������
    * @return       long.       ������
    */
    long HandleSuccessResp
    (
        SipMsgS&                    stSipMsg,
        const SIP_MSG_INFO&         stReqMsgInfo,
        SIP::NOTIFY_TEXT_MSG_RESP&  stNotifyResp
    );

    /**
    * Description:  HandleSuccessRespRegister().     ����Register�ɹ���Ӧ
    * @param  [in]      stSipMsg            SIP��Ϣ����
    * @param  [out] stNotifyResp    ֪ͨ�ı���Ӧ�ṹ������
    * @return       long.       ������
    */
    long HandleSuccessRespRegister
    (
        SipMsgS&                    stSipMsg,
        SIP::NOTIFY_TEXT_MSG_RESP&  stNotifyResp
    );

    /**
    * Description:  HandleSuccessRespInvite().     ����Invite�ɹ���Ӧ
    * @param  [in]  stSipMsg        SIP��Ϣ����
    * @param  [in]  stReqMsgInfo    ������Ϣ��Ϣ����
    * @param  [out] stNotifyResp    ֪ͨ�ı���Ӧ�ṹ������
    * @return       long.       ������
    */
    long HandleSuccessRespInvite
    (
        SipMsgS&                    stSipMsg,
        const SIP_MSG_INFO&         stReqMsgInfo,
        SIP::NOTIFY_TEXT_MSG_RESP&  stNotifyResp
    );

private:
    SIP::PCALLBACK_NOTIFY   m_pfnNotify;        //�ص�����ָ��
    void*                   m_pNotifyFnParams;  //�û�����ָ��

    //�����¼��߳�
    VOS_Thread* m_pProceEventThread;
    //�����¼��߳��Ƿ�����
    VOS_BOOLEAN m_bProcEventThreadRun;
    //ͬ���¼�����
    SIP::CEventSyncQueue m_objEventSyncQueue;

    //��ʱ��ID
    unsigned long m_ulTimerId;
    //��ʱ��ID�붨ʱ����Ϣ��ӳ���
    MAP_TIMER_ID_TO_INFO       m_mapTimerIdToInfo;
    ACE_Recursive_Thread_Mutex m_mutexTimerMap;

    //��ʱ��
    CSipStackTimerTrigger* m_pTimerTrigger;
    // Message ��ʱ��
    CSipStackMsgTimerTrigger *m_pMsgTimerTrigger;
    //�Ƿ���ע��
    volatile VOS_BOOLEAN m_bRegister;

    //ע����Ϣ
    SIP::REGISTER_INFO m_stRegisterInfo;
    //�����MD5��16�����ַ���
    char m_szPwdMd5[SIP::MD5_HEX_LEN];
    //ʹ�ù��ķ�������Ϣ�б�
    LIST_SVR_INFO m_listSvrInfoUsed;
    //��ǰ�ض���������б�
    SIP::VECTOR_SERVER_INFO m_vectorServerInfo;
    //�Ƿ����ض���
    VOS_BOOLEAN m_bHasRedirect;
    //�Ƿ��ض�����������ϱ��ض����б�
    VOS_BOOLEAN m_bRedirected;
    //�Ƿ�Ϊ��ʱ�ض���
    VOS_BOOLEAN m_bTempRedirect;

    //SIP��UDP������
    CSipUdpHandle m_objSipUdpHandle;
    //���ص�ַ��������
    CNetworkAddr  m_objLocalAddr;
    //��������ַ��������
    CNetworkAddr  m_objServerAddr;

    ////////////////////////////// add by w00207027 2012-11-02 Begin ////////////////////////////////////////////
    //��������ַ��������
    CNetworkAddr  m_objServerAddrEx;

    bool      m_bServerAddrEx;
    ////////////////////////////// add by w00207027 2012-11-02 End //////////////////////////////////////////////

    //modify-cwx148380-begin
    string m_strIP;
    unsigned short m_usPort;
    //modify-cwx148380-end

    //TU����ID
    unsigned long m_ulTuObjId;

    //Map��
    VOS_Mutex* m_pMapMutex;
    //BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
    //MiniSIP�߳���
    VOS_Mutex* m_pThreadMutex;
    //END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�

    //�Ի�ID���Ի���Ϣ��ӳ���
    MAP_DIALOG_ID_TO_INFO m_mapDialogIdToInfo;
    //�Ի���Ϣ���Ի�ID��ӳ���
    MAP_DIALOG_INFO_TO_ID m_mapDialogInfoToId;

    //����Register��Call-ID
    string m_strRegisterCallId;

    //CSeq����SIP��Ϣ��Ϣ��ӳ���
    MAP_C_SEQ_TO_SIP_MSG_INFO m_mapCSeqToSipMsgInfo;

    //SIP��Ϣ��CSeq
    unsigned long m_ulCSeq;

    //ƽ̨��Ȩ��Ϣ
    SIP::PLAT_AUTH_INFO m_stPlatAuthInfo;

    //ƽ̨��Ȩ��Ϣ�������
    unsigned long m_ulPlatAuthInfoChangeNum;

    // ����401��������SipMsgS��Ϣ, ��������ע��ʱʹ��
    //SipMsgS    m_authMsg;
    SIP_MSG_INFO m_msgInfo;

    // ����������Ϣ���ݱ���
    VOS_Mutex* m_pReqMsgMutex;
    map<unsigned long, SIP::REQ_MSG_INFO*>  m_msgMap;
    map<unsigned long, SIP::REQ_MSG_INFO*>::iterator m_msgItertor;
    unsigned long m_mapKey;

    unsigned long m_reserved;

    // Ƶ������SetRegisterInfo,�������������
    // for m_listSvrInfoUsed
    VOS_Mutex* m_pThreadlistSvrInfoMutex;
    // for m_vectorServerInfo
    VOS_Mutex* m_pThreadServerInfoMutex;


    //���ڴ���ǰ���ط�SIP���ĵ����ݽṹ��
    int HandleReqResend( const unsigned long enHeader, const SipTptNwInfoS& stTptNwInf, const unsigned long nCSeq );
    int HandleRspResend( const unsigned long enHeader, const SipTptNwInfoS& stTptNwInf, const unsigned long nCSeq );
    class CDealResendData
    {
    public:
        CDealResendData ();
        CDealResendData ( const std::string& strIPv4, unsigned short usPort,  unsigned long nCSeq );


    public:
        const std::string& GetIPv4() const;
        unsigned short GetPort() const;
        unsigned long GetCSeq() const;

        bool IsTheSame ( const CDealResendData& other );
        bool IsTimeout ();

    private:
        const std::string    m_strIPv4;
        const unsigned short m_usPort;
        const unsigned long  m_nCSeq;


        time_t               m_tResendTime;
    };
    typedef std::list<CDealResendData*>         DEAL_RESEND_LIST;
    typedef std::map<int, DEAL_RESEND_LIST>    DEAL_RESEND_HEADER_MAP;

    DEAL_RESEND_HEADER_MAP m_mapReqResend;
    DEAL_RESEND_HEADER_MAP m_mapRspResend;
};

}//end namespace

#endif //_C_SIP_STACK_H_HEADER_INCLUDED_

