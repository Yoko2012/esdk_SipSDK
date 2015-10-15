#include <ace_header.h>


#include "sipstack_msg_wrap.hpp"
#include "sipstack_config.hpp"
#include "CLockGuard.h"
#include "Log.h"

namespace SipStack{

CSipStackMsgWrap::CSipStackMsgWrap(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg)
    :m_objSipStack(objSipStack), m_stTextMsg(stTextMsg),m_strFromTag("")
{
    m_pszRequestUriName     = objSipStack.m_stRegisterInfo.szSipServerId;		//Request-URI��Name
    m_pszRequestUriDomain   = objSipStack.m_stRegisterInfo.szUriDomain;			//Request-URI��Domain
    m_pszUserAgentName = VOS_NULL;

    SetNwAddrInf(m_objSipStack.m_strIP.c_str(), m_objSipStack.m_usPort);
	unsigned short usPort = (unsigned short)atoi(m_objSipStack.m_stRegisterInfo.szLocalPort);
	SetLcAddrInf(m_objSipStack.m_stRegisterInfo.szLocalIp,usPort);

    if  (  (VOS_NULL != m_stTextMsg.pUserData)
        && (SIP::TEXT_MSG_TYPE_SIP_ACK != m_stTextMsg.enTextMsgType)
        && (SIP::TEXT_MSG_TYPE_SIP_BYE != m_stTextMsg.enTextMsgType)
        && (SIP::TEXT_MSG_TYPE_SIP_CANCEL != m_stTextMsg.enTextMsgType)
        )
    {
        SIP::PEER_URI_INFO& stPeerUriInfo = *(SIP::PEER_URI_INFO*)m_stTextMsg.pUserData;

        m_pszRequestUriName     = stPeerUriInfo.szUriUserName;
        //m_pszRequestUriDomain   = stPeerUriInfo.szUriHostName;
        m_pszUserAgentName      = stPeerUriInfo.szUserAgentName;
        //if (NULL != stPeerUriInfo.szCallID)
        //{
        //    m_strCallId   =   stPeerUriInfo.szCallID;
        //}
        //ACE_OS::strcat(m_strCallId.c_str(),  stPeerUriInfo.szCallID);
        //ACE_OS::strcat(m_strCallId.c_str(),  "@");
        //ACE_OS::strcat(m_strCallId.c_str(),  objSipStack.m_stRegisterInfo.szLocalIp);
    }

    m_pszToTag              = VOS_NULL; //To��Tag

    m_ulCSeq        = stTextMsg.ulMsgSeq;    //SIP��Ϣ��CSeq
}

CSipStackMsgWrap::~CSipStackMsgWrap()
{
    m_pszRequestUriName     = VOS_NULL; //Request-URI��Name
    m_pszRequestUriDomain   = VOS_NULL; //Request-URI��Domain
    m_strFromTag            = "";		//From��Tag
    m_pszToTag              = VOS_NULL; //To��Tag
};

/**
* Description:  SetNwAddrInf  ���������ַ��Ϣ
* @return       long.         ������
*/
int CSipStackMsgWrap::SetNwAddrInf( const string& strIP, const unsigned short usPort )
{
    if ( strIP.empty() )
    {
        return (int)SIP::RET_CODE_FAIL;
    }

    m_strIP = strIP;
    m_usPort = usPort;

    return (int)SIP::RET_CODE_OK;
}

/**
* Description:  SetlcAddrInf  ���ñ�����ַ��Ϣ
* @return       int.         ������
*/
int CSipStackMsgWrap::SetLcAddrInf( const string& strIP, const unsigned short usPort )
{
	if ( strIP.empty() )
	{
		return (int)SIP::RET_CODE_FAIL;
	}

	m_strLocalIP = strIP;
	m_usLocalPort = usPort;

	return (int)SIP::RET_CODE_OK;

}

/**
* Description:  SendMsg().  ������Ϣ
* @return       long.       ������
*/
long CSipStackMsgWrap::SendMsg()
{
    INFO_LOG("[ CSeq = %d ]. [MsgType=%d] Send SIP Request Message begin.",m_ulCSeq,m_stTextMsg.enTextMsgType);
    //BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
    //���п��ܷ���MiniSIP��������ڴ����������߳���
    CLockGuard objLocker(m_objSipStack.m_pThreadMutex);
    //END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�

    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //��Ϣ���ͷ�Χ���
    if (   m_stTextMsg.enTextMsgType <= SIP::TEXT_MSG_TYPE_SIP_MIN
        || m_stTextMsg.enTextMsgType >= SIP::TEXT_MSG_TYPE_SIP_MAX)
    {
        ERROR_LOG("Send SIP Message Failed. Text Message Type %d  unsupported.",m_stTextMsg.enTextMsgType);
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    //��ȡSIP������Ϣ����
    EN_SIP_METHOD enSipMethod = SIP::ARR_TEXT_TYPE_TO_SIP_METHOD[m_stTextMsg.enTextMsgType & ~SIP::TEXT_MSG_TYPE_MASK_SIP];

	INFO_LOG("[ MsgSeq = %d ] Send SIP Message Begin. SIP Method is %s. "
		"MsgBodyLen = %d. MsgHeaderLen = %d.", m_stTextMsg.ulMsgSeq, 
		SIP::STR_ARR_SIP_METHOD[enSipMethod], m_stTextMsg.ulMsgBodyLen, m_stTextMsg.ulMsgHeaderLen);
    INFO_LOG("[ CSeq = %d]. [MsgType=%d] after print.",m_ulCSeq,m_stTextMsg.enTextMsgType);

    SipMsgS* pstSipMsg = VOS_NULL;

    //����SIP������Ϣ
    lResult = CreateSipReqMsg(enSipMethod, pstSipMsg);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Create SIP Request Message Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }
    INFO_LOG("[ CSeq = %d]. [MsgType=%d] CreateSipReqMsg.",m_ulCSeq,m_stTextMsg.enTextMsgType);

    //���þ�����Ϣ�ض�ͷ��
    lResult = SetSpecialHeader(*pstSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Set Special SIP Header Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }
    INFO_LOG("[ CSeq = %d]. [MsgType=%d] SetSpecialHeader.",m_ulCSeq,m_stTextMsg.enTextMsgType);
    //���ÿɱ�ͷ��ͷ��
    lResult = SetVarHeader(*pstSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Set Variable Header Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }
    INFO_LOG("[ CSeq = %d]. [MsgType=%d] SetVarHeader.",m_ulCSeq,m_stTextMsg.enTextMsgType);
    //����SIP��Ϣ
    lResult = SendSipMsg(*pstSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Send SIP Message Failed.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //���ͳɹ�����
    PostSendSuccess();

    //������Ӧʱ���ͷŸ���Ϣ
    //SipDsmReleaseMsgRef(&stDataUnit.pstSipMsg);

    INFO_LOG("[ MsgSeq = %d] Send SIP Message Success.",m_stTextMsg.ulMsgSeq);
    return SIP::RET_CODE_OK;
}//lint !e1788

/**
* Description:  CreateSipReqMsg().  ����SIP������Ϣ
* @param  [in]  enSipMethod SIP��������
* @param  [out] pstSipMsg   SIP��Ϣָ��
* @return       long.       ������
*/
long CSipStackMsgWrap::CreateSipReqMsg
(
    EN_SIP_METHOD           enSipMethod, 
    SipMsgS*&               pstSipMsg
)
{

    INFO_LOG("[ MsgSeq = %d] Create SIP Request Message Begin.",m_stTextMsg.ulMsgSeq);
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //����SIP������Ϣ����������CSeqͷ��
    lResult = SIP::CreateReqMsg(enSipMethod, pstSipMsg);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create SIP Request Message Failed.");
        return lResult;
    }

    //����Request-URI
    lResult = SIP::SetRequestUri(*pstSipMsg, m_pszRequestUriName, m_pszRequestUriDomain);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Request-URI Failed On Create SIP Request Message.");
        return lResult;
    }

	//����Viaͷ��
	lResult = SIP::SetVia ( *pstSipMsg, m_strLocalIP.c_str(), m_usLocalPort );
	if (SIP::RET_CODE_OK != lResult)
	{
		ERROR_LOG("Set Via Header Failed On Create SIP Request Message.");
		return lResult;
	}

    //����Call-IDͷ��
    lResult = SIP::SetCallId(*pstSipMsg, m_strCallId);

    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Call-ID Header Failed On Create SIP Request Message.");
        return lResult;
    }


    //����Fromͷ��  
    //From DisplayName.
    if (  (SIP_METHOD_INVITE == pstSipMsg->pstSipMethod->usTokenId)
       || (SIP_METHOD_ACK == pstSipMsg->pstSipMethod->usTokenId)
       || (SIP_METHOD_SUBSCRIBE == pstSipMsg->pstSipMethod->usTokenId)
	   || (SIP_METHOD_REGISTER == pstSipMsg->pstSipMethod->usTokenId)
	   || (SIP_METHOD_MESSAGE == pstSipMsg->pstSipMethod->usTokenId)
       )
    {
		//����from�ֶ�ֵ
		std::string strUrl(m_objSipStack.m_stRegisterInfo.szLocalIp);
		strUrl.append(":");
		strUrl.append(m_objSipStack.m_stRegisterInfo.szLocalPort);
        lResult = SIP::SetFrom(*pstSipMsg, m_objSipStack.m_stRegisterInfo.szLocalId,strUrl.c_str());
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("Set From Header Failed On Create SIP Request Message.");
            return lResult;
        }
    }
    else
    {
        lResult = SIP::SetFrom(*pstSipMsg, m_objSipStack.m_stRegisterInfo.szLocalId, "huawei.com");
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("Set From Header Failed On Create SIP Request Message.");
            return lResult;
        }
    }

    //����Toͷ��    
	if(SIP_METHOD_REGISTER != enSipMethod)
    {
		lResult = SIP::SetTo(*pstSipMsg, m_pszRequestUriName,m_pszRequestUriDomain);
    }
	else//To��NameĬ����Request-URI��ͬ������REGISTER��Ϣ
	{
		//REGISTER��Ϣ��ֱ��ʹ�õ�¼��
		std::string strUrl(m_objSipStack.m_stRegisterInfo.szLocalIp);
		strUrl.append(":");
		strUrl.append(m_objSipStack.m_stRegisterInfo.szLocalPort);
		lResult = SIP::SetTo(*pstSipMsg, m_objSipStack.m_stRegisterInfo.szLocalId, strUrl.c_str());
	}

	if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set To Header Failed On Create SIP Request Message.");
        return lResult;
    }

    //����Max-Forwordsͷ��
    lResult = SIP::SetMaxForwards(*pstSipMsg, SIP::MAX_FORWARDS);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Max-Forwards Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    //����User-Agentͷ��
    if(  VOS_NULL         != m_pszUserAgentName
      && SIP::RET_CODE_OK != SIP::SetUserAgent(*pstSipMsg, m_pszUserAgentName, m_objSipStack.m_stRegisterInfo.szProductVersion)
      )
    {
        WARN_LOG("Add Max-Forwords Header Failed On Create SIP Request Message.");
    }

    if (  (SIP_METHOD_INVITE    != pstSipMsg->pstSipMethod->usTokenId)
       && (SIP_METHOD_SUBSCRIBE != pstSipMsg->pstSipMethod->usTokenId)
       && (SIP_METHOD_ACK       != pstSipMsg->pstSipMethod->usTokenId)
       && (SIP_METHOD_MESSAGE   != pstSipMsg->pstSipMethod->usTokenId)
       && SIP::RET_CODE_OK      != SIP::SetRoute(*pstSipMsg, m_objSipStack.m_objServerAddr, m_objSipStack.m_stRegisterInfo.szLoginDomain)
       )
    {
        WARN_LOG("Add Route Header Failed On Create SIP Request Message.");
    }

    INFO_LOG("[ MsgSeq = %d] Create SIP Request Message Success.",m_stTextMsg.ulMsgSeq);
    return SIP::RET_CODE_OK;
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrap::SetSpecialHeader
(
    SipMsgS& stSipMsg
)
{
    return SIP::RET_CODE_OK;
}//lint !e715 ���಻��ʹ�øñ���

/**
* Description:  SetVarHeader().     ���ÿɱ�ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrap::SetVarHeader
(
    SipMsgS& stSipMsg
)
{
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    if (0 == m_ulCSeq)
    {
        m_ulCSeq = m_objSipStack.GetCSeq();
    }

    INFO_LOG("[ MsgSeq = %d] to [ CSeq = %d ].",m_stTextMsg.ulMsgSeq,m_ulCSeq);

    //���ÿɱ�ͷ��
    lResult = SIP::ResetVarHdr(stSipMsg, m_ulCSeq, m_strFromTag, m_pszToTag);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ CSeq = %d ] Reset Variable Headers Failed On Create SIP Request Message.",m_ulCSeq);
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    if (!m_objSipStack.m_stPlatAuthInfo.strRealm.empty())
    {
        //TODO   ���й��ˣ���ֹ������Ϣ�������ͼ�Ȩ��Ϣ
        if ((SIP_METHOD_REGISTER ==
            (SIP::ARR_TEXT_TYPE_TO_SIP_METHOD[m_stTextMsg.enTextMsgType & ~SIP::TEXT_MSG_TYPE_MASK_SIP]))
            || (SIP_METHOD_OPTIONS ==
            (SIP::ARR_TEXT_TYPE_TO_SIP_METHOD[m_stTextMsg.enTextMsgType & ~SIP::TEXT_MSG_TYPE_MASK_SIP])))
        {
            //����Authorizationͷ��
            lResult = SIP::SetAuthorization(stSipMsg,
                m_objSipStack.m_stRegisterInfo.szLoginName,
                m_objSipStack.m_stRegisterInfo.szPassword,
                m_objSipStack.m_stPlatAuthInfo);
            if (SIP::RET_CODE_OK != lResult)
            {
                ERROR_LOG("[ CSeq = %d] Add Authorization Header Failed On Create SIP Request Message.",m_ulCSeq);
                return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
            }
        }
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  SendSipMsg().   ����SIP��Ϣ
* @param  [in]  stSipMsg   SIP��Ϣ����
* @param  [in]  objServerAddr    ����SIP��Ϣ��������ַ  // add by w00207027 2012-11-02
* @return       long.       ������
*/
long CSipStackMsgWrap::SendSipMsg
(
    SipMsgS& stSipMsg
    //CNetworkAddr& objServerAddr
)
{
    INFO_LOG("[ CSeq = %d]. [MsgType=%d] Send SIP Request Message begin.",m_ulCSeq,m_stTextMsg.enTextMsgType);
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //����SIP��Ϣ��Ϣ�ṹ�壬���캯�����ʼ��
    CSipStack::SIP_MSG_INFO* pstSipMsgInfo = VOS_NEW(pstSipMsgInfo);
    if (NULL == pstSipMsgInfo)
    {
        ERROR_LOG("[ CSeq = %d] Create SIP Message Info Failed On Save Message Info.",m_ulCSeq);
        return SIP::RET_CODE_FAIL;
    }

    pstSipMsgInfo->ulMsgSeq         = m_stTextMsg.ulMsgSeq;         //�ϲ���Ϣ���
    pstSipMsgInfo->enTextMsgType    = m_stTextMsg.enTextMsgType;    //�ı���Ϣ����
    pstSipMsgInfo->pstSipMsg        = &stSipMsg;                    //SIP��Ϣָ��
    pstSipMsgInfo->ulDialogId       = m_stTextMsg.ulDialogId;       //INVITE����ĶԻ�ID

    (void)VOS_MutexLock(m_objSipStack.m_pMapMutex);
    //����SIP��Ϣ����Ϣ
    m_objSipStack.m_mapCSeqToSipMsgInfo[m_ulCSeq] = pstSipMsgInfo;
    (void)VOS_MutexUnlock(m_objSipStack.m_pMapMutex);

    //������Ϣ��
    SipDataUnitS stDataUnit = {0};

    stDataUnit.pstSipMsg = &stSipMsg;

    //����MiniSIP�ķ��ͽӿ�
    /////////////////////////////  modified by w00207027 2012-11-12 Begin //////////////////////////////////
    lResult = SipTxnHiSend(stDataUnit, pstSipMsgInfo->ulTxnObjId, pstSipMsgInfo->ulTuObjId);//lint !e838
    //lResult = SipTxnHiSend(stDataUnit, objServerAddr, pstSipMsgInfo->ulTxnObjId, pstSipMsgInfo->ulTuObjId);
    /////////////////////////////  modified by w00207027 2012-11-12 End //////////////////////////////////    
    // �������ʧ�ܣ��ʹ�m_mapCSeqToSipMsgInfo��ɾ������Ϣ
    if (SIP::RET_CODE_OK != lResult)
    {
        (void)VOS_MutexLock(m_objSipStack.m_pMapMutex);
        //��map����ɾ������
        (void)m_objSipStack.m_mapCSeqToSipMsgInfo.erase(m_ulCSeq);
        (void)VOS_MutexUnlock(m_objSipStack.m_pMapMutex);
        //�ͷ��ڴ�
        VOS_DELETE(pstSipMsgInfo);

        ERROR_LOG("[ CSeq = %d ] SipTxnHiSend Failed On Send SIP Message.",m_ulCSeq);
        return SIP::RET_CODE_FAIL;
    }

    INFO_LOG("[ CSeq = %d]. [MsgType=%d] Send SIP Request Message Success.",m_ulCSeq,pstSipMsgInfo->enTextMsgType);
    return SIP::RET_CODE_OK;
}

/**
* Description:  SendSipReqMsgWithVarHdr().  ���ʹ��ɱ�ͷ���SIP������Ϣ
* @param  [in]  stSipMsg    SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrap::SendSipReqMsgWithVarHdr
(
    SipMsgS& stSipMsg
)
{
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //���ÿɱ�ͷ��ͷ��
    lResult = SetVarHeader(stSipMsg);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Set Variable Header Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //����SIP��Ϣ
    lResult = SendSipMsg(stSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Send SIP Message Failed.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  SipTxnHiSend().  ����MiniSIP�ķ��ͽӿ�
* @param  [in]  stSipSdu             SIP����Ϣ���ݵ�Ԫ
* @param  [in]  objServerAddr    ����SIP��Ϣ��������ַ  // add by w00207027 2012-11-02
* @param  [out] ulTuObjId            ��Ӧ��TU����ID
* @param  [out] ulTxnObjId          ��Ӧ���������ID
* @return       long.       ������
*/
long CSipStackMsgWrap::SipTxnHiSend
(
    SipDataUnitS&   stSipSdu,
    //CNetworkAddr& objServerAddr,
    SS_UINT32&      ulTxnObjId,
    SS_UINT32&      ulTuObjId
)
{
    DEBUG_LOG("sip txn hi send...begin");
    //INFO_LOG("[ CSeq = %d ] SipTxnHiSfSendReq Start. MsgType=%d, RequestUriName = %s. CallID = %s. FromTag = %s. ToTag = %s.", 
    //    m_ulCSeq, m_stTextMsg.enTextMsgType, m_pszRequestUriName, m_strCallId.c_str(), m_pszFromTag, m_pszToTag); 

    //����������Ϣ
    SipTptNwInfoS stTptNwInfo = {0};
    /////////////////////////////  modified by w00207027 2012-11-12 Begin //////////////////////////////////
    //SIP::SetSipTptNwInfo(stTptNwInfo, m_objSipStack.m_objServerAddr, m_objSipStack.m_objLocalAddr);
    if (m_objSipStack.m_bServerAddrEx)
    {
        SIP::SetSipTptNwInfo(stTptNwInfo, m_objSipStack.m_objServerAddrEx, m_objSipStack.m_objLocalAddr);		
    } 
    else
    {
        SIP::SetSipTptNwInfo(stTptNwInfo, m_objSipStack.m_objServerAddr, m_objSipStack.m_objLocalAddr);
    }	
    /////////////////////////////  modified by w00207027 2012-11-12 End //////////////////////////////////

    ulTuObjId = m_objSipStack.GetTuObjId();

    //������״̬������Ϣ
	DEBUG_LOG("call minisip to send request message......dst_addr=%u.%u.%u.%u:%u, type=%d, cseq=%d, call_id=%s, from_tag=%s, to_tag=%s",
		(int)stTptNwInfo.stDstAddr.u.aucIPv4Addr[0],
		(int)stTptNwInfo.stDstAddr.u.aucIPv4Addr[1],
		(int)stTptNwInfo.stDstAddr.u.aucIPv4Addr[2],
		(int)stTptNwInfo.stDstAddr.u.aucIPv4Addr[3],
		(int)stTptNwInfo.stDstAddr.iPort,
		m_stTextMsg.enTextMsgType,
		m_ulCSeq,
		m_strCallId.c_str(),
		m_strFromTag.c_str(),
		m_pszToTag);

	EN_SIP_RESULT enResult = SipTxnHiSfSendReq(ulTuObjId, &stSipSdu, &stTptNwInfo, NULL, &ulTxnObjId, NULL);
    if (SIP_RET_SUCCESS != enResult)
    {        
        DEBUG_LOG("sip txn hi send...failure to send,error= %s",SIP::STR_ARR_EN_SIP_RESULT[enResult]);
        return SIP::RET_CODE_FAIL;
    }

    DEBUG_LOG("sip txn hi send...success");
    return SIP::RET_CODE_OK;
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrap::PostSendSuccess()
{
}

/**
* Description:    CSipMsgRegister(). Default constructor
*/
CSipStackMsgWrapRigister::CSipStackMsgWrapRigister(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, const string& callID) 
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{
    //Register��Request-URI����ҪName Info
    // m_pszRequestUriName = VOS_NULL; //Request-URI��Name
    //      if (VOS_NULL != m_stTextMsg.pUserData)
    //     {
    //         SIP::PEER_URI_INFO& stPeerUriInfo = *(SIP::PEER_URI_INFO*)m_stTextMsg.pUserData;
    // 
    //         m_pszRequestUriName     = stPeerUriInfo.szUriUserName;
    //         m_pszRequestUriDomain   = stPeerUriInfo.szUriHostName;
    //         m_pszUserAgentName      = stPeerUriInfo.szUserAgentName;
    //     }

    //ֱ��ʹ���ϴ�ע��ʱ��CallID����һ��ע��ʱ��ֵΪ��
    // m_strCallId = objSipStack.m_strRegisterCallId;

    m_ulExpireTime = objSipStack.m_stRegisterInfo.ulExpireTime;   //��ʱʱ��
	if ( !callID.empty() )
	{
		m_strCallId = callID;
	}
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapRigister::SetSpecialHeader
(
    SipMsgS&                stSipMsg
)
{
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //����Ϊ��ע��
    m_objSipStack.m_bRegister = VOS_TRUE;

    //����Contactͷ��
    lResult = SIP::SetContact(stSipMsg, m_objSipStack.m_objLocalAddr, m_objSipStack.m_stRegisterInfo.szLocalId);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Contact Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    //����Expiresͷ��
    lResult = SIP::SetExpires(stSipMsg, m_ulExpireTime);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Expires Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapRigister::PostSendSuccess()
{
    //��һ��ע��ʱ����CallID
    if (m_objSipStack.m_strRegisterCallId.empty())
    {
        m_objSipStack.m_strRegisterCallId = m_strCallId;
    }
}

/**
* Description:    CSipMsgUnRegister(). Default constructor
*/
CSipStackMsgWrapUnRigister::CSipStackMsgWrapUnRigister(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg,const string& strCallID) 
    :CSipStackMsgWrapRigister(objSipStack, stTextMsg,strCallID)
{
    //ע��ʱ��ʱʱ��Ϊ0
    m_ulExpireTime = 0; //��ʱʱ��
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapUnRigister::PostSendSuccess()
{
    //����Ϊ��ע��
    m_objSipStack.m_bRegister = VOS_FALSE;

    //����������Ϣ��Ϣ
    m_objSipStack.ClearAllMsgInfo();

    WARN_LOG("[ MsgSeq = %d ] Have Already Terminated All SIP Transaction.",m_stTextMsg.ulMsgSeq); 
}

/**
* Description:    CSipMsgMessage(). Default constructor
*/
CSipStackMsgWrapMessage::CSipStackMsgWrapMessage(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, const string& callID) 
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{
    //��������Request-URI�����ֲ���
    //     if (VOS_NULL != m_stTextMsg.pMsgHeader && 0 < m_stTextMsg.ulMsgHeaderLen)
    //     {
    //         m_pszRequestUriName = (char*)m_stTextMsg.pMsgHeader;
    //     }

    //     if (VOS_NULL != m_stTextMsg.pUserData)
    //     {
    //         SIP::PEER_URI_INFO& stPeerUriInfo = *(SIP::PEER_URI_INFO*)m_stTextMsg.pUserData;
    // 
    //         m_pszRequestUriName     = stPeerUriInfo.szUriUserName;
    //         m_pszRequestUriDomain   = stPeerUriInfo.szUriHostName;
    //         m_pszUserAgentName      = stPeerUriInfo.szUserAgentName;
    //     }

    INFO_LOG("Create SIP Message. Request URI = %s@%s, UserAgent = %s",m_pszRequestUriName,m_pszRequestUriDomain,m_pszUserAgentName);
	if (!callID.empty() )
	{
		m_strCallId = callID;
	}
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapMessage::SetSpecialHeader
(
    SipMsgS&                stSipMsg
)
{
    //modify-123456789-begin 
    m_objSipStack.m_bRegister = VOS_TRUE;
    //long lResult = SIP::SetContentType()
    long lResult = SIP::SetContentEncoding(stSipMsg, "utf-8");
    if ( SIP::RET_CODE_OK != lResult )
    {
        return SIP::RET_CODE_FAIL;
    }

    lResult = SIP::SetServer(stSipMsg, "HUAWEI eSDK/1.0");
    if ( SIP::RET_CODE_OK != lResult )
    {
        return SIP::RET_CODE_FAIL;
    }
    //modify-123456789-end

    //������Ϣ��
    //long lResult = SIP::RET_CODE_FAIL;
    //if (m_objSipStack.IsServerAddrEx())
    //{
    //    lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_T28181_XML, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    //}
    //else
    //{
    //    lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_XML, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    //}
	lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_T28181_SUBSCRIBE_XML, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Message Body Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapMessage::PostSendSuccess()
{
	return;
}

/***********************************************************************************/
/**
* Description:    CSipMsgMessage(). Default constructor
*/

CSipStackMsgWrapOptions::CSipStackMsgWrapOptions(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg) 
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{
    //��������Request-URI�����ֲ���
    //if (VOS_NULL != m_stTextMsg.pMsgHeader && 0 < m_stTextMsg.ulMsgHeaderLen)
    //{
    //    m_pszRequestUriName = (char*)m_stTextMsg.pMsgHeader;
    //}

    //     if (VOS_NULL != m_stTextMsg.pUserData)
    //     {
    //         SIP::PEER_URI_INFO& stPeerUriInfo = *(SIP::PEER_URI_INFO*)m_stTextMsg.pUserData;
    // 
    //         m_pszRequestUriName     = stPeerUriInfo.szUriUserName;
    //         m_pszRequestUriDomain   = stPeerUriInfo.szUriHostName;
    //         m_pszUserAgentName      = stPeerUriInfo.szUserAgentName;
    //     }

    INFO_LOG("Create SIP Message. Request URI = %s@%s, userAgentName = %s",m_pszRequestUriName,m_pszRequestUriDomain,m_pszUserAgentName);
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapOptions::PostSendSuccess()
{
	return;
}

/***********************************************************************************/
/**
* Description:    CSipMsgInvite(). Default constructor
*/
CSipStackMsgWrapInvite::CSipStackMsgWrapInvite ( CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, const string& callID ) 
    :CSipStackMsgWrap ( objSipStack, stTextMsg )
{
    //INVITE������ͷ�����
    memset(&m_stReqHdrInvite, 0, sizeof(m_stReqHdrInvite));

    if (NULL == m_stTextMsg.pMsgHeader)
    {
        ERROR_LOG("Set Special Header Failed On Create Invite Message. Message Header Pointer is Empty.");
        return ;
    }

    if (sizeof(SIP::REQ_HEADER_INVITE) != m_stTextMsg.ulMsgHeaderLen)
    {
        ERROR_LOG("Set Special Header Failed On Create Invite Message. Message Header Length(%d) Must be %d.",m_stTextMsg.ulMsgHeaderLen,sizeof(SIP::REQ_HEADER_INVITE));
        return ;
    }

    //����ͷ�����
    m_stReqHdrInvite = *(SIP::REQ_HEADER_INVITE*)m_stTextMsg.pMsgHeader;


    //ǿ�Ƽ����ַ���������
    m_stReqHdrInvite.szRequestUri[sizeof(m_stReqHdrInvite.szRequestUri) - 1] = 0;

    //��������Request-URI�����ֲ���
    // m_pszRequestUriName = m_stReqHdrInvite.szRequestUri;

    //ǿ�Ƽ����ַ���������
    m_stReqHdrInvite.szSubject[sizeof(m_stReqHdrInvite.szSubject) - 1] = 0;

    if ( !callID.empty() )
    {
        m_strCallId = callID;
    }
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapInvite::SetSpecialHeader
    (
    SipMsgS&                stSipMsg
    )
{
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //modify-123456789-begin
    m_objSipStack.m_bRegister = VOS_TRUE;
    //modify-123456789-end

    //ǿ�Ƽ����ַ���������
    m_stReqHdrInvite.szSubject[sizeof(m_stReqHdrInvite.szSubject) - 1] = 0;


	//ȥ��subject�ֶΣ���ֹ������У��ʧ��
    //����Subjectͷ��
    //lResult = SIP::SetSubject(stSipMsg, m_stReqHdrInvite.szSubject);
    //if (SIP::RET_CODE_OK != lResult)
    //{
    //    ERROR_LOG("Set Message Body Failed On Create SIP Request Message.";
    //    return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    //}

    //����Contactͷ��
    lResult = SIP::SetContact(stSipMsg, m_objSipStack.m_objLocalAddr, m_objSipStack.m_stRegisterInfo.szLocalId);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Contact Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    //����Contactͷ��
    lResult = SIP::SetServer(stSipMsg, "HUAWEI eSDK/1.0");
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Contact Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    ERROR_LOG("Set Message Body Failed On Create SIP Request Message[len:%d,body:%s].",m_stTextMsg.ulMsgBodyLen ,m_stTextMsg.pszMsgBody);
    //������Ϣ��
    lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_SDP, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Message Body Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    return SIP::RET_CODE_OK;
}

long CSipStackMsgWrapInvite::SendMsg()
{
    //BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
    //���п��ܷ���MiniSIP��������ڴ����������߳���
    CLockGuard objLocker(m_objSipStack.m_pThreadMutex);
    //END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�

    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //��Ϣ���ͷ�Χ���
    if (   m_stTextMsg.enTextMsgType <= SIP::TEXT_MSG_TYPE_SIP_MIN
        || m_stTextMsg.enTextMsgType >= SIP::TEXT_MSG_TYPE_SIP_MAX)
    {
        ERROR_LOG("Send SIP Message Failed. Text Message Type(%d) unsupported.",m_stTextMsg.enTextMsgType);
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    //��ȡSIP������Ϣ����
    EN_SIP_METHOD enSipMethod = SIP::ARR_TEXT_TYPE_TO_SIP_METHOD[m_stTextMsg.enTextMsgType & ~SIP::TEXT_MSG_TYPE_MASK_SIP];

	INFO_LOG("[ MsgSeq = %d ] Send SIP Message Begin. SIP Method is %s. "
		"MsgBodyLen = %d. MsgHeaderLen = %d.", m_stTextMsg.ulMsgSeq, 
		SIP::STR_ARR_SIP_METHOD[enSipMethod], m_stTextMsg.ulMsgBodyLen, m_stTextMsg.ulMsgHeaderLen);

    SipMsgS* pstSipMsg = VOS_NULL;

    //����SIP������Ϣ
    lResult = CreateSipReqMsg(enSipMethod, pstSipMsg);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Create SIP Request Message Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //���þ�����Ϣ�ض�ͷ��
    lResult = SetSpecialHeader(*pstSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Set Special SIP Header Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //���ÿɱ�ͷ��ͷ��
    lResult = SetVarHeader(*pstSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Set Variable Header Failed On Send SIP Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //����SIP��Ϣ
    lResult = SendSipMsg(*pstSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Send SIP Message Failed.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //���ͳɹ�����
    PostSendSuccess();

    //������Ӧʱ���ͷŸ���Ϣ
    //SipDsmReleaseMsgRef(&stDataUnit.pstSipMsg);

    INFO_LOG("[ MsgSeq = %d] Send SIP Message Success.",m_stTextMsg.ulMsgSeq);
    return SIP::RET_CODE_OK;
}//lint !e1788

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapInvite::PostSendSuccess()
{
    //���캯���г�ʼ��
    SIP::DIALOG_INFO stDialogInfo;

    stDialogInfo.ulCSeq     = m_ulCSeq;
    stDialogInfo.strCallId  = m_strCallId;

    //����request uri
    stDialogInfo.strRequestUriName = m_pszRequestUriName;
    stDialogInfo.strRequestUriDomain = m_pszRequestUriDomain;

    (void)VOS_MutexLock(m_objSipStack.m_pMapMutex);
    m_objSipStack.m_mapDialogIdToInfo[m_stTextMsg.ulDialogId] = stDialogInfo;
    (void)VOS_MutexUnlock(m_objSipStack.m_pMapMutex);
}

/**
* Description:    CSipMsgBye(). Default constructor
*/
CSipStackMsgWrapBye::CSipStackMsgWrapBye(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg)
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{

    m_ulTxnObjId    = 0;    //��������ID
    m_ulTuObjId     = 0;    //�����û�����ID


    m_bCancel   = VOS_FALSE;    //�Ƿ�ΪCancel��Ϣ
}

/**
* Description:  SendMsg().  ������Ϣ
* @return       long.       ������
*/
long CSipStackMsgWrapBye::SendMsg()
{
    INFO_LOG("[ MsgSeq = %d] Send BYE Message Begin.",m_stTextMsg.ulMsgSeq);

    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //���캯���г�ʼ��
    SIP::DIALOG_INFO stDialogInfo;

    lResult = m_objSipStack.GetDialogInfoById(m_stTextMsg.ulDialogId, stDialogInfo);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create SIP BYE Request Message Failed. Dialog ID(%d) Not Exist.",m_stTextMsg.ulDialogId);
        return SIP::RET_CODE_FAIL;
    }

    //SCC��������INVITEʱ���趯̬������Щֵ
    m_strCallId = stDialogInfo.strCallId;

    //�ǿ�ʱ�����á�SCC��������INVITEʱ����ֵΪ��
    if (!stDialogInfo.strRequestUriName.empty())
    {
        //���� requesturi,��invite��Ϣ�л�ȡ
        m_pszRequestUriName = stDialogInfo.strRequestUriName.c_str();
    }

    //�ѽ��յ�200 OK��Ӧ��ֱ�ӷ���BYE����
    if (stDialogInfo.bRecvOK)
    {
        //ʹ��INVITE��200 OK��From��To��Tag
        m_strFromTag    = stDialogInfo.strLocalTag;     //From��Tag
        m_pszToTag      = stDialogInfo.strRemoteTag.c_str();    //To��Tag

        lResult = CSipStackMsgWrap::SendMsg();
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("[ MsgSeq = %d ] Send BYE Message Failed.",m_stTextMsg.ulMsgSeq);
            return SIP::RET_CODE_FAIL;
        }

        INFO_LOG("[ MsgSeq = %d] Send BYE Message Success.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_OK;
    }

    INFO_LOG("[ MsgSeq = %d] Send BYE Message In. BYE Message Convert To CANCEL Message.",m_stTextMsg.ulMsgSeq);

    //����ΪCancel��Ϣ
    m_bCancel = VOS_TRUE;

    //ʹ��INVITE��CSeq
    m_ulCSeq = stDialogInfo.ulCSeq;

    CSipStack::SIP_MSG_INFO* pstMsgInfo = VOS_NULL;

    //��ȡSIP��Ϣ��Ϣ
    lResult = m_objSipStack.GetSipMsgInfoByCSeq(m_ulCSeq, pstMsgInfo);
    if (SIP::RET_CODE_OK != lResult)
    {
        WARN_LOG("[ MsgSeq = %d] Send CANCEL Message Failed. Invite Message Not Exist.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_FAIL;
    }

    //ʹ��INVITE��TU��TXN�Ķ���ID
    m_ulTxnObjId    = pstMsgInfo->ulTxnObjId;
    m_ulTuObjId     = pstMsgInfo->ulTuObjId;

    //��ȡINVITE��From��Tag
    lResult = SIP::GetFromTag(*pstMsgInfo->pstSipMsg, stDialogInfo.strLocalTag);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Get From Tag Failed On Send CANCEL Message.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_FAIL;
    }

    //ʹ��INVITE��From��Tag��ToTag
    m_strFromTag    = stDialogInfo.strLocalTag;   //From��Tag

    lResult = CSipStackMsgWrap::SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Send CANCEL Message Failed.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_FAIL;
    }

    INFO_LOG("[ MsgSeq = %d] Send CANCEL Message Success.",m_stTextMsg.ulMsgSeq);
    return SIP::RET_CODE_OK;
}

/**
* Description:  SipTxnHiSend().  ����MiniSIP�ķ��ͽӿ�
* @param  [in]  stSipSdu    SIP����Ϣ���ݵ�Ԫ
* @param  [in]  objServerAddr    ����SIP��Ϣ��������ַ  // add by w00207027 2012-11-02
* @param  [out] ulTuObjId   ��Ӧ��TU����ID
* @param  [out] ulTxnObjId  ��Ӧ���������ID
* @return       long.       ������
*/
long CSipStackMsgWrapBye::SipTxnHiSend
(
    SipDataUnitS&   stSipSdu,
    //CNetworkAddr& objServerAddr,
    SS_UINT32&      ulTxnObjId,
    SS_UINT32&      ulTuObjId
)
{
    //BYE��Ϣ��ֱ�ӵ��ø���ķ��ͽӿ�
    if (!m_bCancel)
    {
        /////////////////////////////  modified by w00207027 2012-11-12 Begin //////////////////////////////////
        return CSipStackMsgWrap::SipTxnHiSend(stSipSdu, ulTxnObjId, ulTuObjId);
        //return CSipMsg::SipTxnHiSend(stSipSdu, objServerAddr, ulTxnObjId, ulTuObjId);
        /////////////////////////////  modified by w00207027 2012-11-12 End //////////////////////////////////
    }

    //�޸����󷽷�ΪCANCEL
    stSipSdu.pstSipMsg->uFirstLine.stRequestLine.stMethod.usTokenId = SIP_METHOD_CANCEL;
    stSipSdu.pstSipMsg->stHeaders.pstCseq->stMethod.usTokenId = SIP_METHOD_CANCEL;

    ulTuObjId = m_objSipStack.GetTuObjId();

    //������״̬������Ϣ
    EN_SIP_RESULT enResult = SipTxnHiSfSendCancel(m_ulTxnObjId,
        m_ulTuObjId,
        ulTuObjId,
        &stSipSdu,
        NULL,
        &ulTxnObjId);
    if (SIP_RET_SUCCESS != enResult)
    {        
        ERROR_LOG("[ CSeq = %d ] SipTxnHiSfSendCancel Failed On Send CANCEL Message. Error = %s",m_ulCSeq,SIP::STR_ARR_EN_SIP_RESULT[enResult]);
        return SIP::RET_CODE_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapBye::PostSendSuccess()
{
    //������
    //CLockGuard objLocker(m_objSipStack.m_pMapMutex);

    //��öԻ���Ϣ�Ŀ���
    SIP::DIALOG_INFO stDialogInfo = m_objSipStack.m_mapDialogIdToInfo[m_stTextMsg.ulDialogId];

    unsigned long ulDialogId = 0;
    //��m_mapDialogInfoToId��ɾ����ӳ�䣬keyֵΪ�ṹ�岻�ܼ���erase�Ƴ�
    //���л�ͬʱm_mapDialogIdToInfoɾ����ӳ��
    (void)m_objSipStack.GetDialogIdByInfo(stDialogInfo, ulDialogId, VOS_TRUE);   
}


/**
* Description:    CSipStackMsgWrapInviteResp(). Default constructor
* @param  [in]  objSipStack     SipStack����
* @param  [in]  stSipReqMsg     SIP������Ϣ����
* @param  [in]  ulDialogId          �Ի�ID
* @param  [in]  szMsgBody       ��Ϣ���ַ���
* @param  [in]  ulMsgBodyLen    ��Ϣ�峤��
*/
CSipStackMsgWrapInviteResp::CSipStackMsgWrapInviteResp
(
    CSipStack&      objSipStack, 
    SipMsgS&        stSipReqMsg,
    unsigned long   ulDialogId,
    unsigned long   ulMsgBodyLen,
    char*           pszMsgBody
) : CSipStackMsgWrapResp(objSipStack, stSipReqMsg),
    m_ulDialogId(ulDialogId),
    m_ulMsgBodyLen(ulMsgBodyLen),
    m_pszMsgBody(pszMsgBody)
{
}

/**
* Description:    ~CSipRspMsgInvite(). Destructor
*/
CSipStackMsgWrapInviteResp::~CSipStackMsgWrapInviteResp()
{
    m_pszMsgBody    = VOS_NULL; //��Ϣ������
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stRspSipMsg     SIP��Ӧ��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapInviteResp::SetSpecialHeader
(
    SipMsgS& stRspSipMsg
)
{
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //����Contactͷ��
    lResult = SIP::SetContact(stRspSipMsg, m_objSipStack.m_objLocalAddr, m_objSipStack.m_stRegisterInfo.szLocalId);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Contact Header Failed On Send Invite Response Message.");
        return SIP::RET_CODE_FAIL;
    }

    //�Ի�ID��Ч������ʧ����Ӧ
    // modified by dhong
    //     if (SIP::INVALID_DIALOG_ID == m_ulDialogId)
    //     {
    //         WARN_LOG("Create INVITE Error Response Message Success On Send Invite Response Message.");
    //         return SIP::RET_CODE_OK;
    //     }

    //�ɹ���Ӧ������SDP    
    if (VOS_NULL == m_pszMsgBody || 0 == m_ulMsgBodyLen)
    {
		ERROR_LOG("Set Message Body Failed On Send Invite Response Message.");
        return SIP::RET_CODE_FAIL;
    }

    //������Ϣ��
    lResult = SIP::SetMsgBody(stRspSipMsg, SIP::SUB_MEDIA_TYPE_SDP, m_ulMsgBodyLen, m_pszMsgBody);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Message Body Failed On Send Invite Response Message.");
        return SIP::RET_CODE_FAIL;
    }

    //���캯���г�ʼ��
    SIP::DIALOG_INFO stDialogInfo;

    //��ȡ�Ի���Ϣ��������Ϣ����Զ��
    lResult = SIP::GetDialogInfo(stRspSipMsg, stDialogInfo, VOS_FALSE);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Dialog Info Failed On Send Invite Response Message.");
        return SIP::RET_CODE_FAIL;
    }

    //�����ڷ�����Ϣǰ��ӵ�ӳ����У�������Ӧ��Ϣ����ͻ��Ҳ����öԻ�
    //���������ӳ���֮�����ã����ӳ��ʱ���п���
    stDialogInfo.bRecvOK = VOS_TRUE;

    (void)VOS_MutexLock(m_objSipStack.m_pMapMutex);
    //��ӶԻ�ID��Ϣ��ӳ��
    // modified by dhong 
    // because find invite has no dialogID
    if (0 != m_ulDialogId)
    {
        m_objSipStack.m_mapDialogInfoToId[stDialogInfo] = m_ulDialogId;
        m_objSipStack.m_mapDialogIdToInfo[m_ulDialogId] = stDialogInfo;
    }
    (void)VOS_MutexUnlock(m_objSipStack.m_pMapMutex);

	INFO_LOG("Create INVITE Response Message Success. "
		"DialogID = %#p. Call-ID = %s. RemoteTag = %s. LocalTag = %s.", 
		m_ulDialogId, stDialogInfo.strCallId.c_str(),
		stDialogInfo.strRemoteTag.c_str(), stDialogInfo.strLocalTag.c_str());

    return SIP::RET_CODE_OK;
}


/**
* Description:    CSipMsgInviteResp(). Default constructor
*/
CSipStackMsgWrapInviteRespEx::CSipStackMsgWrapInviteRespEx(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg, unsigned long retCode)
                             :CSipStackMsgWrap(objSipStack, stTextMsg), m_retCode(retCode)
{
}

/**
* Description:  SendMsg().  ������Ϣ
* @return       long.       ������
*/
long CSipStackMsgWrapInviteRespEx::SendMsg()
{
    INFO_LOG("[ CSeq = %d] Send INVITE Response Message Begin. DialogID = %d",m_stTextMsg.ulMsgSeq,m_stTextMsg.ulDialogId);

    if (VOS_NULL == m_stTextMsg.pUserData)
    {
        ERROR_LOG("[ CSeq = %d ] Send INVITE Response Message Failed. User Data is Empty.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    //��Ӧ��
    //  unsigned long ulRspCode = SIP_STATUS_SUCC_OK;
    //     if (SIP::INVALID_DIALOG_ID == m_stTextMsg.ulDialogId)
    //     {
    //         ulRspCode = SIP_STATUS_SERVER_ERR_SERVICE_UNAVAILABLE;
    //     }


    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    SIP::REQ_MSG_INFO* pstReqMsgInfo = (SIP::REQ_MSG_INFO*)m_stTextMsg.pUserData;


    //     if (SIP::INVALID_DIALOG_ID == pstReqMsgInfo->ulDialogId)
    //     {
    //         ulRspCode = SIP_STATUS_SERVER_ERR_SERVICE_UNAVAILABLE;
    //     }

    //����INVITE����Ӧ��Ϣ
    CSipStackMsgWrapInviteResp objSipRspMsgInvite( m_objSipStack, 
                                                   *pstReqMsgInfo->pstSipReqMsg,
                                                   m_stTextMsg.ulDialogId,
                                                   m_stTextMsg.ulMsgBodyLen,
                                                   m_stTextMsg.pszMsgBody
                                                 );

    //������Ӧ��Ϣ
    //     lResult = objSipRspMsgInvite.SendRspMsg(ulRspCode, 
    //                                             pstReqMsgInfo->ulTuObjId, 
    //                                             pstReqMsgInfo->ulTxnObjId);
    lResult = objSipRspMsgInvite.SendRspMsg( m_retCode, 
                                             pstReqMsgInfo->ulTuObjId, 
                                             pstReqMsgInfo->ulTxnObjId
                                           );//lint !e838

    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ CSeq = %d] Send Response Failed On Send INVITE Response Message.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //���ͳɹ����ͷ���Դ������������ϲ����ط�
    // ��ʱ���ӣ�������� dhong
    SipDsmReleaseMsgRef(&(pstReqMsgInfo->pstSipReqMsg));
    VOS_DELETE(pstReqMsgInfo);

    INFO_LOG("[ CSeq = %d] Send INVITE Response Message Success.",m_stTextMsg.ulMsgSeq);
    return SIP::RET_CODE_OK;
}

/**
* Description:    CSipMsgRedirect(). Default constructor
*/
CSipStackMsgWrapRedirect::CSipStackMsgWrapRedirect(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg)
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{
}

/**
* Description:  SendMsg().  ������Ϣ
* @return       long.       ������
*/
long CSipStackMsgWrapRedirect::SendMsg()
{
    if (VOS_NULL == m_stTextMsg.pUserData)
    {
        ERROR_LOG("[ MsgSeq = %d ] Send SIP Redirect Message Failed. User Data Is Empty.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_FAIL;
    }

    SipMsgS& stSipMsg = *(SipMsgS*)m_stTextMsg.pUserData;

    //��������Routeͷ��
    long lResult = SIP::SetRoute(stSipMsg, m_objSipStack.m_objServerAddr, m_objSipStack.m_stRegisterInfo.szLoginDomain);
    //��ѡͷ��ʧ�ܽ���ӡ�澯��־
    if (SIP::RET_CODE_OK != lResult)
    {
        WARN_LOG("Add Route Header Failed On Handle Redirect Event.");
    }

    //���ʹ��ɱ�ͷ���SIP������Ϣ
    lResult = SendSipReqMsgWithVarHdr(stSipMsg);

    return lResult;    
}

/**
* Description:    CSipMsgReAuth(). Default constructor
*/
CSipStackMsgWrapReAuth::CSipStackMsgWrapReAuth(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg)
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{
}

/**
* Description:  SendMsg().  ������Ϣ
* @return       long.       ������
*/
long CSipStackMsgWrapReAuth::SendMsg()
{
    if (VOS_NULL == m_stTextMsg.pUserData)
    {
        ERROR_LOG("[ MsgSeq = %d ] Send SIP Request Message With Authentication Failed. User Data Is Empty.",m_stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_FAIL;
    }

    SipMsgS& stSipMsg = *(SipMsgS*)m_stTextMsg.pUserData;

    string strFromTag;

    long lResult = SIP::GetFromTag(stSipMsg, strFromTag);

    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Send SIP Request Message With Authentication Failed. Get From Tag Failed.",m_stTextMsg.ulMsgSeq);
        return lResult;
    }

    //ʹ��ԭ��Ϣ��FromTag
    m_strFromTag = strFromTag;

    //���ʹ��ɱ�ͷ���SIP������Ϣ
    lResult = SendSipReqMsgWithVarHdr(stSipMsg);

    return lResult;
}


/**
* Description:    CSipMsgAck(). Default constructor
*/
CSipStackMsgWrapAck::CSipStackMsgWrapAck(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg)
    :CSipStackMsgWrap(objSipStack, stTextMsg)
{
    //���öԻ������Ϣ
    if (VOS_NULL == m_stTextMsg.pUserData)
    {
        ERROR_LOG("[ MsgSeq = %d ] Create ACK Message Failed. User Data Is Empty.",stTextMsg.ulMsgSeq);
        return;
    }

    SipMsgS& stSipMsg = *(SipMsgS*)stTextMsg.pUserData;

    //������
    CLockGuard objLocker(objSipStack.m_pMapMutex);

    //���ҶԻ���Ϣ
    CSipStack::MAP_DIALOG_ID_TO_INFO::iterator iter = objSipStack.m_mapDialogIdToInfo.find(stTextMsg.ulDialogId);
    if (objSipStack.m_mapDialogIdToInfo.end() == iter)
    {
        ERROR_LOG("[ MsgSeq = %d] Create ACK Message Failed. Dialog ID(%d) Not Exist.",stTextMsg.ulMsgSeq,stTextMsg.ulDialogId);
        return ;
    }

    SIP::DIALOG_INFO& stDialogInfo = iter->second;

    //��ȡ�Ի���Ϣ��������Ϣ���Ա���
    long lResult = SIP::GetDialogInfo(stSipMsg, stDialogInfo, VOS_TRUE);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Get Dialog Info Failed On Create ACK Message.",stTextMsg.ulMsgSeq);
        return;
    }

    //����Ϊ����OK
    stDialogInfo.bRecvOK = VOS_TRUE;

    //ʹ��INVITE��CSeq��CallID
    m_ulCSeq              = stDialogInfo.ulCSeq;
    m_strCallId           = stDialogInfo.strCallId;
    m_pszRequestUriName   = stDialogInfo.strRequestUriName.c_str();
    m_pszRequestUriDomain = stDialogInfo.strRequestUriDomain.c_str();
    //ʹ��INVITE��200 OK��From��To��Tag
    m_strFromTag    = stDialogInfo.strLocalTag;     //From��Tag
    m_pszToTag      = stDialogInfo.strRemoteTag.c_str();    //To��Tag

    //ͬʱ��ӵ�������ұ�
    //objSipStack.m_mapDialogInfoToId[stDialogInfo] = stTextMsg.ulDialogId;
	INFO_LOG("[ MsgSeq = %d ] Create Ack Message Success. DialogID = %d : LocalTag = %s, RemoteTag = %s.", 
		stTextMsg.ulMsgSeq, stTextMsg.ulDialogId, stDialogInfo.strLocalTag.c_str(), stDialogInfo.strRemoteTag.c_str());
}//lint !e1788

/**
* Description:  SendSipMsg().   ����SIP��Ϣ
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapAck::SendSipMsg
    (
    SipMsgS& stSipMsg
    )
{
    //������Ϣ��
    SipDataUnitS stDataUnit = {0};

    stDataUnit.pstSipMsg = &stSipMsg;

    //����������Ϣ
    SipTptNwInfoS stTptNwInfo = {0};
    /////////////////////////////  modified by w00207027 2012-11-12 Begin //////////////////////////////////
    //SIP::SetSipTptNwInfo(stTptNwInfo, m_objSipStack.m_objServerAddr, m_objSipStack.m_objLocalAddr);
    if (m_objSipStack.m_bServerAddrEx)
    {
        SIP::SetSipTptNwInfo(stTptNwInfo, m_objSipStack.m_objServerAddrEx, m_objSipStack.m_objLocalAddr);		
    } 
    else
    {
        SIP::SetSipTptNwInfo(stTptNwInfo, m_objSipStack.m_objServerAddr, m_objSipStack.m_objLocalAddr);
    }	
    /////////////////////////////  modified by w00207027 2012-11-12 End //////////////////////////////////

    unsigned long ulTuObjId = m_objSipStack.GetTuObjId();

    //modify-123456789-cwx148380-begin
    int lResult = SIP::SetContact(stSipMsg, m_objSipStack.m_objLocalAddr, m_objSipStack.m_stRegisterInfo.szLocalId);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Contact Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    lResult = SIP::SetServer(stSipMsg, "HUAWEI eSDK/1.0");
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Contact Header Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    //lResult = SIP::SetRequestUri(stSipMsg, "123456712340101", "192");
    //if (SIP::RET_CODE_OK != lResult)
    //{
    //    ERROR_LOG("Add Contact Header Failed On Create SIP Request Message.");
    //    return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    //}
    //modify-123456789-cwx148380-end

    //������״̬������Ϣ
    EN_SIP_RESULT enResult = SipTxnHiSlSendReq(ulTuObjId, 0, &stDataUnit, &stTptNwInfo, NULL, NULL);

    //ACKû����Ӧ��ֱ���ͷŸ���Ϣ
    SipDsmReleaseMsgRef(&stDataUnit.pstSipMsg);

    if (SIP_RET_SUCCESS != enResult)
    {        
        ERROR_LOG("SipTxnHiSlSendReq Failed On Send ACK Message. Error = %s",SIP::STR_ARR_EN_SIP_RESULT[enResult]);
        return SIP::RET_CODE_SEND_SIP_MSG_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:    CSipRspMsg(). Default constructor
*/
CSipStackMsgWrapResp::CSipStackMsgWrapResp(CSipStack& objSipStack, SipMsgS& stSipReqMsg, const string& data)
    : m_objSipStack(objSipStack), m_stSipReqMsg(stSipReqMsg), m_SipMsgData(data.c_str())
{
}

/**
* Description:    ~CSipRspMsg(). Destructor
*/
CSipStackMsgWrapResp::~CSipStackMsgWrapResp()
{
}

/**
* Description:  SendRspMsg().  ������Ӧ��Ϣ
* @param  [in]  ulRespCode  ��Ӧ��
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @return       long.       ������
*/
long CSipStackMsgWrapResp::SendRspMsg
(
    unsigned long   ulRespCode,
    unsigned long   ulTuObjId,
    unsigned long   ulTxnObjId
)
{
    //Minisip��������ֵ
    EN_SIP_RESULT enResult = SIP_RET_SUCCESS;
    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    //������Ϣ��
    SipDataUnitS stDataUnitRsp = {0};

    //��������Ϣ�д�����Ӧ��Ϣ
    enResult = SipDsmCreateRspMsgFromReqMsg(ulRespCode, &m_stSipReqMsg, &stDataUnitRsp.pstSipMsg);//lint !e838
    if (SIP_RET_SUCCESS != enResult)
    {
        ERROR_LOG("Create Response Message Failed On Send Stateful Response Message. Error = %s",SIP::STR_ARR_EN_SIP_RESULT[enResult]);
        return SIP::RET_CODE_FAIL;
    }

    SipMsgS& stRspSipMsg = *stDataUnitRsp.pstSipMsg;


    //���ù�����To��Tag
    lResult = SIP::SetToTag(stRspSipMsg);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set To Tag Failed On Send Stateful Response Message.");
        return lResult;
    }

	//����Contact
	std::string IP = "";
	unsigned short port = 0;
	if(SIP::RET_CODE_OK == SIP::GetContact(m_stSipReqMsg,IP,port) && SIP_STATUS_SUCC_OK == ulRespCode)
	{
		CNetworkAddr addr;
		addr.m_lIpAddr = (long)ACE_OS::inet_addr(IP.c_str());
		addr.m_usPort = htons(port);
		(void)SIP::SetContact(stRspSipMsg,addr,"");
	}

    //��������ͷ��
    lResult = SetSpecialHeader(stRspSipMsg);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Special Header Failed On Send Stateful Response Message.");
        return lResult;
    }

    //�����Ϣ��
    if ( !m_SipMsgData.empty())
    {
        lResult = SIP::SetMsgBody(stRspSipMsg, SIP::SUB_MEDIA_TYPE_T28181_XML, m_SipMsgData.size() + 1, const_cast<char*>(m_SipMsgData.c_str()) );
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("Set Message Body Failed On Send Stateful Response Message.");
            return lResult;
        }
    }

    //������Ӧ��Ϣ
    enResult = SipTxnHiSfSendRsp(ulTxnObjId, ulTuObjId, &stDataUnitRsp, NULL);

    //���ͷ�SIP��Ϣ
    SipDsmReleaseMsgRef(&stDataUnitRsp.pstSipMsg);

    if (SIP_RET_SUCCESS != enResult)
    {
        ERROR_LOG("Send Message Failed On Send Stateful Response Message. Error = %s",SIP::STR_ARR_EN_SIP_RESULT[enResult]);
        return SIP::RET_CODE_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stRspSipMsg     SIP��Ӧ��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapResp::SetSpecialHeader
(
    SipMsgS& stRspSipMsg
)
{
    return SIP::RET_CODE_OK;
}//lint !e715 ���಻��ʹ�øñ���


/**
* Description:    CSipStackMsgWrapInviteResp(). Default constructor
* @param  [in]  objSipStack     SipStack����
* @param  [in]  stSipReqMsg     SIP������Ϣ����
* @param  [in]  ulDialogId          �Ի�ID
* @param  [in]  szMsgBody       ��Ϣ���ַ���
* @param  [in]  ulMsgBodyLen    ��Ϣ�峤��
*/
//CSipStackMsgWrapInviteResp::CSipStackMsgWrapInviteResp
//(
//    CSipStack&      objSipStack, 
//    SipMsgS&        stSipReqMsg,
//    unsigned long   ulDialogId,
//    unsigned long   ulMsgBodyLen,
//    char*           pszMsgBody
//) : CSipStackMsgWrapResp(objSipStack, stSipReqMsg),
//    m_ulDialogId(ulDialogId),
//    m_ulMsgBodyLen(ulMsgBodyLen),
//    m_pszMsgBody(pszMsgBody)
//{
//}

/**
* Description:    CSipRspMsgRegist(). Default constructor
* @param  [in]  objSipStack     SipStack����
* @param  [in]  stSipReqMsg     SIP������Ϣ����
*/
CSipStackMsgWrapRegisterResp::CSipStackMsgWrapRegisterResp( CSipStack& objSipStack, SipMsgS& stSipReqMsg, bool wwwAuth)
    : CSipStackMsgWrapResp(objSipStack, stSipReqMsg),m_bwwwAuth(wwwAuth),m_ulExpireTime(CSipStackConfigMgr::Instance().GetServiceCfg().GetExpires())
{

}

CSipStackMsgWrapRegisterResp::~CSipStackMsgWrapRegisterResp()
{

}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stRspSipMsg     SIP��Ӧ��Ϣ����
* @return       long.       ������
*/

static unsigned long s_turns = 0;

long CSipStackMsgWrapRegisterResp::SetSpecialHeader( SipMsgS& stRspSipMsg )
{
    //�Զ��庯������ֵ
    long lResult = SIP::RET_CODE_OK;

    if (!m_bwwwAuth)
    {
        lResult = SIP::SetExpires(stRspSipMsg,m_ulExpireTime);
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("Create Expires String Failed On Set Register Response Header.");
            return lResult;
        }
        string psDate = SIP::GetCurrentTime();
        if (psDate.empty())
        {
            ERROR_LOG("Create Date String Failed On Set Register Response Header, get current time.");
            return lResult;
        }

        lResult = SIP::SetDate(stRspSipMsg, psDate.c_str());
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("Create Date String Failed On Set Register Response Header.");
            return lResult;
        }

        return SIP::RET_CODE_OK;
    }

   
    //modify-cwx148380-begin
    char nonce[] = "Sc1mA0GPoCyZOhLrBjDFhlkezSw8TIK7";
    char opaq[] = "7340225321182";
    s_turns++;
    nonce[1] = 'a' + s_turns % 2;
    opaq[1] = '0' + s_turns % 2;

    SIP::PLAT_AUTH_INFO stPlatAuthInfo;
    stPlatAuthInfo.stAlgorithm = "MD5";
    stPlatAuthInfo.strNonce= nonce;   // TODO  ���������
    stPlatAuthInfo.strOpaque=opaq;
    stPlatAuthInfo.strRealm= "huawei.com";//;"3402000000"
    //modify-cwx148380-end

    //���SIP������Ϣ����
    //const unsigned long ulSipMethod = stRspSipMsg.uFirstLine.stRequestLine.stMethod.usTokenId;

    //����WWW-Authorization
    SipWWWAuthenticate* pstAuthorization = (SipWWWAuthenticate*)SipMemCpMalloc(stRspSipMsg.hdlMemCp, sizeof(SipWWWAuthenticate));
    if (SS_NULL_PTR == pstAuthorization)
    {
        ERROR_LOG("Create Authorization Header Failed On Set Authorization Header.");
        return SIP::RET_CODE_FAIL;
    }

    //Digest��Ȩ��ʽ
    pstAuthorization->bIsAuthDigestType = SS_TRUE;

    lResult = SIP::CreateQuoteString(stRspSipMsg.hdlMemCp, pstAuthorization->pstrRealm, stPlatAuthInfo.strRealm.c_str());
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Realm String Failed On Set WWW-Authorization Header.");
        return lResult;
    }

    //modify-123456789-cwx148380-begin
    lResult = SIP::CreateSipString(stRspSipMsg.hdlMemCp, pstAuthorization->pstrAlgorithm, stPlatAuthInfo.stAlgorithm.c_str());
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Algorithm String Failed On Set WWW-Authorization Header.");
        return lResult;
    }

    //lResult = SIP::CreateSipString(stRspSipMsg.hdlMemCp, pstAuthorization->pstrDomainURI, "huawei.com");
    //if (SIP::RET_CODE_OK != lResult)
    //{
    //    ERROR_LOG("Create Algorithm String Failed On Set WWW-Authorization Header.");
    //    return lResult;
    //}

    //lResult = SIP::CreateSipString(stRspSipMsg.hdlMemCp, pstAuthorization->pstrQOPOptions, "auth");
    //if (SIP::RET_CODE_OK != lResult)
    //{
    //    ERROR_LOG("Create Algorithm String Failed On Set WWW-Authorization Header.");
    //    return lResult;
    //}

    lResult = SIP::CreateQuoteString(stRspSipMsg.hdlMemCp, pstAuthorization->pstrOpaque, stPlatAuthInfo.strOpaque.c_str());
    if (SIP::RET_CODE_OK != lResult)
    {
    	ERROR_LOG("Create Opaque String Failed On Set WWW-Authorization Header.");
    	return lResult;
    }
    //modify-123456789-cwx148380-end

    lResult = SIP::CreateQuoteString(stRspSipMsg.hdlMemCp, pstAuthorization->pstrNonce, stPlatAuthInfo.strNonce.c_str());
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Nonce String Failed On Set WWW-Authorization Header.");
        return lResult;
    }


    //���WWW-Authrizationͷ��SIP��Ϣ
    lResult = SIP::AddWWWAuthorizationHeader(stRspSipMsg, *pstAuthorization);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add WWW-Authoraization Header Failed On Set WWW-Authorization Header.");
        return lResult;
    }

    return SIP::RET_CODE_OK;//lint e!438
}

/**
* Description:    CSipRspMsgOption(). Default constructor
* @param  [in]  objSipStack     SipStack����
* @param  [in]  stSipReqMsg     SIP������Ϣ����
*/
CSipStackMsgWrapOptionsResp::CSipStackMsgWrapOptionsResp( CSipStack& objSipStack, SipMsgS& stSipReqMsg)
    : CSipStackMsgWrapResp(objSipStack, stSipReqMsg)
{

}

CSipStackMsgWrapOptionsResp::~CSipStackMsgWrapOptionsResp()
{

}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapOptionsResp::SetSpecialHeader
    (
    SipMsgS& stSipMsg
    )
{
    //�Զ��庯������ֵ
    long lResult = SIP::RET_CODE_OK;

    string psDate = SIP::GetCurrentTime();
    if (psDate.empty())
    {
        ERROR_LOG("Create Date String Failed On Set Register Option Header, get current time.");
        return lResult;
    }

    lResult = SIP::SetDate(stSipMsg, psDate.c_str());
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Date String Failed On Set Register Option Header.");
        return lResult;
    }

    return SIP::RET_CODE_OK;
}//lint !e715 ���಻��ʹ�øñ���

/**
* Description:    CSipMsgSubscribe(). Default constructor
*/
CSipStackMsgWrapSubscribe::CSipStackMsgWrapSubscribe(CSipStack& objSipStack, const SIP::TEXT_MSG& stTextMsg) 
    :CSipStackMsgWrap(objSipStack, stTextMsg),m_ulExpireTime(90)
{
    //��������Request-URI�����ֲ���
    //     if (VOS_NULL != m_stTextMsg.pMsgHeader && 0 < m_stTextMsg.ulMsgHeaderLen)
    //     {
    //         m_pszRequestUriName = (char*)m_stTextMsg.pMsgHeader;
    //     }

    //     if (VOS_NULL != m_stTextMsg.pUserData)
    //     {
    //         SIP::PEER_URI_INFO& stPeerUriInfo = *(SIP::PEER_URI_INFO*)m_stTextMsg.pUserData;
    // 
    //         m_pszRequestUriName     = stPeerUriInfo.szUriUserName;
    //         m_pszRequestUriDomain   = stPeerUriInfo.szUriHostName;
    //         m_pszUserAgentName      = stPeerUriInfo.szUserAgentName;
    //     }

    //INFO_LOG("Create SIP Subscribe. Request URI = %s@%s userAgent:%s. ",m_pszRequestUriName, m_pszRequestUriDomain, m_pszUserAgentName);
}

/**
* Description:  SetSpecialHeader().     ��������ͷ��
* @param  [in]  stSipMsg   SIP��Ϣ����
* @return       long.       ������
*/
long CSipStackMsgWrapSubscribe::SetSpecialHeader
    (
    SipMsgS&                stSipMsg
    )
{
    long lResult = SIP::SetExpires(stSipMsg, m_ulExpireTime);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Expires String Failed On Set Subscribe Request Header.");
        return lResult;
    }

    //����Contactͷ��
    lResult = SIP::SetContact(stSipMsg, m_objSipStack.m_objLocalAddr, m_objSipStack.m_stRegisterInfo.szLocalId);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Contact String Failed On Set Subscribe Request Header.");
        //return lResult;
    }

    lResult = SIP::SetEvent(stSipMsg, "presence");
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Event String Failed On Set Subscribe Request Header.");
        return lResult;
    }

    //������Ϣ��
/*    if (m_objSipStack.IsServerAddrEx())
    {
        lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_T28181_XML, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    }
    else
    {
        lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_XML, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    }*/	
    lResult = SIP::SetMsgBody(stSipMsg, SIP::SUB_MEDIA_TYPE_T28181_SUBSCRIBE_XML, m_stTextMsg.ulMsgBodyLen, m_stTextMsg.pszMsgBody);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set Message Body Failed On Create SIP Request Message.");
        return SIP::RET_CODE_CREATE_SIP_MSG_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  PostSendSuccess().  ���ͳɹ�����
* @return       void. 
*/
void CSipStackMsgWrapSubscribe::PostSendSuccess()
{
	return;
}

}//end namespace 

