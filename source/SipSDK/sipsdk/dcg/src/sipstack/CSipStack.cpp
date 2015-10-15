#include <cerrno>
#include <iostream>
#include <ctime>
#include <algorithm>
#include "tinyxml.h"
#include "vos_config.h"
#include "digcalc.h"
#include "CLockGuard.h"
#include "sipstack_msg_ctrl.h"
#include "sipstack_msg_wrap.hpp"
#include "sipstack_task.h"
#include "sipstack_agent_server_impl.hpp"
#include "CSipStack.hpp"
#include "Log.h"

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#else
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef CONN_ERRNO
#define CONN_ERRNO errno
#endif


#define RESEND_INTERVAL             (10)

#pragma warning(push)
#pragma warning(disable:4996)

//lint -e429
namespace SipStack{

//��ʼ��ȫ��SIPЭ��ջָ��
CSipStack* g_pSipStack = NULL;

/**
* Description:  ������ʱ�¼�����
* @param  [in]  pArg        ���崦����
* @param  [in]  ullScales   ��ʱ��ָ��
* @param  [in]  enStyle     ��������
*/
void CSipStackTimerTrigger::OnTrigger
(
    void *pArg, 
    ULONGLONG ullScales, 
    TriggerStyle enStyle
)
{
    //INFO_LOG("CSipTimerTrigger::onTrigger.");

    CSipStack* pSipStack = VOS_CAST((CSipStack*)pArg);

    if (NULL == pSipStack)
    {
        ERROR_LOG("timer trigger - on trigger - failure to dync-cast sip stack class - pArg=%p.", pArg);
        return ;
    }

    SIP::EVENT_INFO stEventInfo(SIP::EVENT_TYPE_STACK_TIMER_TRIGGER);
    //��Ӷ�ʱ�������¼�
    long lResult = g_pSipStack->AddEvent(stEventInfo);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("timer trigger - on trigger - failure to add event for timer = Ret=%d.", lResult);
        return ;
    }

    //�����������账��
    ullScales   = ullScales;
    enStyle     = enStyle;
}//lint !e438

/**
* Description:  ������ʱ�¼�����
* @param  [in]  pArg        ���崦����
* @param  [in]  ullScales   ��ʱ��ָ��
* @param  [in]  enStyle     ��������
*/
void CSipStackMsgTimerTrigger::OnTrigger
(
    void *pArg, 
    ULONGLONG ullScales, 
    TriggerStyle enStyle
)
{
    //INFO_LOG("CSipTimerTrigger::onTrigger.");

    CSipStack* pSipStack = VOS_CAST((CSipStack*)pArg);

    //ERROR_LOG("Enter CSipMsgTimerTrigger::onTrigger";

    if (NULL == pSipStack)
    {
        ERROR_LOG("timer trigger - on trigger - failure to dync-cast sip stack class - ");
        return ;
    }

    pSipStack->CheckReqMsg();

    //�����������账��
    ullScales   = ullScales;
    enStyle     = enStyle;
}//lint !e438

/**
* Description:    CSipUdpHandle(). Default constructor
*/
CSipUdpHandle::CSipUdpHandle()
{
}

/**
* Description:    ~CSipUdpHandle(). Destructor
*/
CSipUdpHandle::~CSipUdpHandle()
{
}

//// modified by dhong
static string  g_recvStr;

/**
* Description:  handle_recv(). �������ݵ���ʱ���ô˺���
* @return       void.
*/
void CSipUdpHandle::handle_recv(void)
{
    DEBUG_LOG("*********************handle_recv:begin*********************");

    char szRecvBuf[SIP::MAX_LEN_UDP_PACKET] = {0};

    sockaddr_in stPeerAddr = {0};
    socklen_t nPeerAddrLen = sizeof(stPeerAddr);

    int nResult = recvfrom((unsigned long)m_lSockFD, szRecvBuf, sizeof(szRecvBuf), 0, (sockaddr*)&stPeerAddr, &nPeerAddrLen);

    //���ü�����������
    setHandleRecv(VOS_TRUE);

    if (0 >= nResult)
    {
        WARN_LOG("handle_recv:can't receive data from socket.");
        return ;
    }
	if(nResult >= SIP::MAX_LEN_UDP_PACKET)
	{
		WARN_LOG("handle_recv:receive too long data from socket.");
		return ;
	}

    //���ý�����
    szRecvBuf[nResult] = 0;
    char* recv_data_buf_p = new char[(unsigned int)nResult];
    ACE_OS::memcpy(recv_data_buf_p, szRecvBuf, sizeof(char)*(unsigned int)nResult);
    if ( NULL == recv_data_buf_p )
    {
        WARN_LOG("handle_recv:can't malloc recv data buffer.");
        return ;
    }

    const unsigned char* addr_ipv4_p = (unsigned char*)&(stPeerAddr.sin_addr.s_addr);
    const int   addr_port_i = ntohs(stPeerAddr.sin_port);
	DEBUG_LOG( "handle_recv:src_addr=%u.%u.%u.%u:%u,length=%d,data=\n%s",
		(unsigned int)addr_ipv4_p[0],
		(unsigned int)addr_ipv4_p[1],
		(unsigned int)addr_ipv4_p[2],
		(unsigned int)addr_ipv4_p[3],
		addr_port_i,
		nResult, 
		szRecvBuf);

	POST_TO_APP_ST* post_to_pu_p = new POST_TO_APP_ST; //�ͷŽ���Э��ջ�����߳�
    ACE_OS::memset(post_to_pu_p, 0x0, sizeof(POST_TO_APP_ST));

    post_to_pu_p->message_string_st.pcData = recv_data_buf_p;
    post_to_pu_p->message_string_st.ulLen  = (unsigned int)nResult;

    post_to_pu_p->tpt_network_info_st.ucAddressType              = SIP_ADDR_TYPE_IPV4;
    post_to_pu_p->tpt_network_info_st.ucProtocol                 = SIP_TRANSPORT_UDP;

    post_to_pu_p->tpt_network_info_st.stSrcAddr.u.aucIPv4Addr[0] = (unsigned char)addr_ipv4_p[0];
    post_to_pu_p->tpt_network_info_st.stSrcAddr.u.aucIPv4Addr[1] = (unsigned char)addr_ipv4_p[1];
    post_to_pu_p->tpt_network_info_st.stSrcAddr.u.aucIPv4Addr[2] = (unsigned char)addr_ipv4_p[2];
    post_to_pu_p->tpt_network_info_st.stSrcAddr.u.aucIPv4Addr[3] = (unsigned char)addr_ipv4_p[3];
    post_to_pu_p->tpt_network_info_st.stSrcAddr.iPort            = addr_port_i;

    //TODO:DstAddrӦ��ʹ�õ�ǰЭ��ջ�����ĵ�ַ����֮ǰЭ��ջԴ��ַ��Ŀ�ĵ�ַʹ��ͬһ����ַ���ʴ˴�
    //     ����ԭЭ��ջ���룬�������������
    post_to_pu_p->tpt_network_info_st.stDstAddr.u.aucIPv4Addr[0] = (unsigned char)addr_ipv4_p[0];
    post_to_pu_p->tpt_network_info_st.stDstAddr.u.aucIPv4Addr[1] = (unsigned char)addr_ipv4_p[1];
    post_to_pu_p->tpt_network_info_st.stDstAddr.u.aucIPv4Addr[2] = (unsigned char)addr_ipv4_p[2];
    post_to_pu_p->tpt_network_info_st.stDstAddr.u.aucIPv4Addr[3] = (unsigned char)addr_ipv4_p[3];
    post_to_pu_p->tpt_network_info_st.stDstAddr.iPort            = addr_port_i;

    if ( !CSipStackTask::Instance().PostToApp(post_to_pu_p) )
    {
        delete [] recv_data_buf_p;
        recv_data_buf_p = NULL;
        post_to_pu_p->message_string_st.pcData = NULL;
        post_to_pu_p->message_string_st.ulLen  = 0;
        delete post_to_pu_p;
        post_to_pu_p = NULL;

        ERROR_LOG("handle_recv:failure to post to app.");
        return ;//lint !e438
    }

    DEBUG_LOG("handle_recv:success.");
}

/**
* Description:  handle_send(). �����Է�������ʱ���ô˺���
* @return       void.
*/
void CSipUdpHandle::handle_send(void)
{
}

/**
* Description:    CSipStack(). Default constructor
*/
CSipStack::CSipStack():m_mapKey(0), m_reserved(0)
{
    m_pfnNotify      = VOS_NULL; //�ص�����ָ��
    m_pNotifyFnParams         = VOS_NULL; //�û�����ָ��
    m_pProceEventThread = VOS_NULL; //�����¼��߳�  
    m_pTimerTrigger     = VOS_NULL; //��ʱ��
    m_pMsgTimerTrigger  = VOS_NULL; // MSG timer
    m_pMapMutex         = VOS_NULL; //map��

    m_bRegister             = VOS_FALSE;    //�Ƿ���ע��
    m_bProcEventThreadRun   = VOS_FALSE;    //�����¼��߳��Ƿ�����
    m_bHasRedirect          = VOS_FALSE;    //�Ƿ����ض���
    m_bRedirected           = VOS_FALSE;    //�Ƿ��ض����
    m_bTempRedirect         = VOS_FALSE;    //�Ƿ�Ϊ��ʱ�ض���

    m_ulTuObjId                 = 0;    //TU����ID
    m_ulCSeq                    = 0;    //SIP��Ϣ��CSeq
    m_ulTimerId                 = 0;    //��ʱ��ID
    m_ulPlatAuthInfoChangeNum   = 0;    //ƽ̨��Ȩ��Ϣ�������

    //ע����Ϣ
    memset(&m_stRegisterInfo, 0, sizeof(m_stRegisterInfo));
    //�����MD5��16�����ַ���
    memset(m_szPwdMd5, 0, sizeof(m_szPwdMd5));

    //BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
    //�����߳���
    m_pThreadMutex = VOS_CreateMutex();
    //END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�

    // added by dhong 
    m_pThreadlistSvrInfoMutex = VOS_CreateMutex();
    m_pThreadServerInfoMutex = VOS_CreateMutex();
    // end by dhong

    // ������Ϣ������ // added by dhong
    m_pReqMsgMutex = VOS_CreateMutex();

    ///////////////////////////// add by w00207027 2012-11-02 Begin ///////////////////////////////////////////////////
    m_objServerAddrEx.m_lIpAddr =0;
    m_objServerAddrEx.m_usPort = 0;
    m_bServerAddrEx = false;
    ///////////////////////////// add by w00207027 2012-11-02 End /////////////////////////////////////////////////////

    //��ʼ��ǰ���ط��������ݽṹ
    m_mapReqResend[SIP_METHOD_REGISTER] = DEAL_RESEND_LIST();
    m_mapReqResend[SIP_METHOD_OPTIONS]  = DEAL_RESEND_LIST();
    m_mapReqResend[SIP_METHOD_INVITE]   = DEAL_RESEND_LIST();
    m_mapReqResend[SIP_METHOD_MESSAGE]  = DEAL_RESEND_LIST();

    m_mapRspResend[SIP_METHOD_INVITE]   = DEAL_RESEND_LIST();
    m_mapRspResend[SIP_METHOD_ACK]      = DEAL_RESEND_LIST();
    m_mapRspResend[SIP_METHOD_BYE]      = DEAL_RESEND_LIST();
    m_mapRspResend[SIP_METHOD_MESSAGE]  = DEAL_RESEND_LIST();

	//
	m_usPort = 0;
}

/**
* Description:    ~CSipStack(). Destructor
*/
CSipStack::~CSipStack()
{
    m_pfnNotify      = VOS_NULL; //�ص�����ָ��
    m_pNotifyFnParams         = VOS_NULL; //�û�����ָ��
    m_pProceEventThread = VOS_NULL; //�����¼��߳�
    m_pTimerTrigger     = VOS_NULL; //��ʱ��
    m_pMapMutex         = VOS_NULL; //map��

    //BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
    //���п��ܷ���MiniSIP��������ڴ����������߳���
    if (VOS_NULL != m_pThreadMutex)
    {
        (void)VOS_DestroyMutex(m_pThreadMutex);
        m_pThreadMutex = VOS_NULL;
    }

    // added by dhong
    if (VOS_NULL != m_pReqMsgMutex)
    {
        (void)VOS_DestroyMutex(m_pReqMsgMutex);
        m_pReqMsgMutex = VOS_NULL;
    }

    // added by dhong
    if (NULL != m_pThreadlistSvrInfoMutex)
    {
        (void)VOS_DestroyMutex(m_pThreadlistSvrInfoMutex);
        m_pThreadlistSvrInfoMutex = NULL;
    }

    if (NULL != m_pThreadServerInfoMutex)
    {
        (void)VOS_DestroyMutex(m_pThreadServerInfoMutex) ;
        m_pThreadServerInfoMutex = NULL;
    }
    // end by dhong

    while(!m_msgMap.empty())
    {
        m_msgItertor = m_msgMap.begin();
        SIP::REQ_MSG_INFO* pReqMsgInfo = m_msgItertor->second;
        ACE_Guard<ACE_Recursive_Thread_Mutex> locker(CSipStackTask::MiniSipMutex()); //�����Ƿ�����ɹ��������MINISIP��ʼ��
        if ( !locker.locked() )
        {
            ERROR_LOG("sipstack class - ~CSipStack - someting wrong with minisip locker.");
        }
        SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
        VOS_DELETE(pReqMsgInfo);
        m_msgMap.erase(m_msgItertor);
    }
    //END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
}

long CSipStack::Init(SIP::PCALLBACK_NOTIFY pfnNotify, void* pNotifyFnParams, const unsigned short local_port )
{
    INFO_LOG("sip stack - init - begin");

    //�ص�����ָ�벻��Ϊ��
    if (VOS_NULL == pfnNotify)
    {
        ERROR_LOG("sip stack - init - notify function pointer is null");
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    //����֪ͨ�ص�����״ֵ̬
    m_pfnNotify        = pfnNotify;
    m_pNotifyFnParams  = pNotifyFnParams;

    //����Map��
    m_pMapMutex = VOS_CreateMutex();

    //���CSeq����SIP��Ϣ��Ϣ��ӳ���
    m_mapCSeqToSipMsgInfo.clear();

    //��ʼ��MiniSip
    long lResult = VOS_FAIL;
    {
        ACE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, locker, CSipStackTask::MiniSipMutex(), VOS_FALSE);
        lResult = SIP::InitMiniSip();//lint !e838
        if (SIP::RET_CODE_OK != lResult)
        {
            ERROR_LOG("sip stack - init - failure to init minisip");
            return lResult;
        }
    }

    //���ͨ�Ź�����ʵ��
    CConnMgr& rConnMgr = CONN_MGR;

    if (VOS_SUCCESS != rConnMgr.init(100, VOS_TRUE, VOS_FALSE, VOS_FALSE))
    {
        ERROR_LOG("sip stack - init - failure to init communication manager.");
        return SIP::RET_CODE_FAIL;
    }
    rConnMgr.run();

    //���ñ��ص�ַ
    // SetLocalAddr(local_port);
    m_objLocalAddr.m_usPort = htons(local_port);

    //����UDP socket
    lResult = rConnMgr.regUdpSocket(&m_objLocalAddr, &m_objSipUdpHandle, NULL);
    if(VOS_OK != lResult)
    {
        ERROR_LOG("sip stack - init - failure to create udp socket - ErrorCode = %d",lResult);    
        return SIP::RET_CODE_FAIL;
    }


    //����ȫ�ֱ���
    g_ulSipUdpSocket = (unsigned long)m_objSipUdpHandle.m_lSockFD;

    //��ʼ���¼�ͬ������
    lResult = m_objEventSyncQueue.init(SIP::EVENT_SYNC_QUEUE_NUM);
    if (VOS_OK != lResult)
    {
        ERROR_LOG("sip stack - init - failure to init event synchronous queue.");
        return SIP::RET_CODE_FAIL;
    }

    //���������¼��߳�
    m_bProcEventThreadRun = VOS_TRUE;
    unsigned long ulResult = VOS_CreateThread((VOS_THREAD_FUNC)ProcEventThreadEntry, this, &m_pProceEventThread, VOS_DEFAULT_STACK_SIZE);
    if (VOS_OK != ulResult)
    {
        ERROR_LOG("sip stack - init - failure to create process event thread - ErrorCode = %d",ulResult); 
        return SIP::RET_CODE_FAIL;
    }

    //������ʱ��
    if (NULL == VOS_NEW(m_pTimerTrigger))
    {
        ERROR_LOG("sip stack - init - failure to alloc timer trigger. ");
        return SIP::RET_CODE_FAIL;
    }

    // ��ʼ����ʱ��
    if (VOS_SUCCESS != NVS_TIMER.init(100))
    {
        ERROR_LOG("sip stack - init - failure to init timer");
        return SIP::RET_CODE_FAIL;
    }
    //ע�ᶨʱ��
    if (VOS_OK != NVS_TIMER.registerTimer(m_pTimerTrigger, this, SIP::TIMER_INTERVAL * 10, enRepeated))
    {
        ERROR_LOG("sip stack - init - failure to register timer trigger. ");
        return SIP::RET_CODE_FAIL;
    }

    //������ʱ�� msg
    if (NULL == VOS_NEW(m_pMsgTimerTrigger))
    {
        ERROR_LOG("sip stack - init - failure to alloc message timer trigger. ");
        return SIP::RET_CODE_FAIL;
    }

    //ע�ᶨʱ�� msg
    if (VOS_OK != NVS_TIMER.registerTimer(m_pMsgTimerTrigger, this, SIP::TIMER_INTERVAL * 100, enRepeated))
    {
        ERROR_LOG("sip stack - init - failure to register timer trigger.");
        return SIP::RET_CODE_FAIL;
    }

    NVS_TIMER.run();

    INFO_LOG("sip stack - init - success");
    return SIP::RET_CODE_OK;
}

/**
* Description:  ReleaseSipStack(). �ͷ�SipStack
* @return       long.       ������
*/
long CSipStack::Fini()
{
    INFO_LOG("sipstack - finish - begin.");
    //�ͷŶ�ʱ��
    if (VOS_NULL != m_pTimerTrigger)
    {
        (void)NVS_TIMER.cancelTimer(m_pTimerTrigger);

        VOS_DELETE(m_pTimerTrigger);
    }

    if (VOS_NULL != m_pMsgTimerTrigger)
    {
        (void)NVS_TIMER.cancelTimer(m_pMsgTimerTrigger);

        VOS_DELETE(m_pMsgTimerTrigger);
    }

    //�˳������¼��߳�
    m_bProcEventThreadRun = VOS_FALSE;

    if (VOS_NULL != m_pProceEventThread)
    {
        //�����˳��߳��¼�
        SIP::EVENT_INFO stEventInfo(SIP::EVENT_TYPE_EXIT_THREAD);

        //���¼�ͬ�����в����˳��߳��¼������账�����
        (void)AddEvent(stEventInfo);

        //���ȴ�30�룬����δ�˳���������߼�������
        (void)VOS_ThreadJoin(m_pProceEventThread);
        //�ͷ��ڴ�
        VOS_free(m_pProceEventThread);
        m_pProceEventThread = VOS_NULL;
    }

    //����ȫ�ֱ���Ϊ��Чֵ
    g_ulSipUdpSocket = (unsigned long)InvalidSocket;

    //���ͨ�Ź�����ʵ��
    CConnMgr& objConnMgr = CONN_MGR;

    //��ͨ�Ź��������Ƴ�SIP��UDP��Socket
    objConnMgr.removeUdpSocket(&m_objSipUdpHandle);
    objConnMgr.exit();

    //��ռ�Ȩ��Ϣ
    m_stPlatAuthInfo.strRealm = "";
    m_stPlatAuthInfo.strNonce = "";
    m_stPlatAuthInfo.strOpaque = "";

    //����������Ϣ��Ϣ
    ClearAllMsgInfo();

    //�ͷ�MiniSip
    {
        ACE_Guard<ACE_Recursive_Thread_Mutex> locker(CSipStackTask::MiniSipMutex()); //�����Ƿ�����ɹ��������MINISIP��ʼ��
        if ( !locker.locked() )
        {
            ERROR_LOG("sipstack class - release sipstack - someting wrong with minisip locker.");
        }
        SipLmCoreLibDeInit();
    }

    m_mapDialogIdToInfo.clear();
    m_mapDialogInfoToId.clear();

    //�ͷ�Map��
    if (VOS_NULL != m_pMapMutex)
    {
        (void)VOS_DestroyMutex(m_pMapMutex);
        m_pMapMutex = VOS_NULL;
    }

	NVS_TIMER.exit();
    INFO_LOG("sipstack - finish - end.");
    return SIP::RET_CODE_OK;
}

/**
* Description:  ProcEventThreadEntry().  �����¼��߳����
* @param  [in]  pPara   �߳���ڲ���
* @return       unsigned long.   ������
*/
unsigned long STDCALL CSipStack::ProcEventThreadEntry(void* pPara)
{
    //����ָ�밲ȫת��
    CSipStack* pThis = VOS_CAST((CSipStack*)pPara);
    if (NULL == pThis)
    {
        ERROR_LOG("Process Event Thread Parameter Invalidate. Parameter Address is %x",pPara);
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    //ִ�д����¼��߳�
    pThis->ProcEventThread();

    return SIP::RET_CODE_OK;
}

/**
* Description:  ProcEventThread().  �����¼��߳�
* @return       void.
*/
void CSipStack::ProcEventThread()
{
    //��ն�ʱ����Ϣmap
    m_mapTimerIdToInfo.clear();

    //����ֵ
    long lResult = SIP::RET_CODE_OK;
    //�¼���Ϣָ��
    SIP::EVENT_INFO* pstEventInfo = VOS_NULL;
    const ACE_Time_Value rest_time((long long)0, 50 * 1000);

    while (m_bProcEventThreadRun)
    {
        pstEventInfo = VOS_NULL;
        //����Ϊ��ʱ�����ȣ�ֱ����ȡһ���¼����������˳����̣߳��������в���EVENT_TYPE_EXIT_THREAD�¼�
        if (m_objEventSyncQueue.empty())
        {
            ACE_OS::sleep(rest_time);
            continue;
        }
        lResult = m_objEventSyncQueue.popFrontEv(pstEventInfo);
        if (VOS_OK != lResult || NULL == pstEventInfo )
        {
            ACE_OS::sleep(rest_time);
            continue;
        }

        //�ֱ���ͬ���¼�
        switch (pstEventInfo->enEventType)
        {
        case SIP::EVENT_TYPE_STACK_TIMER_TRIGGER:  //��ʱ�������¼�
            {
                HandleEventTimerTrigger();
                break;
            }

        case SIP::EVENT_TYPE_STACK_TIMER_REGISTER:  //��ʱ��ע���¼�
            {
                if ( NULL != pstEventInfo->pEventInfo )
                {
                    lResult = HandleEventTimerRegister(pstEventInfo->pEventInfo, pstEventInfo->ulEventInfoLen);
                }
                break;
            }

        case SIP::EVENT_TYPE_STACK_TIMER_CANCEL:    //��ʱ��ȡ���¼�
            {
                if ( NULL != pstEventInfo->pEventInfo )
                {
                    lResult = HandleEventTimerCancel(pstEventInfo->pEventInfo, pstEventInfo->ulEventInfoLen);
                }
                break;
            }

        //case SIP::EVENT_TYPE_STACK_REDIRECT:        //�ض����¼�
        //    {
        //        if ( NULL != pstEventInfo->pEventInfo )
        //        {
        //            HandleEventRedirect(pstEventInfo->pEventInfo, pstEventInfo->ulEventInfoLen);
        //        }
        //        break;
        //    }

        case SIP::EVENT_TYPE_EXIT_THREAD:    //�߳��˳�
            {
                INFO_LOG("process event thread - exit thread -begin.");
                m_bProcEventThreadRun = VOS_FALSE;
                break;
            }

        case SIP::EVENT_TYPE_STACK_INVALID:    //��Ч����
        default:    //��֧�ֵ��¼�����
            {
                WARN_LOG("process event thread - unsupport event type -  Event= %s",SIP::STR_ARR_EVENT_TYPE[pstEventInfo->enEventType]);
                //��ֹ���ַǷ�ֵ
                pstEventInfo->enEventType = SIP::EVENT_TYPE_STACK_INVALID;
                break;
            }
        }

        if (SIP::RET_CODE_OK != lResult)
        {
			ERROR_LOG(" process event thread - failure to handle - Error=0x%04X, Event=%s.",
				lResult, SIP::STR_ARR_EVENT_TYPE[pstEventInfo->enEventType]);
        }

        //�ͷŴ˴��¼�
        if ( NULL != pstEventInfo->pEventInfo )
        {
			VOS_DELETE(pstEventInfo->pEventInfo);
        }
        VOS_DELETE(pstEventInfo);

    }

    //�߳����˳�������¼�ͬ�����С�
    while (VOS_OK == m_objEventSyncQueue.popFrontEv(pstEventInfo, 0, QUEUE_MODE_NOWAIT))
    {
        //�ͷ��¼�
        if ( NULL != pstEventInfo )
        {
            if ( NULL != pstEventInfo->pEventInfo )
            {
				VOS_DELETE(pstEventInfo->pEventInfo);
            }
            VOS_DELETE(pstEventInfo);
        }
        pstEventInfo = VOS_NULL;
    }

    INFO_LOG("process event thread - exit.");
}

/**
* Description:  ClearAllMsgInfo().  ����������Ϣ��Ϣ
* @return       void.
*/
void CSipStack::ClearAllMsgInfo()
{
    //������
    INFO_LOG("sipstack class - clear all msg info");
    ACE_Guard<ACE_Recursive_Thread_Mutex> locker(CSipStackTask::MiniSipMutex());
    if ( !locker.locked() )
    {
        ERROR_LOG("sipstack class - clear all msg info - some wrong with minisip lock.");
    }

    //������ֹ��������
    MAP_C_SEQ_TO_SIP_MSG_INFO::iterator iter = m_mapCSeqToSipMsgInfo.begin();
    for (; iter != m_mapCSeqToSipMsgInfo.end(); ++iter)
    {
        SIP_MSG_INFO& stMsgInfo = *iter->second;

        //ǿ����ֹ����
        (void)SipTxnHiTerminateTxn(stMsgInfo.ulTxnObjId, stMsgInfo.ulTuObjId, SIP_TERM_MODE_FORCEFUL_NO_CANCEL_RSP);
        //�ͷ�SIP��Ϣ
        SipDsmReleaseMsgRef(&stMsgInfo.pstSipMsg);
        //�ͷ��ڴ�
        VOS_DELETE(iter->second);
    }

    //���map
    m_mapCSeqToSipMsgInfo.clear();
}

/**
* Description:  AddEvent(). ����¼�
* @param  [in]  stEventInfo     �¼���Ϣ����
* @return       long.   ������
*/
long CSipStack::AddEvent(const SIP::EVENT_INFO& stEventInfo)
{
    if ( SIP::MAX_STR_ARR_EVENT_TYPE_COUNT <= stEventInfo.enEventType )
    {
        ERROR_LOG("add event - invalid event type - %d",stEventInfo.enEventType);
        return SIP::RET_CODE_FAIL;
    }

    INFO_LOG("add event - begin. %s.", SIP::STR_ARR_EVENT_TYPE[stEventInfo.enEventType]);

    long lResult = SIP::RET_CODE_OK;

    //�����¼�
    SIP::EVENT_INFO* pstEventInfo = VOS_NEW(pstEventInfo);
    //����ʧ��
    if (NULL == pstEventInfo)
    {
        //�˳��߳�ʱ���������¼�ͬ�����в���һ���¼���ʱʹ�ÿ��¼�
        if (SIP::EVENT_TYPE_EXIT_THREAD == stEventInfo.enEventType)
        {
            //���¼�ͬ�����в���һ�����¼�
            lResult = m_objEventSyncQueue.pushBackEv(NULL);
            if (SIP::RET_CODE_OK != lResult)
            {
                ERROR_LOG("add event - failure to  push null event - event= %s",SIP::STR_ARR_EVENT_TYPE[stEventInfo.enEventType]);
            }
            else
            {
                if ( NULL != stEventInfo.pEventInfo )
                {
                    void* reclaim_p = stEventInfo.pEventInfo;
                    VOS_DELETE(reclaim_p);
                }
            }
            return lResult;
        }

        //�������������ʧ��
        ERROR_LOG("add event - failure to create event - event= %s",SIP::STR_ARR_EVENT_TYPE[stEventInfo.enEventType]);
        return SIP::RET_CODE_ALLOC_MEM_FAIL;
    }

    //�����¼���Ϣ
    *pstEventInfo = stEventInfo;

    //���¼�ͬ�����в����¼�
    lResult = m_objEventSyncQueue.pushBackEv(pstEventInfo);
    if (SIP::RET_CODE_OK != lResult)
    {
        //�ͷ���ʱ�ڴ�
        VOS_DELETE(pstEventInfo);

        ERROR_LOG("add event - push event - event= %s",SIP::STR_ARR_EVENT_TYPE[stEventInfo.enEventType]);

		return SIP::RET_CODE_FAIL;
    }

    INFO_LOG("Add Event To Synchronous Queue Success. %s.", SIP::STR_ARR_EVENT_TYPE[stEventInfo.enEventType]);
    return SIP::RET_CODE_OK;
}

/**
* Description:  GetTimerId().   ��ȡ��ʱ��ID
* @return       unsigned long.  ��ʱ��ID
*/
unsigned long CSipStack::GetTimerID()
{
    return ++m_ulTimerId;
}

/**
* Description:  HandleEventTimerTrigger().  ����ʱ�������¼�
* @return       void.
*/
void CSipStack::HandleEventTimerTrigger()
{
    unsigned long ulCurTimeTick = VOS_GetTicks();

    ACE_READ_GUARD(ACE_Recursive_Thread_Mutex, mapLocker, m_mutexTimerMap);

    for ( MAP_TIMER_ID_TO_INFO::iterator iter = m_mapTimerIdToInfo.begin();
          iter != m_mapTimerIdToInfo.end();
          ++iter
        )
    {
        SIP::TIMER_INFO& stTimerInfo = (SIP::TIMER_INFO)(iter->second);

        if ( stTimerInfo.bStop || stTimerInfo.ulTimerLength > ulCurTimeTick - stTimerInfo.ulStartTimeTick )
        {
            continue;
        }

        stTimerInfo.bStop = true;

        //ֹͣ��ʱ��
        SipStopTimer(stTimerInfo.ulTimerId);

        SS_UINT32 ulUser = 0;

        //��ȡģ��ID
        ulUser = ulUser | stTimerInfo.ulTimerPara >> SIP::TIMER_USER_RIGHT_SHIFT;
        switch (ulUser)
        {
        case SIP_COMP_TXN:
            {
                ACE_GUARD(ACE_Recursive_Thread_Mutex, locker, CSipStackTask::MiniSipMutex());
                SipTxnTimeoutHandler(stTimerInfo.ulTimerPara, stTimerInfo.ulTimerId);
                break;
            }

        default:
            {
                break;
            }
        }
    }
}

/**
* Description:  HandleEventTimerRegister(). ����ʱ��ע���¼�
* @param  [in]  pEventInfo      �¼���Ϣָ��
* @param  [in]  ulEventInfoLen  �¼���Ϣ���ݳ���
* @return       long.       ������
*/
long CSipStack::HandleEventTimerRegister
(
    void*           pEventInfo, 
    unsigned long   ulEventInfoLen
)
{
	if (VOS_NULL == pEventInfo || sizeof(SIP::EVENT_INFO_TIMER_REGISTER) != ulEventInfoLen )
	{
		ERROR_LOG("handle to register timer event -  failure - invalid param.");
		return SIP::RET_CODE_PARA_INVALIDATE;
	}

	SIP::EVENT_INFO_TIMER_REGISTER* pstEventInfo = (SIP::EVENT_INFO_TIMER_REGISTER*)pEventInfo;

	//����ӳ���
	ACE_WRITE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, mapLocker, m_mutexTimerMap, SIP::RET_CODE_FAIL);
	(void)m_mapTimerIdToInfo.insert( std::make_pair< unsigned long, SIP::TIMER_INFO >
												(
													pstEventInfo->ulTimerId,
													*pstEventInfo
												)
									);
	//if ( !ret.second )
	//{
	//    ERROR_LOG("handle to register timer event - failure to insert.";
	//}
	//m_mapTimerIdToInfo[pstEventInfo->ulTimerId] = *pstEventInfo;

	return SIP::RET_CODE_OK;
}

/**
* Description:  HandleEventTimerCancel().   ����ʱ��ȡ���¼�
* @param  [in]  pEventInfo      �¼���Ϣָ��
* @param  [in]  ulEventInfoLen  �¼���Ϣ���ݳ���
* @return       long.       ������
*/
long CSipStack::HandleEventTimerCancel
(
    void*           pEventInfo, 
    unsigned long   ulEventInfoLen
)
{
    if (VOS_NULL == pEventInfo)
    {
        ERROR_LOG("Handle Timer Register Event Failed. Event Info is Empty.");
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    if (sizeof(SIP::EVENT_INFO_TIMER_CANCEL) != ulEventInfoLen)
    {
        ERROR_LOG("Handle Timer Register Event Failed. Event Info Length(%d) Must be %d",ulEventInfoLen,sizeof(SIP::EVENT_INFO_TIMER_REGISTER));
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    SIP::EVENT_INFO_TIMER_CANCEL* pstEventInfo = (SIP::EVENT_INFO_TIMER_CANCEL*)pEventInfo;

    ACE_WRITE_GUARD_RETURN(ACE_Recursive_Thread_Mutex, mapLocker, m_mutexTimerMap, SIP::RET_CODE_FAIL)
    (void)m_mapTimerIdToInfo.erase(pstEventInfo->ulTimerId);

    return SIP::RET_CODE_OK;
}

/**
* Description:  HandleEventRedirect().  �����ض����¼�
* @param  [in]  pEventInfo      �¼���Ϣָ��
* @param  [in]  ulEventInfoLen  �¼���Ϣ���ݳ���
* @return       void.
*/
void CSipStack::HandleEventRedirect
(
    void*           pEventInfo, 
    unsigned long   ulEventInfoLen
)
{
    if (VOS_NULL == pEventInfo)
    {
        ERROR_LOG("Handle Redirect Event Failed. Event Info is Empty.");    
        return ;
    }

    if (sizeof(SIP_MSG_INFO) != ulEventInfoLen)
    {
        ERROR_LOG("Handle Redirect Event Failed. Event Info Length(%d) Must be %d",ulEventInfoLen,sizeof(SIP_MSG_INFO));
        return ;
    }

    SIP_MSG_INFO* pstMsgInfo = (SIP_MSG_INFO*)pEventInfo;

    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq      = pstMsgInfo->ulMsgSeq;
    stTextMsg.enTextMsgType = pstMsgInfo->enTextMsgType;
    stTextMsg.ulDialogId    = pstMsgInfo->ulDialogId;
    stTextMsg.pUserData     = pstMsgInfo->pstSipMsg;

    CSipStackMsgWrapRedirect objSipMsg(*this, stTextMsg);

    long lResult = VOS_FAIL;
    {
        ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, locker, CSipStackTask::MiniSipMutex(), ;);
        lResult = objSipMsg.SendMsg();//lint !e838
    }

    //�ض����ط���Ϣ�������ͷ�ԭ��Ϣ
    //SipDsmReleaseMsgRef(pstMsgInfo->pstSipMsg);

    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Send SIP Request Message Failed On Handle Redirect Event.");
        return ;
    }
}

/**
* Description:  GetDialogIdBySipMsg().    ����SIP��Ϣ�ҵ��Ի�ID
* @param  [in]      stSipMsg    SIP��Ϣ�ṹ������
* @param  [out] ulDialogId      �Ի�ID
* @param  [in]      bErase      �Ƿ�ɾ��
* @return       long.       ������
*/
long CSipStack::GetDialogIdBySipMsg
(
    SipMsgS&        stSipMsg,
    unsigned long&  ulDialogId,
    VOS_BOOLEAN     bErase
)
{
    //���캯���г�ʼ��
    SIP::DIALOG_INFO stDialogInfo;

    //��ȡ�Ի���Ϣ��������Ϣ����Զ��
    long lResult = SIP::GetDialogInfo(stSipMsg, stDialogInfo, VOS_FALSE);
    switch (lResult)
    {
    case SIP::RET_CODE_OK:  //�ɹ������
        {
            break;
        }

    case SIP::RET_CODE_NO_TO_TAG:   //��To��Tag���򷵻سɹ�
        {
            INFO_LOG("Get Dialog ID By SIP Message End. Invite Message No To Tag.");
            return SIP::RET_CODE_OK;
            //break;
        }

    default:    //��������ֱ�ӷ���ʧ��
        {
            WARN_LOG("Get Dialog Info Failed On Get Dialog ID By SIP Message.");
            return SIP::RET_CODE_FAIL;
            //break;
        }
    }

    //���ݶԻ���Ϣ�ҵ��Ի�ID
    lResult = GetDialogIdByInfo(stDialogInfo, ulDialogId, bErase);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Dialog ID By Dialog Info Failed On Get Dialog ID By SIP Message.");
        return SIP::RET_CODE_FAIL;
    }

    return SIP::RET_CODE_OK;
}

/**
* Description:  GetDialogIdBySipMsg().    ���ݶԻ���Ϣ�ҵ��Ի�ID
* @param  [in]      stDialogInfo    �Ի���Ϣ
* @param  [out] ulDialogId      �Ի�ID
* @param  [in]      bErase      �Ƿ�ɾ�������л�ͬʱm_mapDialogIdToInfoɾ����ӳ��
* @return       long.       ������
*/
long CSipStack::GetDialogIdByInfo
(
    const SIP::DIALOG_INFO& stDialogInfo,
    unsigned long&          ulDialogId,
    VOS_BOOLEAN             bErase
)
{
    //������
    CLockGuard objLocker(m_pMapMutex);

    MAP_DIALOG_INFO_TO_ID::iterator iter = find_if(m_mapDialogInfoToId.begin(),
        m_mapDialogInfoToId.end(),
        FindDialogInfo(stDialogInfo));
    //δ�ҵ�ƥ����������ش���
    if (iter == m_mapDialogInfoToId.end())
    {
		ERROR_LOG("Get Dialog ID By Dialog Info Failed. Dialog Info Invalidate. "
			"Call-ID = %s. LocalTag = %s. RemoteTag = %s.", stDialogInfo.strCallId.c_str(),
			stDialogInfo.strLocalTag.c_str(), stDialogInfo.strRemoteTag.c_str());
        return SIP::RET_CODE_FAIL;
    }

    //���öԻ� ID
    ulDialogId = (unsigned long)iter->second;

    if (bErase)
    {
        (void)m_mapDialogInfoToId.erase(iter);
        (void)m_mapDialogIdToInfo.erase(ulDialogId);
    }

	INFO_LOG("Get Dialog ID By Dialog Info Success. Dialog Info( DialogID = %d ): "
		"Call-ID = %s. LocalTag = %s. RemoteTag = %s.", ulDialogId, stDialogInfo.strCallId.c_str(),
		stDialogInfo.strLocalTag.c_str(), stDialogInfo.strRemoteTag.c_str());

    return SIP::RET_CODE_OK;
}//lint !e1788

/**
* Description:  GetDialogInfoById().      ���ݶԻ�ID�ҵ��Ի���Ϣ
* @param  [in]      ulDialogId      �Ի�ID
* @param  [out]     stDialogInfo    �Ի���Ϣ
* @return       long.       ������
*/
long CSipStack::GetDialogInfoById
(
    unsigned long       ulDialogId,
    SIP::DIALOG_INFO&   stDialogInfo
)
{
    //������
    CLockGuard objLocker(m_pMapMutex);

    //���ҶԻ���Ϣ
    MAP_DIALOG_ID_TO_INFO::iterator iter = m_mapDialogIdToInfo.find(ulDialogId);
    if (m_mapDialogIdToInfo.end() == iter)
    {
        ERROR_LOG("Get Dialog Info By ID Failed. Dialog ID(%d) Not Exist." ,ulDialogId);
        return SIP::RET_CODE_FAIL;
    }

    stDialogInfo = iter->second;
    return SIP::RET_CODE_OK;
}//lint !e1788

/**
* Description:  SetLocalAddr().  ���ñ��ص�ַ
* @return       void.
*/
void CSipStack::SetLocalAddr(const unsigned short local_port)
{
    sockaddr_in stAddr = {0};
    //socklen_t nAddrLen = sizeof(stAddr);
    //UDP��ʽֻ�ܻ�ȡ�˿�
    // comment by dhong
    /*   int nResult = getsockname((unsigned long)m_objSipUdpHandle.m_lSockFD, (struct sockaddr*)&stAddr, &nAddrLen);
    if (SOCKET_ERROR == nResult)
    {
    ERROR_LOG("getsockname Failed On Get Local Address. ErrorCode = %d.", CONN_ERRNO);    
    return;
    }*/
    int nResult=0;

    //���汾�ض˿�
    // modified by dhong
    m_objLocalAddr.m_usPort = htons(local_port);//stAddr.sin_port;

    //��ȡ����IP
    char szHostName[SIP::MAX_LEN_FILE_PATH] = {0};
    //��ȡ������
    nResult = gethostname(szHostName, sizeof(szHostName));//lint !e838
    if (SOCKET_ERROR == nResult)
    {
        ERROR_LOG("gethostname Failed On Get Local Address. ErrorCode = %d.",CONN_ERRNO);
        return;
    }

    //��ȡ������ַ
    struct hostent* pstHostent = NULL;
    pstHostent = gethostbyname(szHostName);   //lint !e838
    if (pstHostent)
    {
        stAddr.sin_addr.s_addr = *(unsigned long*)(void*)pstHostent->h_addr;
    }

    INFO_LOG("SIP Local Adress: IP = %.15s. Port = %d.", inet_ntoa(stAddr.sin_addr), ntohs(m_objLocalAddr.m_usPort));

    //���汾��IP��ַ
    m_objLocalAddr.m_lIpAddr = (long)stAddr.sin_addr.s_addr;
}//lint !e438

/**
* Description:  SetRegisterInfo().  ����ע����Ϣ
* @param  [in]  stRegisterInfo  ע����Ϣ
* @return       long.           ������
*/
long CSipStack::SetRegisterInfo(const SIP::REGISTER_INFO& stRegisterInfo)
{
	INFO_LOG("Set Register Info In Sip Stack Begin. LoginDomain = %.128s. "
		"LoginName = %.32s. LocalIP = %.16s. ProductName = %.128s. ProductVersion = %.64s. "
		"ExpireTime = %d. ServerInfoNum = %d.",
		stRegisterInfo.szLoginDomain, 
		stRegisterInfo.szLoginName, stRegisterInfo.szLocalIp, stRegisterInfo.szProductName, 
		stRegisterInfo.szProductVersion, stRegisterInfo.ulExpireTime, stRegisterInfo.ulServerInfoNum);

    // SetRegisterInfo��ʱ�򣬽�ֹ��������������κβ��������԰����������������
    // ���������Ӧ��Ƶ�����ã���Ҫ�Ż�  dhong
    CLockGuard objListServerLocker(m_pThreadlistSvrInfoMutex);
    CLockGuard objServerLocker(m_pThreadServerInfoMutex);

    //���ٱ�����һ����������Ϣ
    if (0 == stRegisterInfo.ulServerInfoNum)
    {
        ERROR_LOG("Set Register Info In Sip Stack Failed. Server Info Num Must Greater Than 0.");
        return SIP::RET_CODE_PARA_INVALIDATE;
    }

    //begin modify DTS2011042301313 ɾ����һ���û���¼����Ϣ

    m_listSvrInfoUsed.clear();
    m_vectorServerInfo.clear();

    //end of modify DTS2011042301313

    //����ע����Ϣ
    m_stRegisterInfo = stRegisterInfo;
    //ÿ�����µ�¼��������CallID
    m_strRegisterCallId = "";
    //�˴��������������Ϣ����m_objServerAddr�е�������
    m_stRegisterInfo.ulServerInfoNum = 0;

    //ʹ�����õı���IP
    if (0 != stRegisterInfo.szLocalIp[0])
    {
        //�������ñ���IP��ַ
        m_objLocalAddr.m_lIpAddr = (long)ACE_OS::inet_addr(stRegisterInfo.szLocalIp);
    }
    else
    {
        m_objLocalAddr.m_lIpAddr = (long)ACE_OS::inet_addr("127.0.0.1");
    }

    //�����һ����������IP�Ͷ˿ڣ�������
    m_objServerAddr.m_lIpAddr = (long)ACE_OS::inet_addr(stRegisterInfo.stServerInfoArray[0].szServerIP);
    m_objServerAddr.m_usPort  = htons(stRegisterInfo.stServerInfoArray[0].usServerPort);

    ///////////////////////////// add by w00207027 2012-11-02 Begin ///////////////////////////////////////////////////
    m_objServerAddrEx.m_lIpAddr =0;
    m_objServerAddrEx.m_usPort = 0;
    m_bServerAddrEx = false;
    ///////////////////////////// add by w00207027 2012-11-02 End /////////////////////////////////////////////////////

    //��¼ʹ�ù��ķ�������Ϣ
    {
        m_listSvrInfoUsed.push_back(stRegisterInfo.stServerInfoArray[0]);
    }

	INFO_LOG("Set Register Info In Sip Stack In. ServerInfo1 = %.16s:%d.", 
		stRegisterInfo.stServerInfoArray[0].szServerIP, 
		stRegisterInfo.stServerInfoArray[0].usServerPort);

    //�����ض����б�
    m_vectorServerInfo.push_back(stRegisterInfo.stServerInfoArray[0]);
    for (unsigned long ulIndex = 1; ulIndex < stRegisterInfo.ulServerInfoNum; ++ulIndex)
    {
        //����Ϊ�ض���
        m_bHasRedirect = VOS_TRUE;
        //����Ϊ��ʱ�ض���
        m_bTempRedirect = VOS_TRUE;   

        m_vectorServerInfo.push_back(stRegisterInfo.stServerInfoArray[ulIndex]);    //lint !e661 �ɱ�����
		INFO_LOG("Set Register Info In Sip Stack In. ServerInfo%d = %.16s:%d.", 
			ulIndex + 1,
			stRegisterInfo.stServerInfoArray[ulIndex].szServerIP,    //lint !e661 �ɱ�����
			stRegisterInfo.stServerInfoArray[ulIndex].usServerPort); //lint !e661 !e662 �ɱ�����
    } 

    //����������MD5ֵ
    Md5Hex(m_stRegisterInfo.szPassword, m_szPwdMd5);

    INFO_LOG("Set Register Info In Sip Stack Success.");
    return SIP::RET_CODE_OK;
}//lint !e1788

///////////////////////////////////////////  add by w00207012 2012-11-02 Begin //////////////////////////////////////////////////
/**
* Description:  SetRegisterInfo().  ����ע����Ϣ
* @param  [in]  bServerAddrEx  �Ƿ�ʹ�÷�������ַ
* @param  [in]  serverIP              ������IP
* @param  [in]  serverPort          ������Port
* @return       long.           ������
*/
long CSipStack::SetServerAddrEx(bool bServerAddrEx, const string& serverIP, unsigned short serverPort)
{
    //if ((serverIP.empty()) || (0==serverPort))
    //{
    //	INFO_LOG("Set ServerAddrEx Info In Sip Stack Failed, serverIP[%s] serverPort[%d].", serverIP.c_str(), serverPort;
    //	return SIP::RET_CODE_FAIL;
    //}

    if (bServerAddrEx)
    {
        //�����������IP�Ͷ˿ڣ�������
        m_objServerAddrEx.m_lIpAddr = (long)ACE_OS::inet_addr(serverIP.c_str());
        m_objServerAddrEx.m_usPort  = htons(serverPort);
    }

    m_bServerAddrEx = bServerAddrEx;

    //modify-cwx148380-begin
    m_strIP = serverIP;
    m_usPort = serverPort;
    //modify-cwx148380-end
    INFO_LOG("Set ServerAddrEx Info In Sip Stack Success.");
    return SIP::RET_CODE_OK;
}

/**
* Description:  isServerAddrEx().  �ж��Ƿ���ǰ�˽��������ڷ�װ��Ϣ
* @return       bool.
*/
bool CSipStack::IsServerAddrEx(void)
{
    return m_bServerAddrEx;
}
///////////////////////////////////////////  add by w00207012 2012-11-02 End ////////////////////////////////////////////////////

/**
* Description:  GetTuObjId().   ��ȡTU����ID
* @return       unsigned long.           TU����ID
*/
unsigned long CSipStack::GetTuObjId()
{
    return m_ulTuObjId++;
}

/**
* Description:  SendSipMsg().   ����Sip��Ϣ
* @param  [in]  stTextMsg   �ı���Ϣ
* @return       long.       ������
*/
long CSipStack::SendSipMsg(const SIP::TEXT_MSG& stTextMsg, const string& callID)
{
    //�����߳���
    //CLockGuard objLocker(m_pThreadMutex); //ͳһ��SendMsg�м��߳���

    //����ֵ
    long lResult = SIP::RET_CODE_OK;

    CSipStackMsgWrap* pSipMsg = VOS_NULL;

    try
    {
        switch ((long)stTextMsg.enTextMsgType)
        {
        case SIP::TEXT_MSG_TYPE_SIP_MESSAGE:
            {
                pSipMsg = new CSipStackMsgWrapMessage(*this, stTextMsg,callID);
                break;
            }

        case SIP::TEXT_MSG_TYPE_SIP_OPTIONS:
            {
                pSipMsg = new CSipStackMsgWrapOptions(*this, stTextMsg);
                break;
            }

        case SIP::TEXT_MSG_TYPE_SIP_INVITE:
            {
                pSipMsg = new CSipStackMsgWrapInvite(*this, stTextMsg, callID);
                break;
            }

        case SIP::TEXT_MSG_TYPE_SIP_ACK:
            {
                pSipMsg = new CSipStackMsgWrapAck(*this, stTextMsg);
                break;
            }

        case SIP::TEXT_MSG_TYPE_SIP_BYE:
            {
                pSipMsg = new CSipStackMsgWrapBye(*this, stTextMsg);
                break;
            }

        case SIP::TEXT_MSG_TYPE_SIP_REGISTER:
            {
                pSipMsg = new CSipStackMsgWrapRigister(*this, stTextMsg,callID);
                break;
            }

        case SIP::TEXT_MSG_TYPE_SIP_UNREGISTER:
            {
                pSipMsg = new CSipStackMsgWrapUnRigister(*this, stTextMsg, callID);
                break;
            }

            //             case SIP::TEXT_MSG_TYPE_SIP_INVITE_RSP:
            //                 {
            //                     pSipMsg = new CSipMsgInviteResp(*this, stTextMsg, msgSeq);
            //                     break;
            //                 }

        case SIP::TEXT_MSG_TYPE_SIP_SUBSCRIBE:
            {
                pSipMsg = new CSipStackMsgWrapSubscribe(*this, stTextMsg);
                break;
            }

        default:
            {
                pSipMsg = new CSipStackMsgWrap(*this, stTextMsg);
                break;
            }
        }
    }
    catch(...)
    {
        pSipMsg= VOS_NULL;

        ERROR_LOG("[ MsgSeq = %d] Create Sip Message Object Failed On Send SIP Message.",stTextMsg.ulMsgSeq);
        return SIP::RET_CODE_ALLOC_MEM_FAIL;//lint !e438
    }

    lResult = pSipMsg->SendMsg();//lint !e838

    //�ͷ���Ϣ��
	delete pSipMsg;
	pSipMsg = NULL;

    return lResult;
}

unsigned long CSipStack::GetCSeq()
{
    return ++m_ulCSeq;
}

void CSipStack::GetFromSipString(const SipString &sipString, string &str)
{
    if (NULL==sipString.pcData || 0==sipString.ulLen)
    {
        str = "";
        return ;
    }

    char buf[256]={0};
    unsigned long ulLen = sipString.ulLen;
    if (ulLen > 255)
        ulLen = 255;

    strncpy(buf, sipString.pcData, ulLen);
    buf[255] = '\0';

    str = buf;
}

SIP_HEADER *CSipStack::GetSipHeader(const SipMsgS &sipMsg, bool bReq)
{
    SIP_HEADER *pHeader = new SIP_HEADER;
	INFO_LOG("New SIP_HEADER Addr = [0X%x]",pHeader);

	// get sequence
    pHeader->seq = (unsigned long)sipMsg.stHeaders.pstCseq->ulSequence;

    // get firstLine
    if (!bReq)
    {
        pHeader->iStatusCode = (int)sipMsg.uFirstLine.stStatusLine.iSipStatusCode;       
    }

    // get callID
    GetFromSipString(*(sipMsg.stHeaders.pstCallID), pHeader->callID);

    return pHeader;
}

/**
* Description:  HandleSipTxnHiSfRspInd().   ������״̬Sip��Ӧ��Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]  stSipMsg    �յ���SIP��Ӧ��Ϣ
* @return       void. 
*/
void CSipStack::HandleSipTxnHiSfRspInd
(
    SS_UINT32            ulTuObjId,
    SS_UINT32            ulTxnObjId,
    const SipTptNwInfoS& stTptNwInf,
    SipMsgS&             stSipMsg
)
{
    ulTuObjId   = ulTuObjId;
    ulTxnObjId  = ulTxnObjId;

    if ( VOS_NULL == stSipMsg.stHeaders.pstCseq )
    {
        ERROR_LOG("handle sip stateful respond message - cseq is null.");
        return ;
    }


    //if ( VOS_SUCCESS != HandleRspResend(stSipMsg.stHeaders.pstCseq->stMethod.usTokenId, stTptNwInf, stSipMsg.stHeaders.pstCseq->ulSequence) )
    //{
    //    WARN_LOG("handle sip stateful respond message - failure to handle respond resend.");
    //    return;
    //}

    const unsigned long ulCSeq      = stSipMsg.stHeaders.pstCseq->ulSequence;
    const unsigned long ulRespCode  = (unsigned long)stSipMsg.uFirstLine.stStatusLine.iSipStatusCode;

    //modify-123456789-begin test
     if (!m_bRegister)
     {
         WARN_LOG("[ CSeq = %d] Has Unregistered On Handle SIP stateful Response Message.",ulCSeq);
         return;
     }

     if (501 == ulRespCode)
     {
        WARN_LOG("[ CSeq = %d ] Has Unregistered On Handle SIP stateful Response Message.",ulCSeq);
     }
    //modify-123456789-end test

     
    //ֱ�Ӷ�����ʱ��Ӧ��Ϣ
    if (SIP_STATUS_SUCC_OK > ulRespCode)
    {
        INFO_LOG("[ CSeq = %d ] Receive Temporary Response. RespCode = %d",ulCSeq,ulRespCode);
        return;
    }

    INFO_LOG("[ CSeq = %d ] Handle SIP Stateful Response Message Begin. Response Code is %d",ulCSeq,ulRespCode);

    //�Զ��庯������ֵ
    long lResult = SIP::RET_CODE_OK;

    SIP_MSG_INFO* pstMsgInfo = VOS_NULL;

    //��ȡSIP��Ϣ��Ϣ���������MAP��ɾ��
    lResult = GetSipMsgInfoByCSeq(ulCSeq, pstMsgInfo, VOS_TRUE);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        WARN_LOG("[ CSeq = %d ] Handle SIP Stateful Response Message End. Message Already Expired.",ulCSeq);
        return;
    }

    //֪ͨ��Ϣ������
    unsigned long ulRetCode = SIP::RET_CODE_OK;

    SIP::NOTIFY_TEXT_MSG_RESP stNotifyTextMsgResp = {0};
    //��ȡ��Ϣ���
    stNotifyTextMsgResp.stTextMsg.ulMsgSeq = pstMsgInfo->ulMsgSeq;

    unsigned long ulReason = 0;
    ulRetCode = (unsigned long)SIP::GetReason(stSipMsg,ulReason);//lint !e838
    if(ulRetCode == SIP::RET_CODE_OK)
    {
        stNotifyTextMsgResp.ulReason = ulReason;
    }

    // default retCode is OK. added by dhong
    ulRetCode = SIP::RET_CODE_OK;
    switch (ulRespCode)
    {
    case SIP_STATUS_SUCC_OK:
        // modified by dhong
        {
            switch ((long)pstMsgInfo->enTextMsgType)
            {
            case SIP::TEXT_MSG_TYPE_SIP_REGISTER:
            //case SIP::TEXT_MSG_TYPE_SIP_INVITE:
                {
                    ulRetCode = (unsigned long)HandleSuccessResp(stSipMsg, *pstMsgInfo, stNotifyTextMsgResp);
                    break;
                }
            default:
                break;
            }
            break;
        }

    case SIP_STATUS_CLIENT_ERR_UNAUTHORIZED:
        {
            //����δ��Ȩ��Ӧ
            lResult = HandleUnAuthResp(stSipMsg, *pstMsgInfo);

            //�ط��ɹ�����ֱ���˳�������
            if (SIP::RET_CODE_OK == lResult)
            {              
                //�ط���Ϣ����Ҫ�ͷ�ԭ��Ϣ
                //SipDsmReleaseMsgRef(&stMsgInfo.pstSipMsg);
                //�ͷ�SIP��Ϣ��Ϣ�ṹ��
                VOS_DELETE(pstMsgInfo);
                return;
            }

            ulRetCode = SIP::RET_CODE_AUTHENTICATE_ERROR;
            break;
        }

    case SIP_STATUS_CLIENT_ERR_REQUEST_TIMEOUT:
        {
            INFO_LOG("***********deal with timeout");
            ulRetCode = SIP::RET_CODE_REQUEST_TIME_OUT;
            break;
        }

    case SIP_STATUS_REDIR_MOVED_TEMPORARILY:
        {
            m_bTempRedirect = VOS_TRUE;
            INFO_LOG("***********deal with temporarily");
            //break;
        }
        //lint -fallthrough �˴�����ͬSIP_STATUS_REDIR_MOVED_PERMANENTLY�����������ض����б�
    case SIP_STATUS_REDIR_MOVED_PERMANENTLY:
        {
            //�ض���ʧ��ʱ�ķ�����
            ulRetCode = SIP::RET_CODE_FAIL;

            //��ȡ�ض�����Ϣ
            {
                CLockGuard objLocker(m_pThreadServerInfoMutex);
                lResult = SIP::GetRedirectInfo(stSipMsg, m_vectorServerInfo);
            }//lint !e1788
            if (SIP::RET_CODE_OK != lResult)
            {
                WARN_LOG("Get Redirect Info Failed On Handle SIP Stateful Response Message.");
            }

            //��Ҫ�ض���
            m_bHasRedirect = VOS_TRUE;

            break;
        }

    default:
        {
            ulRetCode = SIP::RET_CODE_FAIL;

            break;
        }
    }

    //��Ȩ��Ϣ�����������
    m_ulPlatAuthInfoChangeNum = 0;

    //�����ض�����
    if (HasRedirectEvent(*pstMsgInfo))
    {
        VOS_DELETE(pstMsgInfo);
        pstMsgInfo = NULL;
        INFO_LOG("[ CSeq = %d] Handle SIP Stateful Response Message Success. Have Added Redirect Event.",ulCSeq);
        return;//lint !e438
    }

    //����֪ͨ��Ϣ�ķ�����
    stNotifyTextMsgResp.ulRetCode = ulRetCode;
    stNotifyTextMsgResp.stTextMsg.enTextMsgType = pstMsgInfo->enTextMsgType;

    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, false);
    pHeader->sipMsg = g_recvStr;
    SIP::PEER_URI_INFO  stFromUri;
    memset(&stFromUri,0x0, sizeof(SIP::PEER_URI_INFO));
    SIP::GetFromUriInfo(stSipMsg, stFromUri);
    pHeader->from = stFromUri.szUriUserName;

    ////////////////////////////////////////// add by w00207027 2012-11-03 Begin ///////////////////////////////
    if ((SIP_STATUS_SUCC_OK==ulRespCode)
        && (SIP::TEXT_MSG_TYPE_SIP_INVITE == (long)pstMsgInfo->enTextMsgType))
    {
        char* pMsgBody = VOS_NULL;
        unsigned long ulMsgBodyLen = 0;

        //��ȡ��Ϣ��
        lResult = SIP::GetMsgBody(stSipMsg, pMsgBody, ulMsgBodyLen);
        if (SIP::RET_CODE_OK != lResult)
        {
            VOS_DELETE(pstMsgInfo);
            pstMsgInfo = NULL;
            ERROR_LOG("[ MsgSeq = %d] Get Message Body Failed On Handle SIP Invite Response Message.",ulCSeq);
            return ;//lint !e438
        }

        pHeader->pMsgBody = pMsgBody;

        unsigned long userSeq = InsertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, pstMsgInfo->ulDialogId);
        pHeader->userSeq  = userSeq;
   
    }

    //if ( SIP::TEXT_MSG_TYPE_SIP_MESSAGE == (long)pstMsgInfo->enTextMsgType )
    //{
    //    char* pMsgBody = VOS_NULL;
    //    unsigned long ulMsgBodyLen = 0;

    //    //��ȡ��Ϣ��
    //    lResult = SIP::GetMsgBody(stSipMsg, pMsgBody, ulMsgBodyLen);
    //    if (SIP::RET_CODE_OK != lResult)
    //    {
    //        ERROR_LOG("[ MsgSeq = %d ] Get Message Body Failed On Handle SIP Invite Response Message.", ulCSeq);
    //        return ;
    //    }

    //    pHeader->pMsgBody = pMsgBody;

    //    unsigned long userSeq = insertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, pstMsgInfo->ulDialogId);
    //    pHeader->userSeq  = userSeq; 
    //}

    ////////////////////////////////////////// add by w00207027 2012-11-03 End ////////////////////////////////

    // modified by dhong
    stNotifyTextMsgResp.stTextMsg.pUserData = pHeader;    
    INFO_LOG("HandleSipTxnHiSfRspInd sipHeader seq = %d, from = %s",pHeader->seq,pHeader->from.c_str());

    //�ϱ��ı���Ӧ��Ϣ
    lResult = NotifyTextMsgResp(stNotifyTextMsgResp);

    //�ͷ�SIP��Ϣ
    SipDsmReleaseMsgRef(&pstMsgInfo->pstSipMsg);
    //�ͷ�SIP��Ϣ��Ϣ�ṹ���ڴ�
    VOS_DELETE(pstMsgInfo);
    pstMsgInfo = NULL;
    //�ͷ�֪ͨ��Ϣ�е��ڴ�
    //VOS_DELETE((char*&)stNotifyTextMsgResp.stTextMsg.pszMsgBody, MULTI);
    //VOS_DELETE((char*&)stNotifyTextMsgResp.stTextMsg.pMsgHeader, MULTI);

    if (SIP::RET_CODE_OK != lResult)
    {
		ERROR_LOG("[ CSeq = %d ] Handle SIP Stateful Response Message Failed. "
			"Notify Text Message Response Has Error(0x%04X).", ulCSeq, lResult);
        return;
    }

    INFO_LOG("[ CSeq = %d ] Handle SIP Stateful Response Message Success.",ulCSeq);
}//lint !e438

/**
* Description:  HandleSuccessResp().     ����ɹ���Ӧ
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [in]  stReqMsgInfo  ������Ϣ��Ϣ����
* @param  [out] stNotifyResp    ֪ͨ�ı���Ӧ�ṹ������
* @return       long.       ������
*/
long CSipStack::HandleSuccessResp
(
    SipMsgS&                    stSipMsg,
    const SIP_MSG_INFO&         stReqMsgInfo,
    SIP::NOTIFY_TEXT_MSG_RESP&  stNotifyResp
)
{
    //�Զ��庯������ֵ
    long lResult = SIP::RET_CODE_OK;

    //����Nonce
    (void)SIP::GetNextNonce(stSipMsg, m_stPlatAuthInfo.strNonce);

    //�Ѳ���Ҫ�ض�����
    m_bHasRedirect = VOS_FALSE;

    switch ((long)stReqMsgInfo.enTextMsgType)
    {
    case SIP::TEXT_MSG_TYPE_SIP_REGISTER:
        {
            lResult = HandleSuccessRespRegister(stSipMsg, stNotifyResp);
            break;
        }

    case SIP::TEXT_MSG_TYPE_SIP_INVITE:
        {
            lResult = HandleSuccessRespInvite(stSipMsg, stReqMsgInfo, stNotifyResp);
            break;
        }

        // added by dhong
    case SIP::TEXT_MSG_TYPE_SIP_OPTIONS:
        // keep alive success
        {
            //do something to indicate that keep alive success
            break;
        }

    default:
        {

            break;
        }
    }

    return lResult;
}

/**
* Description:  HandleSuccessRespRegister().     ����Register�ɹ���Ӧ
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] stNotifyResp    ֪ͨ�ı���Ӧ�ṹ������
* @return       long.       ������
*/
long CSipStack::HandleSuccessRespRegister
(
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_RESP&  stNotifyResp
)
{
    unsigned long ulExpires = 0;
    long lResult = SIP::GetExpires(stSipMsg, ulExpires);
    if (SIP::RET_CODE_OK != lResult)
    {
        WARN_LOG("Get Expires Header Failed On Handle Register Success Response.");
        //û��Expireͷ��ֱ�Ӹ�ֵΪ0����ʾʹ��ԭֵ
        ulExpires = 0;

        //return SIP::RET_CODE_FAIL;
    }    

    //��������Ϣ����
    unsigned long ulSvrInfoNum = 0;

    if (m_bRedirected)
    {
        CLockGuard objLocker(m_pThreadServerInfoMutex);
        ulSvrInfoNum = m_vectorServerInfo.size();
    }//lint !e1788

	unsigned long ulHeaderRegLen = 0;
	if(0 != ulSvrInfoNum)
	{	
		if( ULONG_MAX-sizeof(SIP::RESP_HEADER_REGISTER) > (ulSvrInfoNum-1)*sizeof(SIP::SERVER_INFO) )
		{
			ulHeaderRegLen = sizeof(SIP::RESP_HEADER_REGISTER)+(ulSvrInfoNum-1)*sizeof(SIP::SERVER_INFO);
		}
		else
		{
			ERROR_LOG("ulSvrInfoNum is too large");
			return SIP::RET_CODE_FAIL;
		}
	}
	else
	{
		ulHeaderRegLen = sizeof(SIP::RESP_HEADER_REGISTER);
	}
	    
    char* pHeaderReg = VOS_NEW(pHeaderReg, ulHeaderRegLen);
    if (VOS_NULL == pHeaderReg)
    {
        ERROR_LOG("Create Register Header Failed On Handle Register Success Response.");
        return SIP::RET_CODE_FAIL;
    }
    memset(pHeaderReg, 0, ulHeaderRegLen);

    SIP::RESP_HEADER_REGISTER* pstHeaderReg = (SIP::RESP_HEADER_REGISTER*)pHeaderReg; //lint !e826 �ɱ�����

    pstHeaderReg->ulExpires = ulExpires;    //��ʱʱ��
    pstHeaderReg->bRedirect = m_bRedirected;//�Ƿ��ض���
    //�����Ȩ��ս�е�Opaque
    strncpy(pstHeaderReg->szOpaque, m_stPlatAuthInfo.strOpaque.c_str(), sizeof(pstHeaderReg->szOpaque) - 1);

    if (m_bRedirected)
    {
        //���÷�������Ϣ
        for (unsigned long ulIndex = 0; ulIndex < ulSvrInfoNum; ++ulIndex)
        {
            CLockGuard objLocker(m_pThreadServerInfoMutex);
            pstHeaderReg->stServerInfoArray[ulIndex] = m_vectorServerInfo[ulIndex];
        }//lint !e1788

        pstHeaderReg->ulServerInfoNum = ulSvrInfoNum;
    }

    //����֪ͨ��Ӧ����Ϣͷ��
    stNotifyResp.stTextMsg.pMsgHeader       = pHeaderReg;
    stNotifyResp.stTextMsg.ulMsgHeaderLen   = ulHeaderRegLen;

    INFO_LOG("Register Expires: Old Value = %d,. New Value = %d", m_stRegisterInfo.ulExpireTime,ulExpires);

    //�����µĳ�ʱʱ��
    m_stRegisterInfo.ulExpireTime = ulExpires;
    //����Ϊ��ע��
    INFO_LOG("-----------------------Register success here");
    m_bRegister = VOS_TRUE;

    return SIP::RET_CODE_OK; 
}

/**
* Description:  HandleSuccessRespInvite().     ����Invite�ɹ���Ӧ
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [in]  stReqMsgInfo    ������Ϣ��Ϣ����
* @param  [out] stNotifyResp    ֪ͨ�ı���Ӧ�ṹ������
* @return       long.       ������
*/
long CSipStack::HandleSuccessRespInvite
(
    SipMsgS&                    stSipMsg,
    const SIP_MSG_INFO&         stReqMsgInfo,
    SIP::NOTIFY_TEXT_MSG_RESP&  stNotifyResp
)
{
    INFO_LOG("[ MsgSeq = %d ] Handle SIP Invite Response Message Begin.",stReqMsgInfo.ulMsgSeq);

    long lResult = SIP::RET_CODE_OK;

    char* pMsgBody = VOS_NULL;
    unsigned long ulMsgBodyLen = 0;

    //��ȡ��Ϣ��
    lResult = SIP::GetMsgBody(stSipMsg, pMsgBody, ulMsgBodyLen);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Get Message Body Failed On Handle SIP Invite Response Message.",stReqMsgInfo.ulMsgSeq);
        return lResult;
    }

    stNotifyResp.stTextMsg.pszMsgBody   = pMsgBody;
    stNotifyResp.stTextMsg.ulMsgBodyLen = ulMsgBodyLen;

	//SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(stNotifyResp.stTextMsg., false);

    //����ACK��Ϣ����
    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq      = stReqMsgInfo.ulMsgSeq;
    stTextMsg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_ACK;
    stTextMsg.ulDialogId    = stReqMsgInfo.ulDialogId;
    stTextMsg.pUserData     = &stSipMsg;

    ////���캯���г�ʼ��
    //SIP::DIALOG_INFO stDialogInfo;

    ////��ȡ�Ի���Ϣ��������Ϣ����Զ��
    //lResult = SIP::GetDialogInfo(stSipMsg, stDialogInfo, VOS_FALSE);
    //if (SIP::RET_CODE_OK != lResult)
    //{
    //	ERROR_LOG("Get Dialog Info Failed On Receive Invite Response Message.");
    //	return lResult;
    //}

    ////�����ڷ�����Ϣǰ��ӵ�ӳ����У�������Ӧ��Ϣ����ͻ��Ҳ����öԻ�
    ////���������ӳ���֮�����ã����ӳ��ʱ���п���
    //stDialogInfo.bRecvOK = VOS_TRUE;

    //(void)VOS_MutexLock(m_pMapMutex);
    ////��ӶԻ�ID��Ϣ��ӳ��
    //// modified by dhong 
    //// because find invite has no dialogID
    //
    ////if (0 != pstMsgInfo->ulDialogId)
    ////{
    //	m_mapDialogInfoToId[stDialogInfo] = stReqMsgInfo.ulDialogId;
    //	m_mapDialogIdToInfo[stReqMsgInfo.ulDialogId] = stDialogInfo;
    ////}
    //(void)VOS_MutexUnlock(m_pMapMutex);

    CSipStackMsgWrapAck objSipMsgAck(*this, stTextMsg);

    //����ACK
    lResult = objSipMsgAck.SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d] Send ACK Message Failed On Handle SIP Invite Response Message.",stReqMsgInfo.ulMsgSeq);
        return lResult;
    }

    INFO_LOG("[ MsgSeq = %d] Handle SIP Invite Response Message Success.",stReqMsgInfo.ulMsgSeq);
    return SIP::RET_CODE_OK;
}

/**
* Description:  HasRedirect().  ����Ƿ����ض����¼�
* @param  [in]  stSipMsgInfo    SIP��Ϣ��Ϣ����
* @return       VOS_BOOLEAN.    �Ƿ�  ���ض���
*/
VOS_BOOLEAN CSipStack::HasRedirectEvent
(
    const SIP_MSG_INFO& stSipMsgInfo
)
{
    //�����ض�����
    if (m_bHasRedirect)
    {
        m_bHasRedirect = VOS_FALSE;
        {
            CLockGuard objLocker(m_pThreadServerInfoMutex);

            for (unsigned long ulIndex = 0; ulIndex < m_vectorServerInfo.size(); ++ulIndex)
            {
                {
                    CLockGuard listobjLocker(m_pThreadlistSvrInfoMutex);

                    SIP::SERVER_INFO& stSvrInfo = m_vectorServerInfo[ulIndex];
                    LIST_SVR_INFO::iterator iter1 = find_if(m_listSvrInfoUsed.begin(),  m_listSvrInfoUsed.end(), FindServerInfo(stSvrInfo));
                    if (iter1 != m_listSvrInfoUsed.end())
                    {
                        continue;
                    }

                    m_listSvrInfoUsed.push_back(m_vectorServerInfo[ulIndex]);
                }//lint !e1788


                //�����һ����������IP�Ͷ˿ڣ�������
                m_objServerAddr.m_lIpAddr = (long)ACE_OS::inet_addr(m_vectorServerInfo[ulIndex].szServerIP);
                m_objServerAddr.m_usPort  = htons(m_vectorServerInfo[ulIndex].usServerPort);

                m_bHasRedirect  = VOS_TRUE;

                //�����ض��򣬲�����Ϊ�ض����
                if (!m_bTempRedirect)
                {
                    m_bRedirected   = VOS_TRUE; //���ض����
                }

                //�ҵ�ֱ���˳�
                break;
            }
        }//lint !e1788

        if (m_bHasRedirect)
        {
            SIP::EVENT_INFO stEventInfo(SIP::EVENT_TYPE_STACK_REDIRECT, (void*)&stSipMsgInfo, sizeof(stSipMsgInfo));//lint !e1773
            //����ض����¼�
            long lResult = AddEvent(stEventInfo);
            if (SIP::RET_CODE_OK != lResult)
            {
                WARN_LOG("Add Redirect Event Failed On Handle SIP Stateful Response Message.");
                m_bHasRedirect = VOS_FALSE;
            }
        }
    }

    return m_bHasRedirect;
}

/**
* Description:  HandleUnAuthResp().     ����δ��Ȩ��Ӧ
* @param  [in]  stSipMsg            SIP��Ϣ����
* @param  [in]  stReqMsgInfo  ������Ϣ��Ϣ����
* @return       long.       ������
*/
long CSipStack::HandleUnAuthResp
(
    SipMsgS&        stSipMsg,
    SIP_MSG_INFO&   stReqMsgInfo,
    bool            bFirst
)
{
    long lResult = SIP::RET_CODE_OK;

    //��ȡ��Ȩ��Ϣ
    lResult = SIP::GetPlatAuthInfo(stSipMsg, m_stPlatAuthInfo);//lint !e838

    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Plat Authentication Info Failed On Handle UnAuthentication Response.");
        return lResult;
    }

    //ͬһ�μ�Ȩ��Ϣ���ܳ���ָ������
    if (SIP::MAX_NUM_AUTH_INFO_CHANGE < m_ulPlatAuthInfoChangeNum)
    {
        ERROR_LOG("Handle UnAuthentication Response Failed. Plat Authentication Info Changed More Than %d",SIP::MAX_NUM_AUTH_INFO_CHANGE); 
        return SIP::RET_CODE_AUTHENTICATE_ERROR;
    }

    //��Ȩ��Ϣ��������ۼ�
    m_ulPlatAuthInfoChangeNum++;

    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq      = stReqMsgInfo.ulMsgSeq;
    stTextMsg.enTextMsgType = stReqMsgInfo.enTextMsgType;
    stTextMsg.ulDialogId    = stReqMsgInfo.ulDialogId;
    stTextMsg.pUserData     = stReqMsgInfo.pstSipMsg;

    CSipStackMsgWrapReAuth objSipMsg(*this, stTextMsg);

    lResult = objSipMsg.SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Send Request Message With Authentication Failed On Handle UnAuthentication Response Message.");
        return lResult;
    }


    return lResult;
}//lint !e1764

/**
* Description:  HandleUnAuthResp().     ����δ��Ȩ��Ӧ
* @param  [in]  stSipMsg            SIP��Ϣ����
* @param  [in]  stReqMsgInfo  ������Ϣ��Ϣ����
* @return       long.       ������
*/
// not used now added by dhong
long CSipStack::HandleReRegister(void)
{
    return 0;

    //     long lResult = SIP::RET_CODE_OK;
    // 
    //     //��ȡ��Ȩ��Ϣ
    //     //    lResult = SIP::GetPlatAuthInfo(m_authMsg, m_stPlatAuthInfo);
    // 
    //     if (SIP::RET_CODE_OK != lResult)
    //     {
    //         ERROR_LOG("Get Plat Authentication Info Failed On Handle Unauthentication Response.");
    //         return lResult;
    //     }
    // 
    //     //ͬһ�μ�Ȩ��Ϣ���ܳ���ָ������
    //     if (SIP::MAX_NUM_AUTH_INFO_CHANGE < m_ulPlatAuthInfoChangeNum)
    //     {
    //         ERROR_LOG("Handle Unauthentication Response Failed. Plat Authentication Info Changed More Than %d.", 
    //                 SIP::MAX_NUM_AUTH_INFO_CHANGE);
    //         return SIP::RET_CODE_AUTHENTICATE_ERROR;
    //     }
    // 
    //     //��Ȩ��Ϣ��������ۼ�
    //     m_ulPlatAuthInfoChangeNum++;
    // 
    //     SIP::TEXT_MSG stTextMsg = {0};
    //     stTextMsg.ulMsgSeq      = m_msgInfo.ulMsgSeq;
    //     stTextMsg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_REGISTER;//stReqMsgInfo.enTextMsgType;
    //     stTextMsg.ulDialogId    = m_msgInfo.ulDialogId;
    //     stTextMsg.pUserData     = m_msgInfo.pstSipMsg;
    // 
    //     CSipMsgReAuth objSipMsg(*this, stTextMsg);
    // 
    //     lResult = objSipMsg.SendMsg();
    //     if (SIP::RET_CODE_OK != lResult)
    //     {
    //         ERROR_LOG("Send Request Message With Authenication Failed "
    //                 "On Handle Unauthentication Response Message.");
    //         return lResult;
    //     }
    // 
    //     return lResult;
}

// added by dhong
long CSipStack::HandleAuthOnLineResp
(
    const SipMsgS&        stSipMsg,
    SIP_MSG_INFO&   stReqMsgInfo
)
{
    long lResult = SIP::RET_CODE_OK;

    //......
    /*
    lResult = SIP::GetPlatAuthInfo(stSipMsg, m_stPlatAuthInfo);

    if (SIP::RET_CODE_OK != lResult)
    {
    ERROR_LOG("Get Plat Authentication Info Failed On Handle Unauthentication Response.");
    return lResult;
    }
    */

    //
    if (SIP::MAX_NUM_AUTH_INFO_CHANGE < m_ulPlatAuthInfoChangeNum)
    {
        ERROR_LOG("Handle Unauthentication Response Failed. Plat Authentication Info Changed More Than %d",SIP::MAX_NUM_AUTH_INFO_CHANGE); 
        return SIP::RET_CODE_AUTHENTICATE_ERROR;
    }

    //..........
    m_ulPlatAuthInfoChangeNum++;

    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq      = stReqMsgInfo.ulMsgSeq;
    stTextMsg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_REGISTER;//stReqMsgInfo.enTextMsgType;
    stTextMsg.ulDialogId    = stReqMsgInfo.ulDialogId;
    stTextMsg.pUserData     = stReqMsgInfo.pstSipMsg;

    CSipStackMsgWrapReAuth objSipMsg(*this, stTextMsg);

    lResult = objSipMsg.SendMsg();//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Send Request Message With Authenication Failed  On Handle Unauthentication Response Message.");
        return lResult;
    }

    return lResult;
}//lint !e1764
/**
* Description:  HandleSipTxnHiSfReqInd().   ������״̬Sip������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]  stSipMsg    �յ���SIP������Ϣ
* @return       void.
*/
// dhong modified tomorrow here 2012-5-16
void CSipStack::HandleSipTxnHiSfReqInd
(
    SS_UINT32            ulTuObjId,
    SS_UINT32            ulTxnObjId,
    const SipTptNwInfoS& stTptNwInf,
    SipMsgS&             stSipMsg
)
{
    if ( VOS_NULL == stSipMsg.stHeaders.pstCseq )
    {
        ERROR_LOG("handle sip stateful request message - cseq is null.");
        return ;
    }
	
    if ( SS_CTRL_MSG_MGR_INS_REF.IsControl2Drop(stSipMsg) )
    {
        WARN_LOG("handle sip stateful request message - message control to drop.");
        //������Ӧ��Ϣ
        CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
        //������Ӧ��Ϣ�����账����ֵ
        (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SERVER_ERR_SERVICE_UNAVAILABLE, ulTuObjId, ulTxnObjId);
        return ;
    }

    INFO_LOG("handle sip stateful request message - begin - tu_id=%d,txn_id=%d",ulTuObjId,ulTxnObjId);

    long lResult = SIP::RET_CODE_OK;

    const unsigned long ulSipMethod = stSipMsg.uFirstLine.stRequestLine.stMethod.usTokenId;
    if (SIP_METHOD_BUTT <= ulSipMethod)
    {
        ERROR_LOG("handle sip stateful request message - unsupported sip method - method=%d",ulSipMethod);
        return;
    }

    //�ı���Ϣ����
    SIP::NOTIFY_TEXT_MSG_REQ stNotifyTextMsgReq = {0};
    stNotifyTextMsgReq.ulMsgSeq                 = stSipMsg.stHeaders.pstCseq->ulSequence;
    stNotifyTextMsgReq.enTextMsgType            = SIP::ARR_SIP_METHOD_TO_TEXT_TYPE[ulSipMethod];
	stNotifyTextMsgReq.pUserData = NULL;

    switch (ulSipMethod)
    {
    case SIP_METHOD_REGISTER:
        {
            lResult = HandleReqRegister(ulTuObjId, ulTxnObjId, stTptNwInf, stSipMsg, stNotifyTextMsgReq);
			if(NULL != stNotifyTextMsgReq.pUserData)
			{
				SipStack::SIP_HEADER *pHeader = (SipStack::SIP_HEADER*)(stNotifyTextMsgReq.pUserData);
				INFO_LOG("delete pHeader = [0X%x]",pHeader);
				delete pHeader;
				pHeader = NULL;
			}
            break;
        }

    case SIP_METHOD_NOTIFY:
        {
        	lResult = HandleReqNotify(ulTuObjId, ulTxnObjId, stTptNwInf, stSipMsg, stNotifyTextMsgReq);
        	break;
        }

    case SIP_METHOD_OPTIONS:
        {
            lResult = HandleReqOption(ulTuObjId, ulTxnObjId, stTptNwInf, stSipMsg, stNotifyTextMsgReq);
            break;
        }

    case SIP_METHOD_INVITE:
        {   
            lResult = HandleReqInvite(ulTuObjId, ulTxnObjId, stSipMsg, stNotifyTextMsgReq);
            break;
        }

    case SIP_METHOD_MESSAGE:
        {
            lResult = HandleReqMessage(ulTuObjId, ulTxnObjId, stTptNwInf, stSipMsg, stNotifyTextMsgReq);
            break;
        }

        // Cance ��Byeһ���� added by dhong
    case SIP_METHOD_BYE: 
    case SIP_METHOD_CANCEL:
        {
            lResult = HandleReqBye(ulTuObjId, ulTxnObjId, stSipMsg, stNotifyTextMsgReq);
            break;
        }

    default:
        {
			//������Ӧ��Ϣ
			CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
			//������Ӧ��Ϣ�����账����ֵ
			(void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
			break;
        }
    }

    if (SIP::RET_CODE_OK != lResult)
    {
        WARN_LOG("handle sip stateful request message - failure to handle - Error= %d",lResult);
        return;
    }

    INFO_LOG("handle sip stateful request message - end");
}

/**
* Description:  HandleSipTxnHiSlReqInd().   ������״̬Sip������Ϣ
* @param  [in]  stSipMsg    �յ���SIP������Ϣ
* @return       void.
*/
void CSipStack::HandleSipTxnHiSlReqInd
(
    const SipTptNwInfoS& stTptNwInf,
    SipMsgS&             stSipMsg
)
{
    if ( VOS_NULL == stSipMsg.stHeaders.pstCseq )
    {
        ERROR_LOG("handle sip stateless request message - cseq is null.");
        return ;
    }
    //if ( VOS_SUCCESS != HandleReqResend(stSipMsg.uFirstLine.stRequestLine.stMethod.usTokenId, stTptNwInf, stSipMsg.stHeaders.pstCseq->ulSequence) )
    //{
    //    WARN_LOG("handle sip stateless request message - failure to handle resend.");
    //    return;
    //}

    INFO_LOG("handle sip stateless request message - begin.");

    long lResult = SIP::RET_CODE_OK;

    const unsigned long ulSipMethod = stSipMsg.uFirstLine.stRequestLine.stMethod.usTokenId;
    const unsigned long ulCSeq = stSipMsg.stHeaders.pstCseq->ulSequence;

    if (SIP_METHOD_BUTT <= ulSipMethod)
    {
        ERROR_LOG("handle sip stateless request message - unsupported sip method - method=%d",ulSipMethod);
        return;
    }

    //�ı���Ϣ����
    const SIP::TEXT_MSG_TYPE enTextMsgType = SIP::ARR_SIP_METHOD_TO_TEXT_TYPE[ulSipMethod];

    SIP::NOTIFY_TEXT_MSG_REQ stNotifyTextMsgReq = {0};
    stNotifyTextMsgReq.ulMsgSeq         = ulCSeq;
    stNotifyTextMsgReq.enTextMsgType    = enTextMsgType;
	stNotifyTextMsgReq.pUserData = NULL;

    lResult = GetDialogIdBySipMsg(stSipMsg, stNotifyTextMsgReq.ulDialogId);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        //invite's ack has no dialog ID
        INFO_LOG("handle sip stateless request message - can't find dialog id.");
    }

    // added by dhong
    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);
    pHeader->sipMsg = g_recvStr;
    // modified by dhong
    stNotifyTextMsgReq.pUserData = pHeader;

    //�ϱ��ı���Ӧ��Ϣ
    lResult = NotifyTextMsgReq(stNotifyTextMsgReq);
	stNotifyTextMsgReq.pUserData = NULL;
	INFO_LOG("delete pHeader = [0X%x]",pHeader); 
	delete pHeader;		
	pHeader = NULL;


    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("handle sip stateless request message - failure to handle - Error= %d",lResult);
        return;
    }

    INFO_LOG("handle sip stateful request message - end");
}

//////////////////////////////////////// add by wx153027 2012-12-18 begin  ///////////////////////////////////
/**
* Description:  HandleReqOption().  ����Option������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
* @return       long.       ������
*/
long CSipStack::HandleReqOption
(
    SS_UINT32                   ulTuObjId,
    SS_UINT32                   ulTxnObjId,
    const SipTptNwInfoS&        stTptNwInf,
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
)
{
    INFO_LOG("handle option request - begin.");

    unsigned long userSeq = InsertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, stNotifyReq.ulDialogId);

    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);

    if ( VOS_NULL == pHeader )
    {
        ERROR_LOG("handle options request - header is null.");
        return VOS_FAIL;
    }

    pHeader->userSeq = userSeq;
    if ( VOS_SUCCESS != GetAddrWithSipTptIpPortS(stTptNwInf.stSrcAddr, pHeader->devIP, pHeader->devPort) )
    {
        ERROR_LOG("handle options request - failure to get address info");
        return VOS_FAIL;
    }
    //SipVia* pstSipVia = NULL;
    //long nResult = VppListGetData(stSipMsg.stHeaders.hdlViaList, 0, (void**)&pstSipVia);
    //pHeader->devPort = pstSipVia->stSentBy.iPort;
    //{
    //    ACE_INET_Addr addr;
    //    if (SIP_ADDR_TYPE_IPV4 == pstSipVia->stSentBy.stHost.enHostType)
    //    {
    //        SS_UINT8 pIP4 [SS_IPV4_ADDR_LEN];
    //        for (int i = 0;i<SS_IPV4_ADDR_LEN;i++)
    //        {
    //            pIP4[i] = pstSipVia->stSentBy.stHost.uHostContent.ipv4[SS_IPV4_ADDR_LEN-1-i];
    //        }
    //        addr.set_address((char*)pIP4, SS_IPV4_ADDR_LEN);
    //    }
    //    else if (SIP_ADDR_TYPE_IPV6 == pstSipVia->stSentBy.stHost.enHostType)
    //    {
    //        SS_UINT8 pIP6 [SS_IPV6_ADDR_LEN];
    //        for (int i = 0; i<SS_IPV6_ADDR_LEN; i++)
    //        {
    //            pIP6[i] = pstSipVia->stSentBy.stHost.uHostContent.ipv6[SS_IPV6_ADDR_LEN-1-i];
    //        }
    //        addr.set_address((char*)pIP6, SS_IPV6_ADDR_LEN);
    //    } 
    //    else
    //    {
    //        ERROR_LOG("Get Contact URI's IP Info Failed. SIP URI is Empty.");
    //        return SIP::RET_CODE_FAIL;
    //    }
    //    pHeader->devIP = addr.get_host_addr();
    //}

    SIP::PEER_URI_INFO  stFromUri;
    memset(&stFromUri,0x0, sizeof(SIP::PEER_URI_INFO));
    SIP::GetFromUriInfo(stSipMsg, stFromUri);
    pHeader->from = stFromUri.szUriUserName;
    pHeader->sipMsg = g_recvStr;
    stNotifyReq.pUserData = pHeader;

    INFO_LOG("handle option request - get uri from - From= %s",pHeader->from.c_str());

    //�ϱ��ı���Ӧ��Ϣ
    long lResult = NotifyTextMsgReq(stNotifyReq);


    return lResult;
}
//////////////////////////////////////// add by wx153027 2012-12-18 end  ///////////////////////////////////

//////////////////////////////////////// add by w00207027 2012-10-27 Begin  ///////////////////////////////////
/**
* Description:  HandleReqRegister().  ����Register������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
* @return       long.       ������
*/
long CSipStack::HandleReqRegister
(
    SS_UINT32                   ulTuObjId,
    SS_UINT32                   ulTxnObjId,
    const SipTptNwInfoS&        stTptNwInf,
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
)
{
    INFO_LOG("handle register request - begin.");

    SipAuthorization * pSipAuthorization = NULL;

	if(0 == strlen(m_stRegisterInfo.szRemoteLoginName))
	{
		CSipStackMsgWrapRegisterResp objSipRspMsg(*this, stSipMsg, true);
		(void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
		INFO_LOG("handle register request - no authenticate header.");
		return SIP::RET_CODE_OK;
	}

    if(  ( SIP::RET_CODE_OK != CheckRegistAuthInfo(stSipMsg, pSipAuthorization) )
      || ( NULL == pSipAuthorization )
      || ( !pSipAuthorization->bIsAuthDigestType )
      )
    {
        CSipStackMsgWrapRegisterResp objSipRspMsg(*this, stSipMsg, true);
        (void)objSipRspMsg.SendRspMsg(SIP_STATUS_CLIENT_ERR_UNAUTHORIZED, ulTuObjId, ulTxnObjId);
        INFO_LOG("handle register request - can't find authenticate header .");
        return SIP::RET_CODE_FAIL;
    }

    long lResult = SIP::RET_CODE_FAIL;

    // ���յ�ժҪ��֤��Ϣ������Ϣ�ϱ�ҵ��㴦��
    unsigned long ulUserSeq = InsertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, stNotifyReq.ulDialogId);

    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);
    pHeader->userSeq = ulUserSeq;
    SIP::GetExpires(stSipMsg, pHeader->expires);
    //SIP::GetContact(stSipMsg, pHeader->devIP, pHeader->devPort);
    if ( VOS_SUCCESS != GetAddrWithSipTptIpPortS( stTptNwInf.stSrcAddr,pHeader->devIP,pHeader->devPort) )
    {
        ERROR_LOG("handle register request - failure to get address info.");
        return VOS_FAIL;
    }
    SIP::PEER_URI_INFO  stFromUri;
    memset(&stFromUri,0x0, sizeof(SIP::PEER_URI_INFO));
    SIP::GetFromUriInfo(stSipMsg, stFromUri	);
    pHeader->from = stFromUri.szUriUserName;	
    INFO_LOG("Handle REGISTER Message sipHeader from = %s",pHeader->from.c_str());
    GetStringFromQuoteString(*(pSipAuthorization->pstrUsername), pHeader->digestUserName);
    GetStringFromQuoteString(*(pSipAuthorization->pstrRealm), pHeader->digestRealm);
    GetStringFromQuoteString(*(pSipAuthorization->pstrNonce), pHeader->digestNonce);
    GetStringFromQuoteString(*(pSipAuthorization->pstrDigestUri), pHeader->digestUri);
    GetStringFromQuoteString(*(pSipAuthorization->pstrDResponse), pHeader->digestResponse);
    //getFromSipString(*(sipAuthorization->pstrOpaque), pHeader->digestOpaque);
    //getFromSipString(*(sipAuthorization->pstrCNonce), pHeader->digestCNonce);
    //getFromSipString(*(sipAuthorization->pstrNonceCount), pHeader->digestNonceCount);
    //GetStringFromQuoteString(*(pSipAuthorization->pstrMessageQOP), pHeader->digestQop);
    //pHeader->digestUserName = "34020000001230000001";
    //pHeader->digestRealm = "huawei.com";
    //pHeader->digestNonce = "oc1mA0GPoEjkOhZ1sjDFhlkezSw8TIK7";
    //pHeader->digestUri = "sip:340200000020000000001@3402000000";
    //modify-123456789-cwx148380-begin
    if ( NULL != pSipAuthorization->pstrOpaque )
    {
        GetStringFromQuoteString(*(pSipAuthorization->pstrOpaque), pHeader->digestOpaque);
    }
    
    if ( NULL != pSipAuthorization->pstrCNonce )
    {
        GetStringFromQuoteString(*(pSipAuthorization->pstrCNonce), pHeader->digestCNonce);
    }
    
    if ( NULL != pSipAuthorization->pstrNonceCount )
    {
        GetFromSipString(*(pSipAuthorization->pstrNonceCount), pHeader->digestNonceCount);
    }
    
    if ( NULL != pSipAuthorization->pstrMessageQOP )
    {
         GetFromSipString(*(pSipAuthorization->pstrMessageQOP), pHeader->digestQop);
    }
   
    //��algorithmΪ�գ���Ĭ��ʹ��MD5�㷨��
    if ( NULL != pSipAuthorization->pstrAlgorithm )
    {
        GetStringFromQuoteString(*(pSipAuthorization->pstrAlgorithm), pHeader->digestAlgorithm);
    }
    else
    {
        pHeader->digestAlgorithm = "MD5";
    }

    //����Response
    HASHHEX HA1;
    HASHHEX Response;

    DigestCalcHA1((char*)"",//lint !e1773
                  m_stRegisterInfo.szRemoteLoginName, (char*)pHeader->digestRealm.c_str(), //lint !e1773
    	          m_stRegisterInfo.szRemotePassword, (char*)"", (char*)"", HA1);//lint !e1773
    DigestCalcResponse(HA1, (char*)pHeader->digestNonce.c_str(), (char*)pHeader->digestNonceCount.c_str(),//lint !e1773
        (char*)pHeader->digestCNonce.c_str(), //lint !e1773
    	(char*)pHeader->digestQop.c_str(), (char*)"REGISTER",//lint !e1773
    	(char*)pHeader->digestUri.c_str(), (char*)"", Response);//lint !e1773
	std::string strResponse(Response);
	if(strResponse == pHeader->digestResponse)
	{
		CSipStackMsgWrapRegisterResp objSipRspMsg(*this, stSipMsg, true);
		(void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
		INFO_LOG("handle register request - 200 OK .");
		return SIP::RET_CODE_OK;
	}
	else
	{
		CSipStackMsgWrapRegisterResp objSipRspMsg(*this, stSipMsg, true);
		(void)objSipRspMsg.SendRspMsg(SIP_STATUS_CLIENT_ERR_FORBIDDEN, ulTuObjId, ulTxnObjId);
		WARN_LOG("Receive Response[%s] is not equal %s",pHeader->digestResponse.c_str(),strResponse.c_str());
		return SIP::RET_CODE_FAIL;
	}

    pHeader->sipMsg = g_recvStr;
    stNotifyReq.pUserData = pHeader;

    //�ϱ��ı���Ӧ��Ϣ
    lResult = NotifyTextMsgReq(stNotifyReq);//lint !e838

    if (SIP::RET_CODE_OK != lResult)
    {
        CSipStackMsgWrapRegisterResp objSipRspMsg(*this, stSipMsg, false);
        (void)objSipRspMsg.SendRspMsg(SIP_STATUS_CLIENT_ERR_BAD_REQUEST, ulTuObjId, ulTxnObjId);
        ERROR_LOG("handle register request - bad request - Error= %d",lResult);
        return SIP::RET_CODE_FAIL;
    }

    INFO_LOG("handle register request - end.");
    return SIP::RET_CODE_OK;
}


/**
* Description:  getStringFromQuoteString().  У��Regist�����Authenticate��Ϣ
* @param  [in]      sipString           SIP��Ϣ����
* @param  [out]    str                      �����Quote��string
* @return    long   .       ������
*/
long  CSipStack::GetStringFromQuoteString(const SipString &sipString, string &str)
{
    if (NULL==sipString.pcData || 0==sipString.ulLen)
    {
        ERROR_LOG("Fail to parse string from SIP's Quote String.");
        return SIP::RET_CODE_FAIL;
    }

    //ȥ��ĩβ˫����
    sipString.pcData[sipString.ulLen -1] = '\0';

    //����buffer����󳤶�
    unsigned long ulBufLen =sipString.ulLen;

    //���Ͻ������ĳ���
    ulBufLen += 1;

    char* pBuffer = VOS_NEW(pBuffer, ulBufLen);
    if (VOS_NULL == pBuffer)
    {
        ERROR_LOG("Create Buffer Failed On SIP's Quote String.");
        return SIP::RET_CODE_FAIL;
    }

    //������ַ���������
    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, sipString.pcData + 1, sipString.ulLen - 2);
    str = pBuffer;

    //�ͷ�buffer
    VOS_DELETE(pBuffer, MULTI);

    return SIP::RET_CODE_OK;
}

/**
* Description:  CheckRegistAuthInfo().  У��Regist�����Authenticate��Ϣ
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out]  sipAuthorization   Regist�����Authenticate��Ϣ
* @return       long.       ������
*/
long CSipStack::CheckRegistAuthInfo
(
    SipMsgS&                    stSipMsg,
    SipAuthorization*&          pstSipAuthorization
)
{
    long lResult = SIP::RET_CODE_OK;

    SipAuthorizationListHdl pAuthorizationListHdl = (SipAuthorizationListHdl)SipDsmGetHdrFromMsg(SIP::EX_HDR_ID_AUTHORIZATION, &stSipMsg);

    // ��������Ȩ��Ϣ������Ϊ��Ϣ���Ϸ�
    if (VOS_NULL == pAuthorizationListHdl)
    {
        ERROR_LOG("Fail to get Authenticate Header Info.");
        return SIP::RET_CODE_FAIL;
    }

    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    nResult = VppListGetData(pAuthorizationListHdl, 0, (void**)&pstSipAuthorization);//lint !e838
    if ((VPP_SUCCESS != nResult) || (NULL == pstSipAuthorization))
    {
        ERROR_LOG("Fail to get Authenticate Header Value Info. ErrorCode = %d",nResult);
        return SIP::RET_CODE_FAIL;
    }

    //modify-cwx148380-begin
    //if (VOS_NULL == pstSipAuthorization->pstrAlgorithm)
    //{
    //    ERROR_LOG("Fail to get Authentication Info Failed. Algorithm is Empty.";
    //    return SIP::RET_CODE_FAIL;
    //}
    //modify-cwx148380-end

    // ��ΪDigest���ͣ�����Ҫusername��realm��nonce��uri��response��algorithm
    if (pstSipAuthorization->bIsAuthDigestType)
    {
        if (VOS_NULL == pstSipAuthorization->pstrUsername)
        {
            ERROR_LOG("Fail to get Authentication Info Failed. Digest Username is Empty.");
            return SIP::RET_CODE_FAIL;
        }

        if (VOS_NULL == pstSipAuthorization->pstrRealm)
        {
            ERROR_LOG("Fail to get Authentication Info Failed. Digest Realm is Empty.");
            return SIP::RET_CODE_FAIL;
        }

        if (VOS_NULL == pstSipAuthorization->pstrNonce)
        {
            ERROR_LOG("Fail to get Authentication Info Failed. Digest Nonce is Empty.");
            return SIP::RET_CODE_FAIL;
        }

        if (VOS_NULL == pstSipAuthorization->pstrDigestUri)
        {
            ERROR_LOG("Fail to get Authentication Info Failed. Digest Uri is Empty.");
            return SIP::RET_CODE_FAIL;
        }

        if (VOS_NULL == pstSipAuthorization->pstrDResponse)
        {
            ERROR_LOG("Fail to get Authentication Info Failed. Digest Response is Empty.");
            return SIP::RET_CODE_FAIL;
        }

        //if (VOS_NULL == pstSipAuthorization->pstrAuthScheme)
        //{
        //	ERROR_LOG("Fail to get Authentication Info Failed. Digest Algorithm is Empty.";
        //	return SIP::RET_CODE_FAIL;
        //}
    }

    return lResult;
}

/**
* Description:  HandleReqNotify().  ����Notify������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
* @return       long.       ������
*/
long CSipStack::HandleReqNotify
(
    SS_UINT32                   ulTuObjId,
    SS_UINT32                   ulTxnObjId,
    const SipTptNwInfoS&        stTptNwInf,
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
)
{
    INFO_LOG("Handle Notify Message Begin.");

    long lResult = SIP::RET_CODE_OK;

    char* pMsgBody = VOS_NULL;
    unsigned long ulMsgBodyLen = 0;

    //��ȡ��Ϣ��
    lResult = SIP::GetMsgBody(stSipMsg, pMsgBody, ulMsgBodyLen);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Message Body Failed On Handle NOTIFY Message.");
        return lResult;
    }

    //������Ϣ��
    // added by dhong
    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);
    pHeader->sipMsg = g_recvStr;
    pHeader->pMsgBody = pMsgBody;

    if ( VOS_SUCCESS != GetAddrWithSipTptIpPortS( stTptNwInf.stSrcAddr,
        pHeader->devIP,
        pHeader->devPort
        )
        )
    {
        ERROR_LOG("handle register request - failure to get address info.");
        return VOS_FAIL;
    }

    SIP::PEER_URI_INFO  stFromUri;
    memset(&stFromUri,0x0, sizeof(SIP::PEER_URI_INFO));
    SIP::GetFromUriInfo(stSipMsg, stFromUri);
    pHeader->from = stFromUri.szUriUserName;
    INFO_LOG("Handle NOTIFY Message sipHeader seq = %d,from = %s",pHeader->seq,pHeader->from.c_str());
    //pHeader->digestRealm = stFromUri.szUriHostName;
    pHeader->devDomain = "huawei.com";
    // modified by dhong
    stNotifyReq.pUserData = pHeader;

    stNotifyReq.pszMsgBody = NULL; // pMsgBody;
    stNotifyReq.ulMsgBodyLen = ulMsgBodyLen;

    ///////////////////////////////////  modified by w00207027 2012-11-03  Begin /////////////////////////////////////
    //modified by cwx153028 begin
    if (SIP::RET_CODE_OK == IsRespDirect(pMsgBody))
    {
        //������Ӧ��Ϣ
        CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
        //������Ӧ��Ϣ�����账����ֵ
        (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
    }
    else
    {
        unsigned long userSeq = InsertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, stNotifyReq.ulDialogId);
        pHeader->userSeq = userSeq;
        //SIP::GetContact(stSipMsg, pHeader->devIP, pHeader->devPort);
    }
    //modified by cwx153028 end
    ///////////////////////////////////  modified by w00207027 2012-11-03  End /////////////////////////////////////

    ////������Ӧ��Ϣ
    //CSipRspMsg objSipRspMsg(*this, stSipMsg);
    ////������Ӧ��Ϣ�����账����ֵ
    //(void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);

    //�ϱ��ı���Ӧ��Ϣ
    INFO_LOG("============Notify sipHeader seq = %d, from = %s",pHeader->seq,pHeader->from.c_str());
    lResult = NotifyTextMsgReq(stNotifyReq);

    //�ͷ���Ϣ���ڴ�
    // dhong I think this should be invoked
    // VOS_DELETE(pMsgBody, MULTI);

    INFO_LOG("Handle Notify Message End.");
    return lResult;
}

//////////////////////////////////////// add by w00207027 2012-10-27 End //////////////////////////////////////
/**
* Description:  HandleReqInvite().  ����Invite������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
* @return       long.       ������
*/
long CSipStack::HandleReqInvite
(
    SS_UINT32                   ulTuObjId,
    SS_UINT32                   ulTxnObjId,
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
)
{
    INFO_LOG("Handle Invite Message Begin.");

    long lResult = SIP::RET_CODE_OK;

    lResult = GetDialogIdBySipMsg(stSipMsg, stNotifyReq.ulDialogId);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Dialog ID Failed On Handle Invite Message.");
        return lResult;
    }

    char* pMsgBody = VOS_NULL;
    unsigned long ulMsgBodyLen = 0;

    //��ȡ��Ϣ��
    lResult = SIP::GetMsgBody(stSipMsg, pMsgBody, ulMsgBodyLen);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Message Body Failed On Handle Invite Message.");
        return lResult;
    }

    //������Ϣ��
    stNotifyReq.pszMsgBody       = pMsgBody;
    stNotifyReq.ulMsgBodyLen     = ulMsgBodyLen;

    char* pSubject = VOS_NULL;
    unsigned long ulSubjectLen = 0;

    //��ȡSubject����
    lResult = SIP::GetSubject(stSipMsg, pSubject, ulSubjectLen);
    if (SIP::RET_CODE_OK != lResult)
    {
        //�ͷ���Ϣ���ڴ�
        VOS_DELETE(pMsgBody, MULTI);

        ERROR_LOG("Get Subject Value Failed On Handle Invite Message.");
        return lResult;
    }

    //     //����������Ϣ��Ϣ,���ڴ��ڷ�����Ӧ��Ϣ���ͷ�
    //     SIP::REQ_MSG_INFO* pReqMsgInfo = VOS_NEW(pReqMsgInfo);
    //     if (NULL != pReqMsgInfo)
    //     {
    //         pReqMsgInfo->pstSipReqMsg   = &stSipMsg;
    //         pReqMsgInfo->ulTuObjId      = ulTuObjId;
    //         pReqMsgInfo->ulTxnObjId     = ulTxnObjId;
    //         pReqMsgInfo->ulDialogId     = stNotifyReq.ulDialogId;
    //         pReqMsgInfo->secs = time(NULL);
    //     }   
    // 
    //     {
    //         CLockGuard objGard(m_pReqMsgMutex);
    //         unsigned long mapKey = (unsigned long)stSipMsg.stHeaders.pstCseq->ulSequence;
    //         m_msgMap[mapKey] = pReqMsgInfo;
    //     }
    unsigned long userSeq = InsertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, stNotifyReq.ulDialogId);

    stNotifyReq.pMsgHeader       = pSubject;
    stNotifyReq.ulMsgHeaderLen   = ulSubjectLen;

    // added by dhong
    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);
    pHeader->sipMsg = g_recvStr;
    pHeader->userSeq = userSeq;
    pHeader->pMsgBody = pMsgBody;
    if (NULL != pSubject)
    {
        pHeader->subJect = pSubject;
    }
    // modified by dhong
    stNotifyReq.pUserData = pHeader;

    //�ϱ��ı���Ӧ��Ϣ
    lResult = NotifyTextMsgReq(stNotifyReq);

    //�ͷ���Ϣ���ڴ�
    // VOS_DELETE(pMsgBody, MULTI);
    //�ͷ�Subject�ڴ�
    VOS_DELETE(pSubject, MULTI);

    INFO_LOG("Handle Invite Message Success.");
    return SIP::RET_CODE_OK;//lint !e438
}

/**
* Description:  HandleReqMessage().     ����Message������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
* @return       long.       ������
*/
long CSipStack::HandleReqMessage
(
    SS_UINT32                   ulTuObjId,
    SS_UINT32                   ulTxnObjId,
    const SipTptNwInfoS&        stTptNwInf,
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
)
{
    INFO_LOG("Handle MESSAGE Message Begin.");

    long lResult = SIP::RET_CODE_OK;

    char* pMsgBody = VOS_NULL;
    unsigned long ulMsgBodyLen = 0;

    //��ȡ��Ϣ��
    lResult = SIP::GetMsgBody(stSipMsg, pMsgBody, ulMsgBodyLen);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Message Body Failed On Handle MESSAGE Message.");
        return lResult;
    }

    //������Ϣ��
    // added by dhong
    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);
    pHeader->sipMsg = g_recvStr;
    pHeader->pMsgBody = pMsgBody;

    if ( VOS_SUCCESS != GetAddrWithSipTptIpPortS( stTptNwInf.stSrcAddr,
        pHeader->devIP,
        pHeader->devPort
        )
        )
    {
        ERROR_LOG("handle register request - failure to get address info.");
		VOS_DELETE(pMsgBody,MULTI);
        return VOS_FAIL;
    }

    SIP::PEER_URI_INFO  stFromUri;
    memset(&stFromUri,0x0, sizeof(SIP::PEER_URI_INFO));
    SIP::GetFromUriInfo(stSipMsg, stFromUri);
    pHeader->from = stFromUri.szUriUserName;
    INFO_LOG("Handle MESSAGE Message sipHeader seq = %d,from = %s",pHeader->seq,pHeader->from.c_str());
    //pHeader->digestRealm = stFromUri.szUriHostName;
    pHeader->devDomain = "huawei.com";
    // modified by dhong
    stNotifyReq.pUserData = pHeader;

    stNotifyReq.pszMsgBody = NULL; // pMsgBody;
    stNotifyReq.ulMsgBodyLen = ulMsgBodyLen;

    ///////////////////////////////////  modified by w00207027 2012-11-03  Begin /////////////////////////////////////
    //modified by cwx153028 begin
    int nRet = IsRespDirect(pMsgBody);
    if (0 == nRet)
    {
        //������Ӧ��Ϣ
        CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
        //������Ӧ��Ϣ�����账����ֵ
        (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
        INFO_LOG("Handle MESSAGE Message sipHeader seq = %d, from = %s,sipStack send 200 OK to pu.",pHeader->seq,pHeader->from.c_str());
    }
	else if(1 == nRet)
	{
		//������Ӧ��Ϣ
		CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
		(void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
		delete pHeader;
		pHeader = NULL;
		INFO_LOG("delete pHeader = [0X%x]",pHeader); 
		stNotifyReq.pUserData = NULL;
		VOS_DELETE(pMsgBody, MULTI);
		INFO_LOG("Keep Alive Message");
		return SIP::RET_CODE_OK;//lint !e438
	}
    else if (2 == nRet)
    {
        //������Ӧ��Ϣ
        CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
        //������Ӧ��Ϣ�����账����ֵ
        (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
    }
    else
    {
        unsigned long userSeq = InsertReqMsg(stSipMsg, ulTuObjId, ulTxnObjId, stNotifyReq.ulDialogId);
        pHeader->userSeq = userSeq;
        //SIP::GetContact(stSipMsg, pHeader->devIP, pHeader->devPort);
     }
    //modified by cwx153028 end
    ///////////////////////////////////  modified by w00207027 2012-11-03  End /////////////////////////////////////

    ////������Ӧ��Ϣ
    //CSipRspMsg objSipRspMsg(*this, stSipMsg);
    ////������Ӧ��Ϣ�����账����ֵ
    //(void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);

    //�ϱ��ı���Ӧ��Ϣ
    INFO_LOG("============Message sipHeader seq = %d, from = %s",pHeader->seq,pHeader->from.c_str());
    lResult = NotifyTextMsgReq(stNotifyReq);

    //�ͷ���Ϣ���ڴ�
    // dhong I think this should be invoked
    //VOS_DELETE(pMsgBody, MULTI);

    INFO_LOG("Handle MESSAGE Message Success.");
    return SIP::RET_CODE_OK;//lint !e438
}

//////////////////////////////////////////// add by w00207027 2012-11-03 Begin /////////////////////////////////////
/**
* Description:  isRespDirect().     �ж��Ƿ�ֱ�ӻ���Ӧ��Ϣ
* @param  [in]      pMsgBody            SIP��Ϣ��
* @return   bool.       ������
*/
long CSipStack::IsRespDirect
(
    const char* pMsgBody
)
{
    TiXmlDocument doc;
    doc.Parse(pMsgBody,0,TIXML_ENCODING_LEGACY);

    if (doc.Error())
    {
        return 1; 
    }

    TiXmlElement *root = doc.RootElement();
    if (NULL == root)
    {
        return 1; 
    }

    TiXmlElement *pList = root->FirstChildElement(T28181_XML_CMD_TYPE);
    if (NULL == pList)
    {
        return 0;
    }

    const char *pCmd = pList->GetText();
    if ((NULL != pCmd)
        &&((0==strncmp(T28181_XML_CMD_TYPE_KEEP_ALIVE, pCmd,  T28181_XML_CMD_TYPE_LENGTH))))
    {
        return 1;
    }

    if ((NULL != pCmd)
        &&((0==strncmp( T28181_XML_CMD_TYPE_ALARM, pCmd,  T28181_XML_CMD_TYPE_LENGTH))))
    {
        return 2;
    }

    return 0;
}
//////////////////////////////////////////// add by w00207027 2012-11-03 End /////////////////////////////////////

/**
* Description:  HandleReqBye().     ����Bye������Ϣ
* @param  [in]  ulTuObjId   ��Ӧ��TU����ID
* @param  [in]  ulTxnObjId  ��Ӧ���������ID
* @param  [in]      stSipMsg            SIP��Ϣ����
* @param  [out] stNotifyReq     ֪ͨ�ı�����ṹ������
* @return       long.       ������
*/
long CSipStack::HandleReqBye
(
    SS_UINT32                   ulTuObjId,
    SS_UINT32                   ulTxnObjId,
    SipMsgS&                    stSipMsg,
    SIP::NOTIFY_TEXT_MSG_REQ&   stNotifyReq
)
{
    INFO_LOG("Handle BYE Message Begin.");

    long lResult = SIP::RET_CODE_OK;

    //��ȡ�Ի�ID�����ҵ������MAP��ɾ��
    lResult = GetDialogIdBySipMsg(stSipMsg, stNotifyReq.ulDialogId, VOS_TRUE);//lint !e838
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Get Dialog ID Failed On Handle BYE Message.");
        //   return lResult;
    }

    SIP_HEADER *pHeader = GetSipHeader(stSipMsg, true);
    // modified by dhong
    pHeader->sipMsg = g_recvStr;
    // pHeader->pData = NULL;
    stNotifyReq.pUserData = pHeader;

    //�ϱ��ı���Ӧ��Ϣ
    lResult = NotifyTextMsgReq(stNotifyReq);

    //������Ӧ��Ϣ
    CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
    //������Ӧ��Ϣ�����账����ֵ
    (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, ulTuObjId, ulTxnObjId);
    INFO_LOG("Handle BYE Message End. Result = 0x%04X.", lResult);
    return SIP::RET_CODE_OK;
}
/*
SS_UINT32                   ulTuObjId,
SS_UINT32                   ulTxnObjId,
*/
unsigned long CSipStack::InsertReqMsg(SipMsgS &sipMsg, const SS_UINT32 &ulTuObjId, const SS_UINT32 &ulTxnObjId, const unsigned long &ulDialogId)
{
    INFO_LOG("CSipStack::insertReqMsg entered");
    //����������Ϣ��Ϣ,���ڴ��ڷ�����Ӧ��Ϣ���ͷ�
    SIP::REQ_MSG_INFO* pReqMsgInfo = VOS_NEW(pReqMsgInfo);
    if (NULL == pReqMsgInfo)
    {
        INFO_LOG("CSipStack::insertReqMsg insert failed: lack memory");
        return 0;
    }

	if(SIP_RET_SUCCESS != SipDsmCopyMsgRef (&sipMsg, &(pReqMsgInfo->pstSipReqMsg)))   // ���ô���+1
        //if(SIP::RET_CODE_OK != SipDsmCloneMsg (&sipMsg, &(pReqMsgInfo->pstSipReqMsg)))  // ��ȫ����һ��
    {
        INFO_LOG("CSipStack::insertReqMsg insert failed: lack memory");
        VOS_DELETE(pReqMsgInfo);
        return 0;
    }

    pReqMsgInfo->ulTuObjId      = ulTuObjId;
    pReqMsgInfo->ulTxnObjId     = ulTxnObjId;
    pReqMsgInfo->ulDialogId     = ulDialogId;
    pReqMsgInfo->secs = (unsigned long)time(NULL); 
    pReqMsgInfo->state = SIP::INIT;

    if (0 == m_mapKey)
    {
        ++m_mapKey;
    }

    {
        //  cout << "CSipStack::insertReqMsg insert: " << m_mapKey << endl;
        CLockGuard objGard(m_pReqMsgMutex);
        m_msgMap[m_mapKey++] = pReqMsgInfo;
    }//lint !e1788

    INFO_LOG("CSipStack::insertReqMsg insert success");

    return (m_mapKey-1);
}

void CSipStack::CheckReqMsg(void)
{
    unsigned long curSec = (unsigned long)time(NULL);

    CLockGuard objGard(m_pReqMsgMutex);

    for(m_msgItertor = m_msgMap.begin(); m_msgItertor != m_msgMap.end();)
    {
        if ((curSec - 40) > m_msgItertor->second->secs )    
        {
            //    cout << SipStack::genCurTime() << "   " << "CSipStack::checkReqMsg timeout:" << endl;
            SIP::REQ_MSG_INFO *pInfo = m_msgItertor->second;
            if((NULL!=pInfo)
                && (SIP::OCCUPIED!=pInfo->state))
            {
                m_msgMap.erase(m_msgItertor++);

                SipMsgS *stSipMsg = pInfo->pstSipReqMsg;

                if (NULL != stSipMsg)
                {
                    //ǿ����ֹ����
                    /*(void)SipTxnHiTerminateTxn(stSipMsg->, stMsgInfo.ulTuObjId, SIP_TERM_MODE_FORCEFUL_NO_CANCEL_RSP);*/
                    //�ͷ�SIP��Ϣ
                    SipDsmReleaseMsgRef(&stSipMsg);
                    VOS_DELETE(pInfo);
                }

                continue;
            }
        }

        ++m_msgItertor;
    }
}//lint !e1788

SIP::REQ_MSG_INFO* CSipStack::GetReqMsg(unsigned long seq, bool bGetAndDelete)
{
    CLockGuard objGard(m_pReqMsgMutex);

    m_msgItertor = m_msgMap.find(seq);
    if (m_msgItertor == m_msgMap.end())
    {
        return NULL;
    }

    SIP::REQ_MSG_INFO *pInfo = m_msgItertor->second;
    if (bGetAndDelete)
    {
        m_msgMap.erase(m_msgItertor);
    }
    else
    {
        pInfo->state = SIP::OCCUPIED;
    }

    return pInfo;
}//lint !e1788

long CSipStack::SendInviteResp(const string &msg, const unsigned long seq, const unsigned long retCode)
{
    long lResult = SIP::RET_CODE_FAIL;
    // cout<<SipStack::genCurTime()<<" | " << "CSipStack::sendInviteResp Entered:" << seq << endl;
    SIP::REQ_MSG_INFO* pReqMsgInfo = GetReqMsg(seq);
    if (NULL == pReqMsgInfo)
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::sendInviteResp get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    SIP::TEXT_MSG _msg;
    memset(&_msg, 0, sizeof(_msg));
    _msg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_INVITE_RSP;
    _msg.ulMsgSeq=2;
    _msg.ulMsgBodyLen = msg.length();
    _msg.pszMsgBody = (char *)msg.c_str();//lint !e1773
    _msg.pUserData = pReqMsgInfo;

    CSipStackMsgWrap* pSipMsg = new CSipStackMsgWrapInviteRespEx(*this, _msg, retCode);
    lResult = pSipMsg->SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Send Message Failed On Handle SIP Register Response Message.");
    }
    else
    {
        INFO_LOG("CSipStack::sendInviteResp Success");
    }

	delete pSipMsg;
	pSipMsg = NULL;

    return lResult;
}

long CSipStack::SendRegisterResp(const unsigned long seq, const unsigned long retCode)
{
    long lResult = SIP::RET_CODE_FAIL;
    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq);
    if (NULL == pReqMsgInfo)
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::SendRegisterResp get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    CSipStackMsgWrapRegisterResp* pSipMsg = new CSipStackMsgWrapRegisterResp(*this, *(pReqMsgInfo->pstSipReqMsg), false);
    lResult = pSipMsg->SendRspMsg(retCode, pReqMsgInfo->ulTuObjId, pReqMsgInfo->ulTxnObjId);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG(" Send Message Failed On Handle SIP Register Response Message.");
    }
    else
    {
        INFO_LOG("CSipStack::SendRegisterResp Success");
    }

	delete pSipMsg;
	pSipMsg = NULL;
    SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
	delete pReqMsgInfo;
	pReqMsgInfo = NULL;

    return lResult;
}

long CSipStack::SendKeepaliveResp(const unsigned long seq, const unsigned long retCode)
{
    long lResult = SIP::RET_CODE_FAIL;
    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq);
    if (NULL == pReqMsgInfo)
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::SendKeepaliveResp get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    CSipStackMsgWrapResp* pSipMsg = new CSipStackMsgWrapResp(*this, *(pReqMsgInfo->pstSipReqMsg));
    lResult = pSipMsg->SendRspMsg(retCode, pReqMsgInfo->ulTuObjId, pReqMsgInfo->ulTxnObjId);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Send Message Failed On Handle SIP Keepalive Response Message.");
    }
    else
    {
        INFO_LOG("CSipStack::SendKeepaliveResp Success");
    }

	delete pSipMsg;
	pSipMsg = NULL;
    SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
	delete pReqMsgInfo;
	pReqMsgInfo = NULL;

    return lResult;
}

long CSipStack::SendOptionResp(const unsigned long seq, const unsigned long retCode)
{
    long lResult = SIP::RET_CODE_FAIL;
    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq);
    if (NULL == pReqMsgInfo)
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::SendOptionResp get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    CSipStackMsgWrapOptionsResp* pSipMsg = new CSipStackMsgWrapOptionsResp(*this, *(pReqMsgInfo->pstSipReqMsg));
    lResult = pSipMsg->SendRspMsg(retCode, pReqMsgInfo->ulTuObjId, pReqMsgInfo->ulTxnObjId);
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("Send Message Failed On Handle SIP Option Response Message.");
    }
    else
    {
        INFO_LOG("CSipStack::SendOptionResp Success");
    }

	delete pSipMsg;
	pSipMsg = NULL;
    SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
	delete pReqMsgInfo;
	pReqMsgInfo = NULL;

    return lResult;
}

long CSipStack::SendInviteAck(const unsigned long seq)
{
    long lResult = SIP::RET_CODE_FAIL;
    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq, false);
    if ((NULL == pReqMsgInfo)
        || (NULL ==pReqMsgInfo->pstSipReqMsg)
        || (NULL ==pReqMsgInfo->pstSipReqMsg->stHeaders.pstCseq))
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::SendInviteAck get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    MAP_DIALOG_ID_TO_INFO::iterator iter_id_inf = m_mapDialogIdToInfo.find(pReqMsgInfo->ulDialogId);
    if ( m_mapDialogIdToInfo.end() == iter_id_inf )
    {
        return lResult;
    }


    //����ACK��Ϣ����
    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq     = pReqMsgInfo->pstSipReqMsg->stHeaders.pstCseq->ulSequence;
    stTextMsg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_ACK;
    stTextMsg.ulDialogId    = pReqMsgInfo->ulDialogId;
    stTextMsg.pUserData     = pReqMsgInfo->pstSipReqMsg;

    CSipStackMsgWrapAck objSipMsgAck(*this, stTextMsg);

    //����ACK
    lResult = objSipMsgAck.SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Send ACK Message Failed On Handle SIP Invite Response Message.",stTextMsg.ulMsgSeq);
    }
    else
    {
        INFO_LOG("CSipStack::SendInviteAck Success, msgType=%d",stTextMsg.enTextMsgType);
    }

    return lResult;
}

long CSipStack::SendInviteBye(const unsigned long seq)
{
    long lResult = SIP::RET_CODE_FAIL;
    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq);
    if ((NULL == pReqMsgInfo)
        || (NULL ==pReqMsgInfo->pstSipReqMsg)
        || (NULL ==pReqMsgInfo->pstSipReqMsg->stHeaders.pstCseq))
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::SendInviteBye get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    //����BYE��Ϣ����
    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq     = (unsigned long)(pReqMsgInfo->pstSipReqMsg->stHeaders.pstCseq->ulSequence + 1);
    stTextMsg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_BYE;
    stTextMsg.ulDialogId    = pReqMsgInfo->ulDialogId;
    stTextMsg.pUserData     = pReqMsgInfo->pstSipReqMsg;

    CSipStackMsgWrapBye objSipMsgBye(*this, stTextMsg);

    //����BYE
    lResult = objSipMsgBye.SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Send BYE Message Failed On Handle SIP Invite Response Message.",stTextMsg.ulMsgSeq);
    }
    else
    {
        INFO_LOG("CSipStack::SendInviteBye Success");
    }

    SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
    VOS_DELETE(pReqMsgInfo);

    return lResult;
}

long CSipStack::SendInviteCancel(const unsigned long seq)
{
    long lResult = SIP::RET_CODE_FAIL;
    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq);
    if ((NULL == pReqMsgInfo)
        || (NULL ==pReqMsgInfo->pstSipReqMsg)
        || (NULL ==pReqMsgInfo->pstSipReqMsg->stHeaders.pstCseq))
    {
        // cout<<SipStack::genCurTime()<<" | " << "CSipStack::SendInviteCancel get ReqMsg failed:" << seq << endl;
        return lResult;
    }

    //����Cancel��Ϣ����
    SIP::TEXT_MSG stTextMsg = {0};
    stTextMsg.ulMsgSeq     = pReqMsgInfo->pstSipReqMsg->stHeaders.pstCseq->ulSequence;
    stTextMsg.enTextMsgType = SIP::TEXT_MSG_TYPE_SIP_CANCEL;
    stTextMsg.ulDialogId    = pReqMsgInfo->ulDialogId;
    stTextMsg.pUserData     = pReqMsgInfo->pstSipReqMsg;

    CSipStackMsgWrapAck objSipMsgAck(*this, stTextMsg);   //  TODO  Cancel ��Ϣ����װ

    //����Cancel
    lResult = objSipMsgAck.SendMsg();
    if (SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("[ MsgSeq = %d ] Send Cancel Message Failed On Handle SIP Invite Response Message.",stTextMsg.ulMsgSeq);
    }
    else
    {
        INFO_LOG("CSipStack::SendInviteCancel Success");
    }

    SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
    VOS_DELETE(pReqMsgInfo);

    return lResult;
}

long CSipStack::SendResp(const unsigned long seq)
{
    //  cout<<SipStack::genCurTime()<<" | " << "CSipStack::sendResp Entered:" << seq << endl;

    SIP::REQ_MSG_INFO* pReqMsgInfo=GetReqMsg(seq);
    if (NULL == pReqMsgInfo)
    {
        //    cout<<SipStack::genCurTime()<<" | " << "CSipStack::sendResp get REQ_MSG_INFO failed:" << seq << endl;
        return -1;
    }

    SipMsgS &stSipMsg = *(pReqMsgInfo->pstSipReqMsg);

    CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg);
    (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, pReqMsgInfo->ulTuObjId, pReqMsgInfo->ulTxnObjId);

    // ��ʱ���ӣ�������� dhong
    SipDsmReleaseMsgRef(&(pReqMsgInfo->pstSipReqMsg));
    VOS_DELETE(pReqMsgInfo);

    INFO_LOG("CSipStack::sendResp Success");

    return 0;
}

long CSipStack::AnsMsg( const string& ans_code, const string& data)
{
    unsigned long i_ans_code = (unsigned long)ACE_OS::atoi(ans_code.c_str());
    SIP::REQ_MSG_INFO* req_msg_info_ptr = GetReqMsg(i_ans_code);
    if ( NULL == req_msg_info_ptr )
    {
        return -1;
    }

    SipMsgS &stSipMsg = *(req_msg_info_ptr->pstSipReqMsg);

    CSipStackMsgWrapResp objSipRspMsg(*this, stSipMsg, data);
    (void)objSipRspMsg.SendRspMsg(SIP_STATUS_SUCC_OK, req_msg_info_ptr->ulTuObjId, req_msg_info_ptr->ulTxnObjId);

    // ��ʱ���ӣ�������� dhong
    SipDsmReleaseMsgRef(&(req_msg_info_ptr->pstSipReqMsg));
    VOS_DELETE(req_msg_info_ptr);

    INFO_LOG("CSipStack::sendResp Success");

    return 0;
}

/**
* Description:  NotifyCallBack(). ���ûص�����
* @param  [in]  stInfo  ֪ͨ��Ϣ�ṹ��
* @return       long.   ������
*/
long CSipStack::NotifyCallBack(SIP::NOTIFY_INFO& stInfo)
{
    //ָ��ǿռ��
    if (NULL == m_pfnNotify)
    {
        return SIP::RET_CODE_MEMORY_NULL;
    }

    //���ûص�����
    long lResult = m_pfnNotify(&stInfo, m_pNotifyFnParams);

    return lResult;
}

/**
* Description:  NotifyTextMsgResp(). ֪ͨ�ı���Ӧ��Ϣ
* @param  [in]  stNotifyInfo    �ı���Ӧ��Ϣ֪ͨ��Ϣ�ṹ��
* @return       long.   ������
*/
long CSipStack::NotifyTextMsgResp(SIP::NOTIFY_TEXT_MSG_RESP& stNotifyInfo)
{
    SIP::NOTIFY_INFO stInfo = {0};

    stInfo.ulNotifyType     = SIP::NOTIFY_TYPE_TEXT_MSG_RESP;
    stInfo.pNotifyInfo      = &stNotifyInfo;
    stInfo.ulNotifyInfoLen  = sizeof(stNotifyInfo);

    long lResult = NotifyCallBack(stInfo);

    return lResult;
}

/**
* Description:  NotifyTextMsgReq(). ֪ͨ�ı�������Ϣ, �豸����ƽ̨��������Ϣ
* @param  [in]  stNotifyInfo        �ı�������Ϣ֪ͨ��Ϣ�ṹ��
* @return       long.   ������
*/
long CSipStack::NotifyTextMsgReq(SIP::NOTIFY_TEXT_MSG_REQ& stNotifyInfo)
{
    SIP::NOTIFY_INFO stInfo = {0};

    stInfo.ulNotifyType     = SIP::NOTIFY_TYPE_TEXT_MSG_REQ;
    stInfo.pNotifyInfo      = &stNotifyInfo;
    stInfo.ulNotifyInfoLen  = sizeof(stNotifyInfo);

    long lResult = NotifyCallBack(stInfo);

    return lResult;
}

/**
* Description:  GetSipMsgInfoByCSeq().      ����CSeq�ҵ�SIP��Ϣ����Ϣ
* @param  [in]      ulCSeq              SIP��Ϣ��CSeq
* @param  [out]     pstSipMsgInfo    SIP��Ϣ��Ϣָ��
* @param  [in]  bErase      �Ƿ�ɾ��
* @return       long.       ������
*/
long CSipStack::GetSipMsgInfoByCSeq
(
    unsigned long   ulCSeq,
    SIP_MSG_INFO*&  pstSipMsgInfo,
    VOS_BOOLEAN     bErase
)
{
    //������
    CLockGuard objLocker(m_pMapMutex);

    //�������ݰ�
    CSipStack::MAP_C_SEQ_TO_SIP_MSG_INFO::iterator iter = m_mapCSeqToSipMsgInfo.find(ulCSeq);
    //û���ҵ�����
    if (m_mapCSeqToSipMsgInfo.end() == iter)
    {  
        WARN_LOG("[ CSeq = %d ] SIP Message Info Not Found.",ulCSeq);
        return SIP::RET_CODE_FAIL;
    }

    pstSipMsgInfo = iter->second;

    if (bErase)
    {
        //��map����ɾ������
        (void)m_mapCSeqToSipMsgInfo.erase(iter);
    }

    return SIP::RET_CODE_OK;
}//lint !e1788

//BEGIN R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�
/**
* Description:  GetThreadMutex().   ��ȡ�߳���
* @return       VOS_Mutex*.     ��ָ��
*/
VOS_Mutex* CSipStack::GetThreadMutex()
{
    return m_pThreadMutex;
}
//END   R002C01SPC001 ADD 2011-08-24 ligengqiang 00124479 for �����߳�����MiniSIPֻ֧�ֵ��߳�

int CSipStack::GetAddrWithSipTptIpPortS( IN  const SipTptIpPortS& stTptAddr,
                                         OUT std::string&         strIPv4,
                                         OUT unsigned short&      usPort
                                       )
{
    char buf[24] = {0};
    int pos = snprintf( buf, sizeof(buf), "%u.%u.%u.%u",
                        stTptAddr.u.aucIPv4Addr[0],
                        stTptAddr.u.aucIPv4Addr[1],
                        stTptAddr.u.aucIPv4Addr[2],
                        stTptAddr.u.aucIPv4Addr[3]
                      );
    if ( 0 >= pos )
    {
        ERROR_LOG("failure to get address - snprintf");
        return VOS_FAIL;
    }
    buf[pos] = '\0';
    strIPv4 = buf;
    usPort  = (unsigned short)stTptAddr.iPort;

    return VOS_SUCCESS;
}

int CSipStack::HandleReqResend( const unsigned long enHeader, const SipTptNwInfoS& stTptNwInf, const unsigned long nCSeq )
{
    if ( m_mapRspResend.empty() )
    {
        ERROR_LOG("handle request resend - resend map is empty.");
        return VOS_FAIL;
    }

    std::string    strIPv4 = "";
    unsigned short usPort  = 0;
    if (  VOS_SUCCESS != GetAddrWithSipTptIpPortS(stTptNwInf.stDstAddr, strIPv4, usPort)
       || strIPv4.empty()	||	0 == usPort
       )
    {
        ERROR_LOG("handle request resend - resend address is invalid.");
        return VOS_FAIL;
    }

    DEAL_RESEND_HEADER_MAP::iterator hit = m_mapReqResend.find((int)enHeader);
    if ( hit == m_mapReqResend.end() )
    {
        ERROR_LOG("handle request resend - unsupport header - Header_Type=%d, CSeq=%d.",enHeader,nCSeq);
        return VOS_FAIL;
    }

    CSipStack::CDealResendData aDealRespndData(strIPv4, usPort, nCSeq);
	DEAL_RESEND_LIST& dList = (DEAL_RESEND_LIST)hit->second;
    if ( !dList.empty() )
    {
        for ( DEAL_RESEND_LIST::iterator iter = dList.begin();
            iter != dList.end();

            )
        {
            if ( NULL == *iter )
            {
                dList.erase(iter++);
                continue;
            }

            if ( !(*iter)->IsTimeout() )
            {
                if ( (*iter)->IsTheSame( aDealRespndData ) )
                {
					WARN_LOG("handle request resend - drop resend- Header_Type=%d, IPv4=%s, Port=%d, CSeq=%d.",
						enHeader, (*iter)->GetIPv4().c_str(), (*iter)->GetPort(), nCSeq);
                    return VOS_FAIL;
                }
                else
                {
                    iter ++;
                }
            }
            else
            {
				DEBUG_LOG( "handle requst resend - earse - Header_Type=%d, IPv4=%s, Port=%d, CSeq=%d.",
					enHeader, (*iter)->GetIPv4().c_str(), (*iter)->GetPort(), nCSeq);
                
                delete (*iter);
                (*iter) = NULL;
                dList.erase(iter++);
                continue;
            }
        }
    }

	DEBUG_LOG( "handle request resend - push - Header_Type=%d, IPv4=%s, Port=%d, CSeq=%d.",
		enHeader, aDealRespndData.GetIPv4().c_str(), aDealRespndData.GetPort(), nCSeq);

	CSipStack::CDealResendData* pData = new CSipStack::CDealResendData(strIPv4, usPort, nCSeq);
    dList.push_back(pData);
    return VOS_SUCCESS;
}

int CSipStack::HandleRspResend( const unsigned long enHeader, const SipTptNwInfoS& stTptNwInf, const unsigned long nCSeq )
{
    if ( m_mapRspResend.empty() )
    {
        ERROR_LOG("handle respond resend - resend map is empty.");
        return VOS_FAIL;
    }

    std::string    strIPv4 = "";
    unsigned short usPort  = 0;
    if (  VOS_SUCCESS != GetAddrWithSipTptIpPortS(stTptNwInf.stDstAddr, strIPv4, usPort)
       || strIPv4.empty()
       || 0 == usPort
       )
    {
        ERROR_LOG("handle respond resend - resend address is invalid.");
        return VOS_FAIL;
    }

    DEAL_RESEND_HEADER_MAP::iterator hit = m_mapRspResend.find((int)enHeader);
    if ( hit == m_mapRspResend.end() )
    {
        ERROR_LOG("handle respond resend - unsupport header - Header_Type=%d, CSeq=%d",enHeader,nCSeq);
        return VOS_FAIL;
    }

    CSipStack::CDealResendData aDealRespndData(strIPv4, usPort, nCSeq);
	DEAL_RESEND_LIST & dList = (DEAL_RESEND_LIST)hit->second;
    if ( !dList.empty() )
    {
        for ( DEAL_RESEND_LIST::iterator iter = dList.begin();
            iter != dList.end();

            )
        {
            if ( NULL == *iter )
            {
                dList.erase(iter++);
                continue;
            }

            if ( !(*iter)->IsTimeout() )
            {
                if ( (*iter)->IsTheSame(aDealRespndData) )
                {
					WARN_LOG("handle respond resend - drop resend- Header_Type=%d, IPv4=%s, Port=%d, CSeq=%d.",
						enHeader, (*iter)->GetIPv4().c_str(), (*iter)->GetPort(), nCSeq);
                    return VOS_FAIL;
                }
                else
                {
                    iter ++;
                }
            }
            else
            {
                delete (*iter);
                (*iter) = NULL;
                dList.erase(iter++);
                continue;
            }
        }
    }

    CSipStack::CDealResendData* pData = new CSipStack::CDealResendData(strIPv4, usPort, nCSeq);
    dList.push_back(pData);
    return VOS_SUCCESS;
}

CSipStack::CDealResendData::CDealResendData ( const std::string& strIPv4, unsigned short usPort,  unsigned long nCSeq )
                           :m_strIPv4(strIPv4.c_str())
                           ,m_usPort(usPort)
                           ,m_nCSeq(nCSeq)
                           ,m_tResendTime(ACE_OS::time())
{
}

const std::string& CSipStack::CDealResendData::GetIPv4() const
{
    return m_strIPv4;
}

unsigned short CSipStack::CDealResendData::GetPort() const
{
    return m_usPort;
}

unsigned long CSipStack::CDealResendData::GetCSeq() const
{
    return m_nCSeq;
}

bool CSipStack::CDealResendData::IsTheSame ( const CDealResendData& other )
{
    return ( m_strIPv4 == other.m_strIPv4 && m_usPort == other.m_usPort && m_nCSeq == other.m_nCSeq );
}

bool CSipStack::CDealResendData::IsTimeout ()
{
    return (RESEND_INTERVAL < (ACE_OS::time() - m_tResendTime));
}

}//end namespace

//lint +e429

#pragma warning(pop)

