#ifndef IVSCBB_SS_SIPSTACK_WRAP_HPP
#define IVSCBB_SS_SIPSTACK_WRAP_HPP

#include "CSipStack.hpp"

namespace SipStack{

class CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsg(). Default constructor
    */
    CSipStackMsgWrap(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

    /**
    * Description:  SetNwAddrInf  ���������ַ��Ϣ
    * @return       int.         ������
    */
    int SetNwAddrInf( const string& ip, const unsigned short port );

	    /**
    * Description:  SetlcAddrInf  ���ñ�����ַ��Ϣ
    * @return       int.         ������
    */
    int SetLcAddrInf( const string& ip, const unsigned short port );

    /**
    * Description:    ~CSipMsg(). Destructor
    */
    virtual ~CSipStackMsgWrap();

    /**
    * Description:  SendMsg().  ������Ϣ
    * @return       long.       ������
    */
    virtual long SendMsg();

protected:
    /**
    * Description:  CreateSipReqMsg().  ����SIP������Ϣ
    * @param  [in]  enSipMethod SIP��������
    * @param  [out] pstSipMsg   SIP��Ϣָ��
    * @return       long.       ������
    */
    long CreateSipReqMsg
    (
        EN_SIP_METHOD           enSipMethod, 
        SipMsgS*&               pstSipMsg
    );

    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS& stSipMsg
    );

    /**
    * Description:  SetVarHeader().     ���ÿɱ�ͷ��
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    long SetVarHeader
    (
        SipMsgS& stSipMsg
    );

    /**
    * Description:  SendSipMsg().   ����SIP��Ϣ
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @param  [in]  objServerAddr    ����SIP��Ϣ��������ַ  // add by w00207027 2012-11-02
    * @return       long.       ������
    */
    virtual long SendSipMsg
    (
        SipMsgS& stSipMsg
        //CNetworkAddr& objServerAddr
    );

    /**
    * Description:  SendSipReqMsgWithVarHdr().  ���ʹ��ɱ�ͷ���SIP������Ϣ
    * @param  [in]  stSipMsg    SIP��Ϣ����
    * @return       long.       ������
    */
    long SendSipReqMsgWithVarHdr
    (
        SipMsgS& stSipMsg
    );

    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();

    /**
    * Description:  SipTxnHiSend().  ����MiniSIP�ķ��ͽӿ�
    * @param  [in]  stSipSdu             SIP����Ϣ���ݵ�Ԫ
    * @param  [in]  objServerAddr    ����SIP��Ϣ��������ַ  // add by w00207027 2012-11-02
    * @param  [out] ulTuObjId            ��Ӧ��TU����ID
    * @param  [out] ulTxnObjId          ��Ӧ���������ID
    * @return       long.       ������
    */
    long SipTxnHiSend
    (
        SipDataUnitS&   stSipSdu,
        SS_UINT32&      ulTxnObjId,
        SS_UINT32&      ulTuObjId
    );

    friend class CSipStack;

protected:
    //SIPЭ��ջ��������
    CSipStack& m_objSipStack;
    //�ı���Ϣ�ṹ������
    const SIP::TEXT_MSG& m_stTextMsg;

    //Request-URI��Name
    const char* m_pszRequestUriName;
    //Request-URI��Domain
    const char* m_pszRequestUriDomain;
    // User-Agent��Name
    const char* m_pszUserAgentName;
    //SIP��Ϣ��CSeq
    unsigned long m_ulCSeq;

    //CallID
    string m_strCallId;
    //From��Tag
    string m_strFromTag;
    //To��Tag
    const char* m_pszToTag;

    //modify-cwx148380-begin 2013-1-15 14:25:34
    string m_strIP;
    unsigned short m_usPort;
    //modify-cwx148380-end 2013-1-15 14:25:34
	string m_strLocalIP;
	unsigned short m_usLocalPort;
};

/**
* Class name:   CSipMsgRegister
* @brief        SIP Register��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapRigister : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgRegister(). Default constructor
    */
    CSipStackMsgWrapRigister(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, const string& callID);

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS&                stSipMsg
    );

    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();

protected:
    //��ʱʱ��
    unsigned long m_ulExpireTime;
};

/**
* Class name:   CSipMsgUnRegister
* @brief        SIP Register��Ϣ������ע��
*
* Description:  
*/
class CSipStackMsgWrapUnRigister : public CSipStackMsgWrapRigister
{
public:
    /**
    * Description:    CSipMsgUnRegister(). Default constructor
    */
    CSipStackMsgWrapUnRigister(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg,const string& callID);

protected:
    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();
};

/**
* Class name:   CSipMsgMessage
* @brief        SIP Message��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapMessage : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgMessage(). Default constructor
    */
    CSipStackMsgWrapMessage(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, const string& callID);

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS&                stSipMsg
    );

    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();
};

/**
* Class name:   CSipMsgMessage
* @brief        SIP Message��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapOptions : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgMessage(). Default constructor
    */
    CSipStackMsgWrapOptions(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

protected:
    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();
};

/**
* Class name:   CSipMsgInvite
* @brief        SIP Invite��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapInvite : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgInvite(). Default constructor
    */
    CSipStackMsgWrapInvite ( CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, const string& callID );

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS&                stSipMsg
    );

    /**
    * Description:  SendMsg().  ������Ϣ
    * @return       long.       ������
    */
    virtual long SendMsg();

    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();

private:
    //INVITE������ͷ�����
    SIP::REQ_HEADER_INVITE m_stReqHdrInvite;    
};

/**
* Class name:   CSipMsgAck
* @brief        SIP Invite��Ack��Ӧ��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapAck : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgAck(). Default constructor
    */
    CSipStackMsgWrapAck(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

protected:
    /**
    * Description:  SendSipMsg().   ����SIP��Ϣ
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    long SendSipMsg
    (
        SipMsgS& stSipMsg
    );
};

/**
* Class name:   CSipMsgBye
* @brief        SIP BYE��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapBye : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgBye(). Default constructor
    */
    CSipStackMsgWrapBye(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

    /**
    * Description:  SendMsg().  ������Ϣ
    * @return       long.       ������
    */
    virtual long SendMsg();

protected:
	/**
	* Description:  SipTxnHiSend().  ����MiniSIP�ķ��ͽӿ�
	* @param  [in]  stSipSdu    SIP����Ϣ���ݵ�Ԫ
	* @param  [in]  objServerAddr    ����SIP��Ϣ��������ַ  // add by w00207027 2012-11-02
	* @param  [out] ulTuObjId   ��Ӧ��TU����ID
	* @param  [out] ulTxnObjId  ��Ӧ���������ID
	* @return       long.       ������
	*/
	long SipTxnHiSend
	(
		SipDataUnitS&   stSipSdu,
		SS_UINT32&      ulTxnObjId,
		SS_UINT32&      ulTuObjId
	);

    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();

protected:
    //��������ID
    unsigned long m_ulTxnObjId;
    //�����û�����ID
    unsigned long m_ulTuObjId;

    //�Ƿ�ΪCancel��Ϣ
    VOS_BOOLEAN m_bCancel;
};

/**
* Class name:   CSipMsgSubscribe
* @brief        SIP SUBSCRIBE������Ϣ
*
* Description:  
*/
class CSipStackMsgWrapSubscribe : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgSubscribe(). Default constructor
    */
    CSipStackMsgWrapSubscribe(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stSipMsg   SIP��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS&                stSipMsg
    );

    /**
    * Description:  PostSendSuccess().  ���ͳɹ�����
    * @return       void. 
    */
    virtual void PostSendSuccess();

private:
    //��ʱʱ��
    unsigned long m_ulExpireTime;
};

/**
* Class name:   CSipMsgInviteResp
* @brief        SIP Invite����Ӧ��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapInviteRespEx : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgInviteResp(). Default constructor
    */
    CSipStackMsgWrapInviteRespEx(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, unsigned long retCode=200);

    /**
    * Description:  SendMsg().  ������Ϣ
    * @return       long.       ������
    */
    virtual long SendMsg();

private:
    unsigned long m_retCode;
};

/**
* Class name:   CSipMsgRedirect
* @brief        SIP �ض�����Ϣ
*
* Description:  
*/
class CSipStackMsgWrapRedirect : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgRedirect(). Default constructor
    */
    CSipStackMsgWrapRedirect(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

    /**
    * Description:  SendMsg().  ������Ϣ
    * @return       long.       ������
    */
    virtual long SendMsg();
};

/**
* Class name:   CSipMsgReAuth
* @brief        SIP �ؼ�Ȩ��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapReAuth : public CSipStackMsgWrap
{
public:
    /**
    * Description:    CSipMsgReAuth(). Default constructor
    */
    CSipStackMsgWrapReAuth(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg);

    /**
    * Description:  SendMsg().  ������Ϣ
    * @return       long.       ������
    */
    virtual long SendMsg();
};

/**
* Class name:   CSipRspMsg
* @brief        SIP��Ӧ��Ϣ
*
* Description:
*/
class CSipStackMsgWrapResp
{
public:
    /**
    * Description:    CSipRspMsg(). Default constructor
    */
    CSipStackMsgWrapResp(CSipStack& objSipStack, SipMsgS& stSipReqMsg, const string& data = "");

    /**
    * Description:    ~CSipRspMsg(). Destructor
    */
    virtual ~CSipStackMsgWrapResp();

    /**
    * Description:  SendRspMsg().  ������Ӧ��Ϣ
    * @param  [in]  ulRespCode  ��Ӧ��
    * @param  [in]  ulTuObjId   ��Ӧ��TU����ID
    * @param  [in]  ulTxnObjId  ��Ӧ���������ID
    * @return       long.       ������
    */
    virtual long SendRspMsg
    (
        unsigned long   ulRespCode,
        unsigned long   ulTuObjId,
        unsigned long   ulTxnObjId
    );

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stRspSipMsg   SIP��Ӧ��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS& stRspSipMsg
    );

protected:
    //SIPЭ��ջ��������
    CSipStack& m_objSipStack;
    //SIP������Ϣ����
    SipMsgS& m_stSipReqMsg;

    string m_SipMsgData;
};

/**
* Class name:   CSipRspMsgInvite
* @brief        SIP INVITE����Ӧ��Ϣ
*
* Description:
*/
class CSipStackMsgWrapInviteResp : public CSipStackMsgWrapResp
{
public:
    /**
    * Description:    CSipRspMsgInvite(). Default constructor
    * @param  [in]  objSipStack     SipStack����
    * @param  [in]  stSipReqMsg     SIP������Ϣ����
    * @param  [in]  ulDialogId          �Ի�ID
    * @param  [in]  szMsgBody       ��Ϣ���ַ���
    * @param  [in]  ulMsgBodyLen    ��Ϣ�峤��
    */
    CSipStackMsgWrapInviteResp
    (
        CSipStack&      objSipStack, 
        SipMsgS&        stSipReqMsg,
        unsigned long   ulDialogId,
        unsigned long   ulMsgBodyLen,
        char*           pszMsgBody
    );

    /**
    * Description:    ~CSipStackMsgWrapInviteResp(). Destructor
    */
    virtual ~CSipStackMsgWrapInviteResp();

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stRspSipMsg     SIP��Ӧ��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS& stRspSipMsg
    );

private:
    unsigned long   m_ulDialogId;   //�Ի�ID
    unsigned long   m_ulMsgBodyLen; //��Ϣ��ĳ���
    char*           m_pszMsgBody;   //��Ϣ������
};

/**
* Class name:   CSipRspMsgRegist
* @brief        SIP REGIST����Ӧ��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapRegisterResp : public CSipStackMsgWrapResp
{
public:
    /**
    * Description:    CSipRspMsgRegist(). Default constructor
    * @param  [in]  objSipStack     SipStack����
    * @param  [in]  stSipReqMsg     SIP������Ϣ����
    * @param  [in]  wwwAuth            ��Ӧ��WWW_AUTHENTICATEͷ��
    */
    CSipStackMsgWrapRegisterResp
    (
        CSipStack&      objSipStack, 
        SipMsgS&        stSipReqMsg,
		bool                   wwwAuth
    );

    /**
    * Description:    ~CSipRspMsgRegist(). Destructor
    */
    virtual ~CSipStackMsgWrapRegisterResp();

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stRspSipMsg     SIP��Ӧ��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS& stRspSipMsg
    );

private:

	bool m_bwwwAuth;

	//��ʱʱ��
	unsigned long m_ulExpireTime;
    //unsigned long   m_ulDialogId;   //�Ի�ID
    //unsigned long   m_ulMsgBodyLen; //��Ϣ��ĳ���
    //char*           m_pszMsgBody;   //��Ϣ������
};

/**
* Class name:   CSipRspMsgOption
* @brief        SIP OPTION����Ӧ��Ϣ
*
* Description:  
*/
class CSipStackMsgWrapOptionsResp : public CSipStackMsgWrapResp
{
public:
    /**
    * Description:    CSipRspMsgOption(). Default constructor
    * @param  [in]  objSipStack     SipStack����
    * @param  [in]  stSipReqMsg     SIP������Ϣ����
    */
    CSipStackMsgWrapOptionsResp
    (
        CSipStack&      objSipStack, 
        SipMsgS&        stSipReqMsg
    );

    /**
    * Description:    ~CSipRspMsgOption(). Destructor
    */
    virtual ~CSipStackMsgWrapOptionsResp();

protected:
    /**
    * Description:  SetSpecialHeader().     ��������ͷ��
    * @param  [in]  stRspSipMsg     SIP��Ӧ��Ϣ����
    * @return       long.       ������
    */
    virtual long SetSpecialHeader
    (
        SipMsgS& stRspSipMsg
    );

private:
	//��ʱʱ��
	//unsigned long m_ulExpireTime;
};

}//end namespace

#endif //_C_SIP_MSG_H_HEADER_INCLUDED_

