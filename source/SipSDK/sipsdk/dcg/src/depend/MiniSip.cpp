/**
* @file  MiniSip.cpp
* @brief MiniSip��غ�����ʵ��
*
* Copyright (c) 2010
* Huawei Tech.Co.,Ltd
*
* @author   Li GengQiang/l00124479
* @date     2010/12/06
* @version  NVS V100R002C01
*
*/

#include "MiniSip.h"
#include "randutil.h"
#include "digcalc.h"

#include "CSipStack.hpp"
#include <sstream>
#include <cstdio>
#include <iostream>
#include <ace/OS_NS_time.h>
#include <ace_header.h>
#include "sip_namespace.h"
#include "Log.h"

//lint -e438
//lint -e818

#ifndef FALSE
#define FALSE false
#endif
//namespace IvsCbb{ namespace SipStack{  
//SIP��UDP��Socket
unsigned long g_ulSipUdpSocket = 0;


EN_SIP_RESULT SipStartTimer
(
    IN  SS_UINT32      ulTimerLength,
    IN  SS_UINT32      ulTimerParam,
    OUT SS_UINT32     *pulTimerId
)
{
    if (VOS_NULL == SipStack::g_pSipStack)
    {
        return SIP_RET_FAILURE;
    }

    //�����¼���Ϣ�������¼��߳����ͷŸ��ڴ�
    SipStack::SIP::EVENT_INFO_TIMER_REGISTER* pstTimerInfo = VOS_NEW(pstTimerInfo);
    if (VOS_NULL == pstTimerInfo)
    {
        return SIP_RET_FAILURE;
    }

    pstTimerInfo->ulTimerId         = SipStack::g_pSipStack->GetTimerID();    //��ʱ��ID
    pstTimerInfo->ulTimerLength     = ulTimerLength;                //��ʱ��ʱ��
    pstTimerInfo->ulTimerPara       = ulTimerParam;                 //��ʱ������
    pstTimerInfo->ulStartTimeTick   = SipStack::VOS_GetTicks();               //��ʼʱ���
    //BEGIN R002C01SPC100 ADD 2011-08-15 ligengqiang 00124479 for ��ʱ��SipTxnTimeoutHandler��������⣬��ֹ�ظ�ֹͣ��ʱ��
    pstTimerInfo->bStop             = false;                        //�Ƿ���ֹͣ
    //END   R002C01SPC100 ADD 2011-08-15 ligengqiang 00124479 for ��ʱ��SipTxnTimeoutHandler��������⣬��ֹ�ظ�ֹͣ��ʱ��

    //���ö�ʱ��ID
    *pulTimerId = pstTimerInfo->ulTimerId;

    SipStack::SIP::EVENT_INFO stEventInfo(SipStack::SIP::EVENT_TYPE_STACK_TIMER_REGISTER, pstTimerInfo, sizeof(*pstTimerInfo));
    //��Ӷ�ʱ��ע���¼�
    long lResult = SipStack::g_pSipStack->AddEvent(stEventInfo);
    if (SipStack::SIP::RET_CODE_OK != lResult)
    {
        ERROR_LOG("AddEvent failed,timer_id=%d,timer_param=%d,timer_length=%d",pstTimerInfo->ulTimerId,ulTimerParam,ulTimerLength);
        VOS_DELETE(pstTimerInfo);
        pstTimerInfo = NULL;
        return SIP_RET_FAILURE;
    }

    return SIP_RET_SUCCESS;
}

SS_VOID SipStopTimer
(
    IN SS_UINT32  ulTimerId
)
{
    if (VOS_NULL == SipStack::g_pSipStack)
    {
        ERROR_LOG("stop timer - invalid param.");
        return ;
    }

    //�����¼���Ϣ�������¼��߳����ͷŸ��ڴ�
    SipStack::SIP::EVENT_INFO_TIMER_CANCEL* pstEventInfo = VOS_NEW(pstEventInfo);
    if (VOS_NULL == pstEventInfo)
    {
        ERROR_LOG("stop timer - alloc event.");
        return ;
    }

    pstEventInfo->ulTimerId = ulTimerId;  //��ʱ��ID

    SipStack::SIP::EVENT_INFO stEventInfo(SipStack::SIP::EVENT_TYPE_STACK_TIMER_CANCEL, pstEventInfo, sizeof(*pstEventInfo));
    //��Ӷ�ʱ��ע���¼�
    long lResult = SipStack::g_pSipStack->AddEvent(stEventInfo);
    if (SipStack::SIP::RET_CODE_OK != lResult)
    {
        VOS_DELETE(pstEventInfo);
        ERROR_LOG("AddEvent failed,timer_id=%d", ulTimerId);
        return ;
    }

    //INFO_LOG("stop timer - success - timer_id=%d.", ulTimerId);
    return ;
}

SS_VOID SipTxnHiSfReqTimeout
(
 IN SS_UINT32         ulTuObjId,
 IN SS_UINT32         ulTxnObjId,
 IN SipDataUnitS     *pstSipSdu,
 IN SipTxnHiAuxDataS *pstAuxData
)
{
    DEBUG_LOG("SipTxnHiSfReqTimeout:mini sip call me.");
    ulTuObjId   = ulTuObjId;
    ulTxnObjId  = ulTxnObjId;
    pstSipSdu   = pstSipSdu;
    pstAuxData  = pstAuxData;
}

EN_SIP_RESULT SipTxnHiSfReqInd
(
 IN  SS_UINT32         ulTxnObjId,
 IN  SipDataUnitS     *pstSipSdu,
 IN  SipTptNwInfoS    *pstTptNwInfo,
 IN  SipTxnHiAuxDataS *pstAuxData,
 OUT SS_UINT32        *pulTuObjId
)
{
    DEBUG_LOG("SipTxnHiSfReqInd:mini sip call me.");

    //�����������账��
    pstAuxData   = pstAuxData;
    pstTptNwInfo = pstTptNwInfo;
    //�������������޸�
    pstSipSdu    = pstSipSdu;

    //�������
    if ( VOS_NULL == pstSipSdu
      || VOS_NULL == pstSipSdu->pstSipMsg
      || VOS_NULL == pstTptNwInfo
      || VOS_NULL == SipStack::g_pSipStack
       )
    {
        DEBUG_LOG("SipTxnHiSfReqInd:invalid args");
        return SIP_RET_FAILURE;
    }

    //����TU����ID
    SS_UINT32 ulTuObjId = SipStack::g_pSipStack->GetTuObjId();
    *pulTuObjId = ulTuObjId;

    //������״̬������Ϣ
    SipStack::g_pSipStack->HandleSipTxnHiSfReqInd(ulTuObjId, ulTxnObjId, *pstTptNwInfo, *pstSipSdu->pstSipMsg);

    return SIP_RET_SUCCESS;
}

SS_VOID SipTxnHiSfRspInd
(
    IN SS_UINT32         ulTuObjId,
    IN SS_UINT32         ulTxnObjId,
    IN SipDataUnitS     *pstSipSdu,
    IN SipTptNwInfoS    *pstTptNwInfo,
    IN SipTxnHiAuxDataS *pstAuxData
)
{
    DEBUG_LOG("SipTxnHiSfRspInd:minisip call me.");

    pstSipSdu       = pstSipSdu;
    pstTptNwInfo    = pstTptNwInfo;
    pstAuxData      = pstAuxData;

    if ( VOS_NULL == pstSipSdu
      || VOS_NULL == pstSipSdu->pstSipMsg
      || VOS_NULL == pstTptNwInfo
      || VOS_NULL == SipStack::g_pSipStack
       )
    {
        ERROR_LOG("SipTxnHiSfReqInd:invalid args");
        return ;
    }

    SipStack::g_pSipStack->HandleSipTxnHiSfRspInd(ulTuObjId, ulTxnObjId, *pstTptNwInfo, *pstSipSdu->pstSipMsg);

    INFO_LOG("SipTxnHiSfRspInd Success."); 
}

SS_VOID SipTxnHiSfMatchedCancelInd
(
 IN SS_UINT32         ulCanceledTuObjId,
 IN SS_UINT32         ulCanceledTxnObjId,
 IN SipDataUnitS     *pstSipSdu,
 IN SipTptNwInfoS    *pstTptNwInfo,
 IN SipTxnHiAuxDataS *pstAuxData
 )
{
    DEBUG_LOG("SipTxnHiSfMatchedCancelInd:minisip call me.");
    ulCanceledTuObjId = ulCanceledTuObjId;
    ulCanceledTxnObjId  = ulCanceledTxnObjId;
    pstSipSdu           = pstSipSdu;
    pstTptNwInfo        = pstTptNwInfo;
    pstAuxData          = pstAuxData;
}

EN_SIP_RESULT SipTptLiStrSend
(
 IN SS_UINT32         ulMsgUsrId,
 IN SS_UINT32         ulMsgUsrTimeStamp,
 IN SipString        *pstStrToTptD,
 IN SS_UINT32         ulMsgFlag,
 IN SipTptNwInfoS    *pstTptNwInfo,
 IN SS_INT            iTmpConPort,
 IN SipVia           *pstViaHdr,
 IN SipTptLiAuxDataS *pstAuxData
 )
{
    DEBUG_LOG("SipTptLiStrSend:minisip call me.");

    //���в������账�����߲����޸�
    ulMsgUsrId          = ulMsgUsrId;
    ulMsgUsrTimeStamp   = ulMsgUsrTimeStamp;
    pstStrToTptD        = pstStrToTptD;
    ulMsgFlag           = ulMsgFlag;
    pstTptNwInfo        = pstTptNwInfo;
    iTmpConPort         = iTmpConPort;
    pstViaHdr           = pstViaHdr;
    pstAuxData          = pstAuxData;

    //��ӡ��־ǰ���滻���һ���ַ�Ϊ������
    char& cEnd = pstStrToTptD->pcData[pstStrToTptD->ulLen - 1];
    const char cEndValue = cEnd;
    cEnd = '\0';
    DEBUG_LOG("ready to send sip message......dst_addr=%d.%d.%d.%d:%d,length=%d,data=\n%s",
		(unsigned int)pstTptNwInfo->stDstAddr.u.aucIPv4Addr[0], 
		(unsigned int)pstTptNwInfo->stDstAddr.u.aucIPv4Addr[1],
		(unsigned int)pstTptNwInfo->stDstAddr.u.aucIPv4Addr[2],
		(unsigned int)pstTptNwInfo->stDstAddr.u.aucIPv4Addr[3],
		(unsigned int)pstTptNwInfo->stDstAddr.iPort,
		pstStrToTptD->ulLen,
		pstStrToTptD->pcData);
    cEnd = cEndValue;//lint !e838

    if (0 == g_ulSipUdpSocket)
    {
        DEBUG_LOG("failure to send sip message......invalid socket.");
        return SIP_RET_FAILURE;
    }

    sockaddr_in stAddr = {0};
    stAddr.sin_family = AF_INET;
    SS_UINT8* pIP = (SS_UINT8*)&stAddr.sin_addr.s_addr;
    pIP[0] = pstTptNwInfo->stDstAddr.u.aucIPv4Addr[0];
    pIP[1] = pstTptNwInfo->stDstAddr.u.aucIPv4Addr[1];
    pIP[2] = pstTptNwInfo->stDstAddr.u.aucIPv4Addr[2];
    pIP[3] = pstTptNwInfo->stDstAddr.u.aucIPv4Addr[3];
    
   // stAddr.sin_addr.s_addr = inet_addr(pIP);
    stAddr.sin_port = htons((unsigned short)pstTptNwInfo->stDstAddr.iPort);

    // cout<<"send to addr: "<<pIP[0]<<"."<<pIP[1]<<"."<<pIP[2]<<"."<<pIP[3]<<":"<<(unsigned short)pstTptNwInfo->stDstAddr.iPort<<endl;

    int iResult = sendto(g_ulSipUdpSocket, pstStrToTptD->pcData, (int)pstStrToTptD->ulLen, 0, (sockaddr*)&stAddr, sizeof(sockaddr));
    if (SOCKET_ERROR == iResult)
    {
        DEBUG_LOG("failure to send sip message......send_result=%d",CONN_ERRNO); 
        return SIP_RET_FAILURE;
    }

    DEBUG_LOG("success to send sip message......send_size=%d",iResult); 
    return SIP_RET_SUCCESS;
}

EN_SIP_RESULT SipTxnHiSlRspInd
(
 IN SipDataUnitS     *pstSipSdu,
 IN SipTptNwInfoS    *pstTptNwInfo,
 IN SipTxnHiAuxDataS *pstAuxData
 )
{
    DEBUG_LOG("SipTxnHiSlRspInd:minisip call me.");
    pstSipSdu       = pstSipSdu;
    pstTptNwInfo    = pstTptNwInfo;
    pstAuxData      = pstAuxData;
    return EN_SIP_RESULT(0);
}

EN_SIP_RESULT SipTxnHiSlReqInd
(
 IN SipDataUnitS     *pstSipSdu,
 IN SipTptNwInfoS    *pstTptNwInfo,
 IN SipTxnHiAuxDataS *pstAuxData
 )
{
    DEBUG_LOG("SipTxnHiSlReqInd:minisip call me.");
    pstSipSdu       = pstSipSdu;
    pstTptNwInfo    = pstTptNwInfo;
    pstAuxData      = pstAuxData;

    if ( VOS_NULL == pstSipSdu
      || VOS_NULL == pstSipSdu->pstSipMsg
      || VOS_NULL == pstTptNwInfo
      || VOS_NULL == SipStack::g_pSipStack
       )
    {
        ERROR_LOG("SipTxnHiSfReqInd:invalid args");
        return SIP_RET_FAILURE;
    }

    //������״̬������Ϣ
    SipStack::g_pSipStack->HandleSipTxnHiSlReqInd(*pstTptNwInfo, *pstSipSdu->pstSipMsg);

    INFO_LOG("SipTxnHiSlReqInd Success.");
    return SIP_RET_SUCCESS;
}

SS_VOID SipLmLogCumwHandler
(
    SS_UINT16          usComponentId,
    EN_SIP_LOG_LEVEL   enLogLevel,
    SS_UINT16          usFileId,
    SS_UINT16          usFunctionId,
    SS_UINT16          usLineNo,
    SS_UINT16          usLogStringId
)
{
    //INFO_LOG("SipLmLogCumwHandler In. ComponentID = %d. LogLevel = %d. FileID = %d. FunctionID = %d. LineNo = %d."
        //"LogStringID = %d.", usComponentId, enLogLevel, usFileId, usFunctionId, usLineNo, usLogStringId);
    //DEBUG_LOG("SipLmLogCumwHandler:ID=file" << usFileId << ".func" << usFunctionId << ".line" << usLineNo << ".str" << usLogStringId << ",thread_id="  << (unsigned long long)ACE_OS::thr_self();
}

/**
  EN_SIP_RESULT (*pFnSipTxnHiSlMsgSendResultInd)
  (
  IN SS_UINT32 ulTuObjId,
  IN SS_UINT32 ulTuObjTimeStamp,
  IN EN_SIP_SEND_RESULT enResultType,
  IN SipTptNwInfoS *pstTptNwInfo,
  IN SipTxnHiAuxDataS *pstAuxData
  )
  */
EN_SIP_RESULT SipTxnHiSlMsgSendResultInd
(
  IN SS_UINT32 ulTuObjId,
  IN SS_UINT32 ulTuObjTimeStamp,
  IN EN_SIP_SEND_RESULT enResultType,
  IN SipTptNwInfoS *pstTptNwInfo,
  IN SipTxnHiAuxDataS *pstAuxData
)
{
    DEBUG_LOG("SipTxnHiSlMsgSendResultInd:minisip call me.");
    return SIP_RET_SUCCESS;
}

/**
  EN_SIP_RESULT (*pFnSipTxnHiSfUnmatchedCancelInd)
  (
  IN SS_UINT32 ulTxnObjId,
  IN SipDataUnitS *pstSipSdu,
  IN SipTptNwInfoS *pstTptNwInfo,
  IN SipTxnHiAuxDataS *pstAuxData,
  IO SS_UINT32 *pulTuObjId
  )
  */
  EN_SIP_RESULT SipTxnHiSfUnmatchedCancelInd
  (
  IN SS_UINT32 ulTxnObjId,
  IN SipDataUnitS *pstSipSdu,
  IN SipTptNwInfoS *pstTptNwInfo,
  IN SipTxnHiAuxDataS *pstAuxData,
  IO SS_UINT32 *pulTuObjId
  )
{
    DEBUG_LOG("SipTxnHiSfUnmatchedCancelInd:minisip call me.");
    return SIP_RET_SUCCESS;
}

/**
* Description:  InitMiniSip().  ��ʼ��MiniSip
* @return       long.       ������
*/
long SipStack::SIP::InitMiniSip()
{
    INFO_LOG("Init MiniSip Begin.");

    //��ʼ��MiniSIP
    SipInitParaS stInitpara = {0};

    //��ѡ����
    SipCoreLibMandConfigS stMandCfg = {0};
    stMandCfg.usMaxObjs         = MAX_NUM_TRANSACTIONS;    //���������
    stMandCfg.usMaxNumOfExtHdrs = EX_HDR_ID_MAX;           //�ⲿSIPͷ������

    //��ѡ����
    SipCoreLibOptConfigS stOptCfg = {0};

    SipCodecOptCfgS stCodecOptCfg = {0};
    stCodecOptCfg.bEncInMultipleLines = SS_TRUE;    //��Viaʱ���ֲ�Ϊ���ͷ��

    stOptCfg.pstCodecCfg = &stCodecOptCfg;

    stInitpara.pstMandCfgPara   = &stMandCfg;
    stInitpara.pstOptCfgPara    = &stOptCfg;
    
    //MiniSip��������ֵ
    EN_SIP_RESULT enResult = SIP_RET_SUCCESS;

    //��ʼ��
    enResult = SipLmCoreLibInit(&stInitpara);//lint !e838
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipLmCoreLibInit Failed On Init MiniSip. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����־�ص�
    enResult = SipLmRegLogHndlr(SipLmLogCumwHandler);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipLmRegLogHndlr Failed On Init MiniSip. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    // register tu callback functions
    SipTxnTuOptRegFnS sipTxnTuOptRegFns;
    sipTxnTuOptRegFns.pfnSfUnmatchedCancelInd = SipTxnHiSfUnmatchedCancelInd;
    sipTxnTuOptRegFns.pfnSlMsgSendResultInd = SipTxnHiSlMsgSendResultInd;
    if (SIP_RET_SUCCESS != SipTxnRegTxnTuOptIntf(&sipTxnTuOptRegFns))
    {
        //ERROR_LOG("SipTxnRegTxnTuOptIntf  Init Failed . Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
	    return RET_CODE_FAIL;
    }

    //ע����չͷ��
    long lResult = RegExHeaders();
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Register Extend Headers Failed On Init MiniSip.");
        return RET_CODE_FAIL;
    }

    //��ʼ������ִ�������
    (void)SipUtilRandInit();

    INFO_LOG("Init MiniSip Success.");
    return RET_CODE_OK;
}

/**
* Description:  RegExHeader().  ע����չͷ��
* @return       long.       ������
*/
long SipStack::SIP::RegExHeaders()
{
    //MiniSip��������ֵ
    EN_SIP_RESULT enResult = SIP_RET_SUCCESS;

    //ע����չͷ��֮Max-Forwards
    enResult = SipCodecRegHeader(EX_HDR_ID_MAX_FORWARDS, SipHdrMaxForwardsInit);//lint !e838
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Max-Forwards Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮User-Agent
    enResult = SipCodecRegHeader(EX_HDR_ID_USER_AGENT, SipHdrUserAgentInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader User-Agent Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮Authorization
    enResult = SipCodecRegHeader(EX_HDR_ID_AUTHORIZATION, SipHdrAuthorizationInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Authorization Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮Expires
    enResult = SipCodecRegHeader(EX_HDR_ID_EXPIRES, SipHdrExpiresInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Expires Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮Content-Type
    enResult = SipCodecRegHeader(EX_HDR_ID_CONTENT_TYPE, SipHdrContentTypeInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Content-Type Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮WWW-Authenticate
    enResult = SipCodecRegHeader(EX_HDR_ID_WWW_AUTHENTICATE, SipHdrWWWAuthenicateInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader WWW-Authenticate Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮Authentication-Info
    enResult = SipCodecRegHeader(EX_HDR_ID_AUTHENTICATION_INFO, SipHdrAuthenticationInfoInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Authentication-Info Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮Subject
    enResult = SipCodecRegHeader(EX_HDR_ID_SUBJECT, SipHdrSubjectInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Subject Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //ע����չͷ��֮reason
    enResult = SipCodecRegHeader(EX_HDR_ID_REASON, SipHdrReasonInit);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipCodecRegHeader Reason Failed On Register Extend Headers. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

	//ע����չͷ��֮Date
	//enResult = SipCodecRegHeader(EX_HDR_ID_DATE, SipHdrDateInit);
	//if (SIP_RET_SUCCESS != enResult)
	//{
	//	//ERROR_LOG("SipCodecRegHeader Date Failed On Register Extend Headers. Error = %s.", 
	//		STR_ARR_EN_SIP_RESULT[enResult]);
	//	return RET_CODE_FAIL;
	//}

    return RET_CODE_OK;
}

/**
* Description:  CreateReqMsg(). ����SIP������Ϣ����������CSeqͷ��
* @param  [in]  enSipMethod SIP�������� 
* @param  [out] pstSipMsg   SIP��Ϣָ��
* @return       long.       ������
*/
long SipStack::SIP::CreateReqMsg
(
    EN_SIP_METHOD   enSipMethod, 
    SipMsgS*&       pstSipMsg
)
{
    //��������
    SipToken stTokenMethod = {0};
    stTokenMethod.usTokenId = (unsigned short)enSipMethod;

    //����������Ϣ���ᴴ��CSeqͷ�򣬵�������������Cseq��ֵ
    EN_SIP_RESULT enResult = SipDsmCreateReqMsg(&stTokenMethod, &pstSipMsg);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipDsmCreateReqMsg Failed On Create SIP Request Message. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

	/*ÿ�η���ֱ����������
    //����CSeq
    pstSipMsg->stHeaders.pstCseq->ulSequence = ulSequence;
    */

    return RET_CODE_OK;
}

/**
* Description:  SetSipTptIpPort().   ����SipTptIpPort
* @param  [out] stTptIpPort �����õ�SipTptIpPortS����
* @param  [in]  objAddr     ��·��ַ
* @return       void.
*/
void SipStack::SIP::SetSipTptIpPort
(
    SipTptIpPortS&      stTptIpPort, 
    const CNetworkAddr& objAddr
)
{
    stTptIpPort.iPort = ntohs(objAddr.m_usPort);
    unsigned char* pIP = (unsigned char*)&objAddr.m_lIpAddr;//lint !e1773
    stTptIpPort.u.aucIPv4Addr[0] = pIP[0];
    stTptIpPort.u.aucIPv4Addr[1] = pIP[1];
    stTptIpPort.u.aucIPv4Addr[2] = pIP[2];
    stTptIpPort.u.aucIPv4Addr[3] = pIP[3];
}

/**
* Description:  SetSipTptNwInfo().  ����SipTptNwInfo
* @param  [out] stTptNwInfo �����õ�SipTptNwInfoS����
* @param  [in]  objDstAddr  Ŀ����·��ַ
* @param  [in]  objSrcAddr  Դ��·��ַ
* @return       long.       ������
*/
void SipStack::SIP::SetSipTptNwInfo
(
    SipTptNwInfoS&      stTptNwInfo,
    const CNetworkAddr& objDstAddr, 
    const CNetworkAddr& objSrcAddr
)
{
    //IPV4
    stTptNwInfo.ucAddressType   = SIP_ADDR_TYPE_IPV4;
    //UDP
    stTptNwInfo.ucProtocol      = SIP_TRANSPORT_UDP;
    //����Ŀ���ַ
    SetSipTptIpPort(stTptNwInfo.stDstAddr, objDstAddr);
    //����Դ��ַ
    SetSipTptIpPort(stTptNwInfo.stSrcAddr, objSrcAddr);
}

/**
* Description:  CreateSipString().  ����SipString
* @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
* @param  [out] pstSipString    ��������SipString
* @param  [in]  szContent       ����
* @return       long.           ������
*/
long SipStack::SIP::CreateSipString
(
    SipMemCpHdl     hdlMemCp, 
    VppStringS*&    pstSipString, 
    const char*     szContent
)
{
    //���ݿ���Ϊ��
    if (VOS_NULL == szContent)
    {
        return RET_CODE_OK;
    }

    VPP_UINT32 nResult = VppStringCreate(hdlMemCp, (char*)szContent, strlen(szContent), &pstSipString);//lint !e1773
    if (VPP_SUCCESS != nResult)
    {
        ERROR_LOG("VppStringCreate Failed On Create SIP String. ErrorCode = %d. Content = %s",nResult,szContent);
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  CreateQuoteString().  ���������ŵ�SipString
* @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
* @param  [out] pstSipString    ��������SipString
* @param  [in]  szContent       ����
* @return       long.           ������
*/
long SipStack::SIP::CreateQuoteString
(
    SipMemCpHdl     hdlMemCp, 
    VppStringS*&    pstSipString,
    const char*     szContent
)
{
    //���ݿ���Ϊ��
    if (VOS_NULL == szContent)
    {
        return RET_CODE_OK;
    }

    //����˫���ŵĳ���
    const unsigned long ulQuoteStrLen = strlen(szContent) + 3;

    VPP_UINT32 nResult = VppStringCreate(hdlMemCp, VOS_NULL, ulQuoteStrLen, &pstSipString);
    if (VPP_SUCCESS != nResult)
    {
        ERROR_LOG("VppStringCreate Failed On Create SIP String. ErrorCode = %d. Content = %s",nResult,szContent);
        return RET_CODE_FAIL;
    }

    //����˫����
    (void)snprintf(pstSipString->pcData, ulQuoteStrLen, "\"%s\"", szContent);
    pstSipString->ulLen = ulQuoteStrLen-1;

    return RET_CODE_OK;
}

/**
* Description:  CreateSipStringData().  ����SipString������
* @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
* @param  [out] stSipString     ��������SipString������
* @param  [in]  szContent       ����
* @return       long.           ������
*/
long SipStack::SIP::CreateSipStringData
(
    SipMemCpHdl     hdlMemCp, 
    VppStringS&     stSipString, 
    const char*     szContent
)
{
    //���ݿ���Ϊ��
    if (VOS_NULL == szContent)
    {
        return RET_CODE_OK;
    }
    
    VPP_UINT32 nResult = VppStringDataCreate(hdlMemCp, (char*)szContent, strlen(szContent), &stSipString);//lint !e1773
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("VppStringDataCreate Failed On Create SIP String. ErrorCode = %d. Content = %s.", nResult, szContent);
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetSipString(). ����SipString
* @param  [in]  stSipString     ��������SipString
* @param  [in]  szContent       ����
* @return       void.
*/
void SipStack::SIP::SetSipString
(
    SipString&  stSipString, 
    const char* szContent
)
{
    stSipString.pcData = (char*)szContent;//lint !e1773
    stSipString.ulLen  = strlen(szContent);
}

/**
* Description:  SetSipHostPort().   ����SipHostPort
* @param  [out] stHostPort  �����õ�HostPort����
* @param  [in]  objAddr     ��·��ַ
* @return       void.
*/
void SipStack::SIP::SetSipHostPort
(
    SipHostPort&        stHostPort, 
    const CNetworkAddr& objAddr
)
{
    stHostPort.iPort = ntohs(objAddr.m_usPort);
    stHostPort.stHost.enHostType = SIP_ADDR_TYPE_IPV4;
    SS_UINT8* pIP = (SS_UINT8*)&objAddr.m_lIpAddr;//lint !e1773
    stHostPort.stHost.uHostContent.ipv4[0] = pIP[0];
    stHostPort.stHost.uHostContent.ipv4[1] = pIP[1];
    stHostPort.stHost.uHostContent.ipv4[2] = pIP[2];
    stHostPort.stHost.uHostContent.ipv4[3] = pIP[3];
}

void SipStack::SIP::SetSipHostPort_New
(
    SipHostPort&         sip_host_port_st_r, 
    const string&   ip_str_r,
    const unsigned short port_us
)
{
    //stHostPort.iPort = ntohs(objAddr.m_usPort);
    //stHostPort.stHost.enHostType = SIP_ADDR_TYPE_IPV4;
    //SS_UINT8* pIP = (SS_UINT8*)&objAddr.m_lIpAddr;
    //stHostPort.stHost.uHostContent.ipv4[0] = pIP[0];
    //stHostPort.stHost.uHostContent.ipv4[1] = pIP[1];
    //stHostPort.stHost.uHostContent.ipv4[2] = pIP[2];
    //stHostPort.stHost.uHostContent.ipv4[3] = pIP[3];
    //(long)inet_addr

    //����Э������
    sip_host_port_st_r.stHost.enHostType = SIP_ADDR_TYPE_IPV4;

    //���ö˿�
    sip_host_port_st_r.iPort = port_us;

    //������ַ
    unsigned long addr = (unsigned long)ACE_OS::inet_addr(ip_str_r.c_str());
    SS_INT8* ip_p = (SS_INT8*)&addr;
    sip_host_port_st_r.stHost.uHostContent.ipv4[0] = (SS_UCHAR)ip_p[0];
    sip_host_port_st_r.stHost.uHostContent.ipv4[1] = (SS_UCHAR)ip_p[1];
    sip_host_port_st_r.stHost.uHostContent.ipv4[2] = (SS_UCHAR)ip_p[2];
    sip_host_port_st_r.stHost.uHostContent.ipv4[3] = (SS_UCHAR)ip_p[3];
}

/**
* Description:  SetSipHostName().    ����HostName
* @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
* @param  [out] stHostPort      �����õ�HostPort����
* @param  [in]  pszHostName     ������
* @return       long.   ����ֵ
*/
long SipStack::SIP::SetSipHostName
(
    SipMemCpHdl     hdlMemCp, 
    SipHostPort&    stHostPort, 
    const char*     pszHostName
)
{
    stHostPort.iPort = SIP_INVALID_VALUE;
    stHostPort.stHost.enHostType = SIP_ADDR_TYPE_HOST_NAME;

    //����HostName
    long lResult = CreateSipString(hdlMemCp, stHostPort.stHost.uHostContent.pstrHostName, pszHostName);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Host Name Failed On Set Host Name.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetURI().   ����URI
* @param  [in]  hdlMemCp    �ڴ���ƿ�ָ��
* @param  [out] stURI       �����õ�URI����
* @param  [in]  pszUriName  URI��
* @return       long.       ������
*/
long SipStack::SIP::SetURI
(
    SipMemCpHdl hdlMemCp, 
    URI&        stURI, 
    const char* pszUriName
)
{
    stURI.enURISchemeType = SIP_URI_SCH_SIP;

    //���ڴ���ƿ��Ϸ����ڴ�
    stURI.uri.pstSipUri = (SipURI*)SipMemCpMalloc(hdlMemCp, sizeof(SipURI));
    if (SS_NULL_PTR == stURI.uri.pstSipUri)
    {
        //ERROR_LOG("SipMemCpMalloc Failed On Set URI.");
        return RET_CODE_FAIL;
    }

    //TTL���ݲ�ʹ��
    stURI.uri.pstSipUri->iTtl = SIP_INVALID_VALUE;

    //����URI UserName
    long lResult = CreateSipString(hdlMemCp, stURI.uri.pstSipUri->pstrUserName, pszUriName);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create URI User Name Failed On Set URI.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetUriByDomain().   ��������URI
* @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
* @param  [out] stURI           �����õ�URI����
* @param  [in]  pszUriName      URI��
* @param  [in]  pszUriDomain    URI��
* @return       long.       ������
*/
long SipStack::SIP::SetUriByDomain
(
    SipMemCpHdl     hdlMemCp, 
    URI&            stURI, 
    const char*     pszUriName, 
    const char*     pszUriDomain
)
{
    long lResult = RET_CODE_OK;

    lResult = SetURI(hdlMemCp, stURI, pszUriName);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set URI Failed By Domain.");
        return RET_CODE_FAIL;
    }

    lResult = SetSipHostName(hdlMemCp, stURI.uri.pstSipUri->stHostPort, pszUriDomain);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set Host Name Failed On Set URI By Domain.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetUriByAddr(). ���õ�ַ��URI
* @param  [in]  hdlMemCp        �ڴ���ƿ�ָ��
* @param  [out] stURI           �����õ�URI����
* @param  [in]  pszUriName      URI��
* @param  [in]  objAddr         ��·��ַ
* @return       long.       ������
*/
long SipStack::SIP::SetUriByAddr
(
    SipMemCpHdl         hdlMemCp, 
    URI&                stURI, 
    const char*         pszUriName, 
    const CNetworkAddr& objAddr
)
{
    long lResult = RET_CODE_OK;

    lResult = SetURI(hdlMemCp, stURI, pszUriName);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set URI Failed By Domain.");
        return RET_CODE_FAIL;
    }

    SetSipHostPort(stURI.uri.pstSipUri->stHostPort, objAddr);

    return RET_CODE_OK;
}

/**
* Description:  SetRequestUri().    ����Request-URI
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszUriName      URI��
* @param  [in]  pszUriDomain    URI��
* @return       long.       ������
*/
long SipStack::SIP::SetRequestUri
(
    SipMsgS&        stSipMsg,
    const char*     pszUriName, 
    const char*     pszUriDomain
)
{
    //Request URI
    long lResult = SetUriByDomain(stSipMsg.hdlMemCp, 
                                  stSipMsg.uFirstLine.stRequestLine.stRequestURI, 
                                  pszUriName,
                                  pszUriDomain);
    return lResult;
}

/**
* Description:  SetCallId().  ����Call-IDͷ��
* @param  [in]          stSipMsg        Sip��Ϣ�ṹ������
* @param  [in/out]  strCallId       CallID�ַ�����ֵ��Ϊ��ʱ���������
* @return       long.       ������
*/
long SipStack::SIP::SetCallId
(
    SipMsgS&    stSipMsg,
    string&     strCallId
)
{    
    //CallID
	SipCallID* pCallID = (SipCallID*)SipDsmCreateHdrInMsg(SIP_BASIC_HDR_ID_CALLID, &stSipMsg);
	if(NULL == pCallID)
	{
		ERROR_LOG("SipDsmCreateHdrInMsg return NULL.");
		return RET_CODE_FAIL;
	}
    SipCallID& stCallID = (*pCallID);

    //ֱ��ʹ������ֵ
    if (!strCallId.empty())
    {
        long lResult = CreateSipStringData(stSipMsg.hdlMemCp, stCallID, strCallId.c_str());

        if (RET_CODE_OK != lResult)
        {
            //ERROR_LOG("Set Call-ID Value Failed On Set Call-ID Header.");
            return RET_CODE_FAIL;
        }
        
        return RET_CODE_OK;
    }
    
    //���������Call-ID
    EN_SIP_RESULT enResult = SipTxnUtilGenTag(stSipMsg.hdlMemCp, &stCallID);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("Create Call-ID Failed On Set Call-ID Header. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    //����buffer����󳤶ȣ����Ͻ������ĳ���
	unsigned long ulBufLen = 0;
	if(ULONG_MAX - stCallID.ulLen > 1)
	{
		ulBufLen = stCallID.ulLen + 1;
	}
	else
	{
		ERROR_LOG("stCallID.ulLen is too large");
		return RET_CODE_FAIL;
	}

    char* pBuffer = VOS_NEW(pBuffer, ulBufLen);
    if (VOS_NULL == pBuffer)
    {
        //ERROR_LOG("Create Buffer Failed On Set Call-ID Header.");
        return RET_CODE_FAIL;
    }
    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, stCallID.pcData, ulBufLen - 1);

    //����Call-ID
    strCallId = pBuffer;

    VOS_DELETE(pBuffer, MULTI);
    return RET_CODE_OK;
}

/**
* Description:  SetFrom().  ����Fromͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszUriName      URI��
* @param  [in]  pszUriDomain    URI��
* @return       long.       ������
*/
long SipStack::SIP::SetFrom
(
    SipMsgS&        stSipMsg,
    const char*     pszUriName, 
    const char*     pszUriDomain
)
{
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    //From
	SipFrom* pSipFrom = (SipFrom*)SipDsmCreateHdrInMsg(SIP_BASIC_HDR_ID_FROM, &stSipMsg);
	if(NULL == pSipFrom)
	{
		WARN_LOG("SipDsmCreateHdrInMsg return null.");
		return RET_CODE_FAIL;
	}
    SipFrom& stFrom = *pSipFrom;

    /*ÿ�η���ֱ����������
    //From Tag
    EN_SIP_RESULT enResult = SipTxnUtilGenTag(stSipMsg.hdlMemCp, &stFrom.strTag);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("Create From Tag Failed On Set From Header. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }
    */

    //From DisplayName
	if (SIP_METHOD_INVITE != stSipMsg.pstSipMethod->usTokenId)
	{
		lResult = CreateSipString(stSipMsg.hdlMemCp, stFrom.pstrDisplayName, pszUriName);
		//���Բ����ã���ʧ��ʱ����ӡ�澯��־
		if (RET_CODE_OK != lResult)
		{
			//WARN_LOG("Create From Display Name Failed On Set From Header.");
		}
	}

    //From URI
    lResult = SipStack::SIP::SetUriByDomain(stSipMsg.hdlMemCp, stFrom.stUri, pszUriName, pszUriDomain);
    if (SipStack::SIP::RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set From URI Failed On Set From Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetTo().    ����Toͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszUriName      URI��
* @param  [in]  pszUriDomain    URI��
* @return       long.       ������
*/
long SipStack::SIP::SetTo
(
    SipMsgS&        stSipMsg,
    const char*     pszUriName, 
    const char*     pszUriDomain
)
{
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    //To
	SipTo* pSipTo = (SipTo*)SipDsmCreateHdrInMsg(SIP_BASIC_HDR_ID_TO, &stSipMsg);
	if(NULL == pSipTo)
	{
		WARN_LOG("SipDsmCreateHdrInMsg return null.");
		return RET_CODE_FAIL;
	}
    SipTo& stTo = *pSipTo;

    //To DisplayName
	if (SIP_METHOD_INVITE != stSipMsg.pstSipMethod->usTokenId)
	{
		lResult = CreateSipString(stSipMsg.hdlMemCp, stTo.pstrDisplayName, pszUriName);
		//���Բ����ã���ʧ��ʱ����ӡ�澯��־
		if (RET_CODE_OK != lResult)
		{
			//WARN_LOG("Create To Display Name Failed On Set To Header.");
		}
	}

    //To URI
    lResult = SipStack::SIP::SetUriByDomain(stSipMsg.hdlMemCp, stTo.stUri, pszUriName, pszUriDomain);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set To URI Failed On Set To Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetToTag().    ����To��Tag
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @return       long.       ������
*/
long SipStack::SIP::SetToTag
(
    SipMsgS& stSipMsg
)
{
    if (VOS_NULL == stSipMsg.stHeaders.pstTo)
    {
        //ERROR_LOG("Set To Tag Failed. To Header Not Exist.");
        return RET_CODE_FAIL;
    }

    //To��Tag������ʱ������
    if (VOS_NULL == stSipMsg.stHeaders.pstTo->strTag.pcData)
    {
        //�������To��Tag
        EN_SIP_RESULT enResult = SipTxnUtilGenTag(stSipMsg.hdlMemCp, &stSipMsg.stHeaders.pstTo->strTag);
        if (SIP_RET_SUCCESS != enResult)
        {
            //ERROR_LOG("Create To Tag Failed On Set To Tag. Error = %s.",
                //STR_ARR_EN_SIP_RESULT[enResult]);
            return RET_CODE_FAIL;
        }
    } 

    return RET_CODE_OK;
}//lint !e1764

/**
* Description:  SetVia().          ����Viaͷ��
* @param  [in]  sip_msg_st_r       Sip��Ϣ�ṹ������
* @param  [in]  ip_str_r           ��·��ַ
                port_us            ����˿�
* @return       long.              ������
*/
long SipStack::SIP::SetVia
(
    SipMsgS&             sip_msg_st_r,
    const string&   ip_str_r,
    const unsigned short port_us
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    //����Via�б�
    SipViaListHdl pViaList = (SipViaListHdl)SipDsmCreateHdrInMsg(SIP_BASIC_HDR_ID_VIA_LIST, &sip_msg_st_r);

    //����Via
    SipVia* pstVia = (SipVia*)SipMemCpMalloc(sip_msg_st_r.hdlMemCp, sizeof(SipVia));
    if (SS_NULL_PTR == pstVia)
    {
        //ERROR_LOG("Create Via Header Failed On Set Via Header.");
        return RET_CODE_FAIL;
    }

    SipVia& stVia = *pstVia;

    //��ӵ�Via�б�
    nResult = VppListInsert(pViaList, 0, &stVia);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Insert Via to List Failed On Set Via Header.");
        return RET_CODE_FAIL;
    }

    //Via SIP
    stVia.ulProtocolType = SIP_VIA_PROTOCOL_SIP;
    //Via SIP Version
    SetSipString(stVia.strProtocolVersion, SIP_VERSION_2);
    //Via UDP
    stVia.stTransport.usTokenId = SIP_TRANSPORT_UDP;
    //Via IP Port
    //SetSipHostPort(stVia.stSentBy, objAddr);
    SetSipHostPort_New(stVia.stSentBy, ip_str_r, port_us);

    /*
    //Via Branch
    EN_SIP_RESULT enResult = SipTxnUtilGenBranchId(stSipMsg.hdlMemCp, &stVia.strBranch);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("Create Branch Failed On Set Via Header. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }
    */

    //Via Rport
    //stVia.iRPort = SIP_INVALID_VALUE;
    stVia.iRPort = SIP_VIA_RPORT_PRESENT_NO_VAL;

    return RET_CODE_OK;
}

/**
* Description:  AddExHeader().  �����չͷ��
* @param  [in]  stSipMsg    Sip��Ϣ�ṹ������
* @param  [in]  usHdrId     ��չͷ��ID
* @param  [in]  pvHdrVal    ��չͷ��ֵָ��
* @return       long.       ������
*/
long SipStack::SIP::AddExHeader
(
    SipMsgS&        stSipMsg,
    unsigned short  usHdrId,
    void*           pvHdrVal
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    //�Ƚ�����ͷ��ɾ��
    SipDsmRmvHdrFromMsg(usHdrId, &stSipMsg);

    //�����չͷ���б�ָ��
    SipHeaderListHdl pHeaderList = stSipMsg.stHeaders.hdlExtHeaders;

    if (SS_NULL_PTR == pHeaderList)
    {
        //������չͷ���б�
        nResult = VppListCreate(stSipMsg.hdlMemCp, MAX_NUM_EX_HEADER, 0, &pHeaderList);
        if (VPP_SUCCESS != nResult)
        {
            //ERROR_LOG("Create Extend Header List Failed On Add Extend Header.");
            return RET_CODE_FAIL;
        }

        //������չͷ���б�ָ��
        stSipMsg.stHeaders.hdlExtHeaders = pHeaderList;
    }
    
    //������չͷ��
    SipHdrIdValue* pstHeadIdValue = (SipHdrIdValue*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipHdrIdValue));
    if (SS_NULL_PTR == pstHeadIdValue)
    {
        //ERROR_LOG("Create Extend Header Failed On Add Extend Header.");
        return RET_CODE_FAIL;
    }

    //��ӵ���չͷ���б�
    nResult = VppListInsert(pHeaderList, VppListGetCount(pHeaderList), pstHeadIdValue);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Insert Header To Extend Header List Failed On Add Extend Header.");
        return RET_CODE_FAIL;
    }

    //����ͷ��ID��ֵ
    pstHeadIdValue->usHdrId     = usHdrId;
    pstHeadIdValue->pvHdrVal    = pvHdrVal;

    return RET_CODE_OK;
}

/**
* Description:  SetMaxForwords().   ����Max-Forwardsͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  nMaxForwards    Max-Forwardsֵ
* @return       long.       ������
*/
long SipStack::SIP::SetMaxForwards
(
    SipMsgS&    stSipMsg,
    SS_UINT32   nMaxForwards
)
{
    //�����չͷ��Max-Forwords
    SipMaxForwards* pstMaxForwards = (SipMaxForwards*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipMaxForwards));
    if (SS_NULL_PTR == pstMaxForwards)
    {
        //ERROR_LOG("Create Max-Forwards Header Failed On Set Max-Forwards.");
        return RET_CODE_FAIL;
    }

    //����ֵ
    *pstMaxForwards = nMaxForwards;

    //�����չͷ��
    long lResult = SipStack::SIP::AddExHeader(stSipMsg, EX_HDR_ID_MAX_FORWARDS, pstMaxForwards);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Add Max-Forwards Header Failed On Set Max-Forwards.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetUserAgent(). ����User-Agentͷ��
* @param  [in]  stSipMsg            Sip��Ϣ�ṹ������
* @param  [in]  pszProductName      ��Ʒ����
* @param  [in]  pszProductVersion   ��Ʒ�汾
* @return       long.       ������
*/
long SipStack::SIP::SetUserAgent
(
    SipMsgS&    stSipMsg,
    const char* pszProductName,
    const char* pszProductVersion
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    SipUserAgentListHdl pUserAgentListHdl = NULL;

    //����UserAgent�б����ֻ��1��Ԫ��
    nResult = VppListCreate(stSipMsg.hdlMemCp, 1, 0, &pUserAgentListHdl);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        ERROR_LOG("Create User-Agent Header List Failed On Set User-Agent Header.");
        return RET_CODE_FAIL;
    }

    //����UserAgent
    SipUserAgent* pstUserAgent = (SipUserAgent*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipUserAgent));
    if (SS_NULL_PTR == pstUserAgent)
    {
        ERROR_LOG("Create User-Agent Header Failed On Set User-Agent Header.");
        return RET_CODE_FAIL;
    }

    //����User-Agent�Ĳ�Ʒ����
    lResult = CreateSipString(stSipMsg.hdlMemCp, pstUserAgent->pstrProduct, pszProductName);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set User-Agent Product Name Failed On Set User-Agent Header.");
        return lResult;
    }
    
    //����User-Agent�Ĳ�Ʒ�汾
    lResult = CreateSipString(stSipMsg.hdlMemCp, pstUserAgent->pstrProductVersion, pszProductVersion);
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Set User-Agent Product Version Failed On Set User-Agent Header.");
        return lResult;
    }

    //���UserAgent�б�
    nResult = VppListInsert(pUserAgentListHdl, 0, pstUserAgent);
    if (VPP_SUCCESS != nResult)
    {
        ERROR_LOG("Insert Header To Extend Header List Failed On Set User-Agent Header.");
        return SipStack::SIP::RET_CODE_FAIL;
    }

    //��ӵ���չͷ���б�
    lResult = SipStack::SIP::AddExHeader(stSipMsg, EX_HDR_ID_USER_AGENT, pUserAgentListHdl);
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add User-Agent Header Failed On Set User-Agent Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetRoute(). ����Routeͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  objRouteAddr    ·�ɵ�ַ
* @param  [in]  pszDisplayName  ��ʾ����
* @return       long.       ������
*/
long SipStack::SIP::SetRoute
(
    SipMsgS&            stSipMsg,
    const CNetworkAddr& objRouteAddr,
    const char*         pszDisplayName
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    //����Route�б����ֻ��1��Ԫ��
    nResult = VppListCreate(stSipMsg.hdlMemCp, 1, 0, &stSipMsg.stHeaders.hdlRouteList);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Create Route Header List Failed On Set Route Header.");
        return RET_CODE_FAIL;
    }

    //����Route
    SipRoute* pstRoute = (SipRoute*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipRoute));
    if (SS_NULL_PTR == pstRoute)
    {
        //ERROR_LOG("Create Route Header Failed On Set Route Header.");
        return RET_CODE_FAIL;
    }

    //����Route�ĵ�ַ
    lResult = SetUriByAddr(stSipMsg.hdlMemCp, pstRoute->stUri, NULL, objRouteAddr);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set Route URI Failed On Set Route Header.");
        return lResult;
    }

    //����Route����ʾ����
    lResult = CreateSipString(stSipMsg.hdlMemCp, pstRoute->pstrDisplayName, pszDisplayName);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set Route Display Name Failed On Set Route Header.");
        return lResult;
    }

    //��ӵ�Route�б�
    nResult = VppListInsert(stSipMsg.stHeaders.hdlRouteList, 0, pstRoute);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Insert Header To Route List Failed On Set Route Header.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  AddExHeader().  �����չͷ��
* @param  [in]  stSipMsg                    Sip��Ϣ�ṹ������
* @param  [in]  stSipAuthorization  Sip��Ȩ��Ϣ
* @return       long.       ������
*/
long SipStack::SIP::AddAuthorizationHeader
(
    SipMsgS&            stSipMsg,
    SipAuthorization&   stSipAuthorization
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    //�Ѵ�����ֱ�����
    SipAuthorizationListHdl pAuthorizationListHdl = (SipAuthorizationListHdl)SipDsmGetHdrFromMsg(EX_HDR_ID_AUTHORIZATION, &stSipMsg);
    if (VOS_NULL != pAuthorizationListHdl)
    {
        const unsigned short usAuthNum = VppListGetCount(pAuthorizationListHdl);
        //����������ķ�Χ�ڣ�������ӣ��������´���
        if (SipStack::SIP::MAX_NUM_AUTH_HEADER > usAuthNum)
        {
            //���Authorization���б�
            nResult = VppListInsert(pAuthorizationListHdl, usAuthNum, &stSipAuthorization);
            if (VPP_SUCCESS != nResult)
            {
                //ERROR_LOG("Insert Header To Extend Header List Failed On Add Authorization Header.");
                return RET_CODE_FAIL;
            }
            
            return RET_CODE_OK;
        }
    }

    //�����µ�Authorization�б����ֻ��SipStack::SIP::MAX_NUM_AUTH_HEADER��Ԫ��
    nResult = VppListCreate(stSipMsg.hdlMemCp, SipStack::SIP::MAX_NUM_AUTH_HEADER, 0, &pAuthorizationListHdl);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Create Authorization Header List Failed On Add Authorization Header.");
        return RET_CODE_FAIL;
    }

    //���Authorization���б�
    nResult = VppListInsert(pAuthorizationListHdl, 0, &stSipAuthorization);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Insert Header To Extend Header List Failed On Add Authorization Header.");
        return RET_CODE_FAIL;
    }

    //��ӵ���չͷ���б�
    lResult = AddExHeader(stSipMsg, EX_HDR_ID_AUTHORIZATION, pAuthorizationListHdl);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Add Authorization To Extend Header Failed On Add Authorization Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  AddExHeader().  �����չͷ��
* @param  [in]  stSipMsg                    Sip��Ϣ�ṹ������
* @param  [in]  stSipAuthorization  Sip��Ȩ��Ϣ
* @return       long.       ������
*/
long SipStack::SIP::AddWWWAuthorizationHeader
(
 SipMsgS&            stSipMsg,
 SipWWWAuthenticate&   stSipAuthorization
 )
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    //�Ѵ�����ֱ�����
    SipWWWAuthenticateListHdl pAuthorizationListHdl = (SipWWWAuthenticateListHdl)SipDsmGetHdrFromMsg(EX_HDR_ID_WWW_AUTHENTICATE, &stSipMsg);
    if (VOS_NULL != pAuthorizationListHdl)
    {
        const unsigned short usAuthNum = VppListGetCount(pAuthorizationListHdl);
        //����������ķ�Χ�ڣ�������ӣ��������´���
        if (SipStack::SIP::MAX_NUM_AUTH_HEADER > usAuthNum)
        {
            //���Authorization���б�
            nResult = VppListInsert(pAuthorizationListHdl, usAuthNum, &stSipAuthorization);
            if (VPP_SUCCESS != nResult)
            {
                //ERROR_LOG("Insert Header To Extend Header List Failed On Add Authorization Header.");
                return RET_CODE_FAIL;
            }

            return RET_CODE_OK;
        }
    }

    //�����µ�Authorization�б����ֻ��SipStack::SIP::MAX_NUM_AUTH_HEADER��Ԫ��
    nResult = VppListCreate(stSipMsg.hdlMemCp, SipStack::SIP::MAX_NUM_AUTH_HEADER, 0, &pAuthorizationListHdl);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Create Authorization Header List Failed On Add Authorization Header.");
        return RET_CODE_FAIL;
    }

    //���Authorization���б�
    nResult = VppListInsert(pAuthorizationListHdl, 0, &stSipAuthorization);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Insert Header To Extend Header List Failed On Add Authorization Header.");
        return RET_CODE_FAIL;
    }

    //��ӵ���չͷ���б�
    lResult = AddExHeader(stSipMsg, EX_HDR_ID_WWW_AUTHENTICATE, pAuthorizationListHdl);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Add Authorization To Extend Header Failed On Add Authorization Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetAuthorization(). ����Authorizationͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszUserName     �û���
* @param  [in]  pszPassword     ����
* @param  [in]  stPlatAuthInfo  ƽ̨��Ȩ��Ϣ
* @return       long.       ������
*/
long SipStack::SIP::SetAuthorization
(
    SipMsgS&                stSipMsg,
    char*                   pszUserName,
    char*                   pszPassword,
    const PLAT_AUTH_INFO&   stPlatAuthInfo
)
{
	//����ֵ
    long lResult = RET_CODE_OK;

    //���SIP������Ϣ����
    const unsigned long ulSipMethod = stSipMsg.uFirstLine.stRequestLine.stMethod.usTokenId;

    if (SIP_METHOD_BUTT <= ulSipMethod)
    {
        ERROR_LOG("Set Authorization Header Failed. SIP Method(%d) Can't be Greater Than SIP_METHOD_BUTT",ulSipMethod);
        return RET_CODE_FAIL;
    }

    //����Authorization
    SipAuthorization* pstAuthorization = (SipAuthorization*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipAuthorization));
    if (SS_NULL_PTR == pstAuthorization)
    {
        ERROR_LOG("Create Authorization Header Failed On Set Authorization Header.");
        return RET_CODE_FAIL;
    }

    char szCNonce[MAX_LEN_AUTH_STRING] = {0};
    //cnonce����From Tag
    strncpy(szCNonce, stSipMsg.stHeaders.pstFrom->strTag.pcData, sizeof(szCNonce) - 1);

    //ת��nonce countΪ�ַ�����ֱ�Ӹ���CSeq
    char szNonceCount[MAX_LEN_AUTH_STRING] = {0};
    (void)sprintf_s(szNonceCount, MAX_LEN_AUTH_STRING,"%08x", stSipMsg.stHeaders.pstCseq->ulSequence);

    //��ȡReuqest-URI�ַ���
    const SipURI& stReqUri = *stSipMsg.uFirstLine.stRequestLine.stRequestURI.uri.pstSipUri;

    char* pszDigestUri = VOS_NULL;
    lResult = CreateAuthDigestUri(stReqUri, pszDigestUri);//lint !e838
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Digest URI String Failed On Set Authorization Header.");
        return lResult;
    }    

    //����Response
    HASHHEX HA1;
    HASHHEX Response;

    DigestCalcHA1((char*)"", pszUserName, (char*)stPlatAuthInfo.strRealm.c_str(), //lint !e1773
                  pszPassword, (char*)"", (char*)"", HA1);//lint !e1773
    DigestCalcResponse(HA1, (char*)stPlatAuthInfo.strNonce.c_str(), szNonceCount, szCNonce, //lint !e1773
                       (char*)stPlatAuthInfo.strQop.c_str(), (char*)STR_ARR_SIP_METHOD[ulSipMethod],//lint !e1773
                       pszDigestUri, (char*)"", Response);//lint !e1773

    //Digest��Ȩ��ʽ
    pstAuthorization->bIsAuthDigestType = SS_TRUE;

    //����Authorization�ĸ�ֵ
    lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrDigestUri, pszDigestUri);
    if (RET_CODE_OK != lResult)
    {
        //�ͷ���ʱ�ڴ�
        VOS_DELETE(pszDigestUri, MULTI);
        
        ERROR_LOG("Create Digest URI String Failed On Set Authorization Header.");
        return lResult;
    }

    //�ͷ���ʱ�ڴ�
    VOS_DELETE(pszDigestUri, MULTI);

    lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrUsername, pszUserName);
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create User Name String Failed On Set Authorization Header.");
        return lResult;
    }

    lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrRealm, stPlatAuthInfo.strRealm.c_str());
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Realm String Failed On Set Authorization Header.");
        return lResult;
    }

    // added by dhong
	lResult = CreateSipString(stSipMsg.hdlMemCp, pstAuthorization->pstrAlgorithm, stPlatAuthInfo.stAlgorithm.c_str());
	if (RET_CODE_OK != lResult)
	{
		ERROR_LOG("Create Algorithm String Failed On Set Authorization Header.");
		return lResult;
	}

	if(!stPlatAuthInfo.strQop.empty())
	{
		lResult = CreateSipString(stSipMsg.hdlMemCp, pstAuthorization->pstrMessageQOP, stPlatAuthInfo.strQop.c_str());
		if (RET_CODE_OK != lResult)
		{
			ERROR_LOG("Create Qop String Failed On Set Authorization Header.");
			return lResult;
		}
	}


	lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrNonce, stPlatAuthInfo.strNonce.c_str());
	if (RET_CODE_OK != lResult)
	{
		ERROR_LOG("Create Nonce String Failed On Set Authorization Header.");
		return lResult;
	}

	if(!stPlatAuthInfo.strOpaque.empty())
	{

		lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrOpaque, stPlatAuthInfo.strOpaque.c_str());
		if (RET_CODE_OK != lResult)
		{
			ERROR_LOG("Create Opaque String Failed On Set Authorization Header.");
			return lResult;
		}


		lResult = CreateSipString(stSipMsg.hdlMemCp, pstAuthorization->pstrNonceCount, szNonceCount);
		if (RET_CODE_OK != lResult)
		{
			ERROR_LOG("Create Nonce Count String Failed On Set Authorization Header.");
			return lResult;
		}

		lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrCNonce, szCNonce);
		if (RET_CODE_OK != lResult)
		{
			ERROR_LOG("Create Client Nonce String Failed On Set Authorization Header.");
			return lResult;
		}
	}

    lResult = CreateQuoteString(stSipMsg.hdlMemCp, pstAuthorization->pstrDResponse, Response);
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Response String Failed On Set Authorization Header.");
        return lResult;
    }

    //���Authrizationͷ��SIP��Ϣ
    lResult = AddAuthorizationHeader(stSipMsg, *pstAuthorization);
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Add Authorization Header Failed On Set Authorization Header.");
        return lResult;
    }
    
    return RET_CODE_OK;
}

/**
* Description:  CreateAuthDigestUri().  ����Authorization��Digest��URI
* @param  [in]      stReqUri        SipURI�ṹ������
* @param  [out]     pszDigestUri    DigestUri�ַ������
* @return       long.       ������
*/
long SipStack::SIP::CreateAuthDigestUri
(
    const SipURI&   stReqUri,
    char*&          pszDigestUri
)
{
    //�û����ĳ���
    unsigned long ulReqUriUserNameLen = 0;

    //Register��Request-URIû��UserName�ֶ�
    if (VOS_NULL != stReqUri.pstrUserName)
    {
        ulReqUriUserNameLen = stReqUri.pstrUserName->ulLen;
    };

    //��������
    const unsigned long ulReqUriDomainLen   = stReqUri.stHostPort.stHost.uHostContent.pstrHostName->ulLen;
    //ժҪURI��ʵ�ʳ���
    const unsigned long ulDigestUriLen      = strlen(SIP_URI_SCHEME) + ulReqUriUserNameLen + ulReqUriDomainLen + 2;
    
    if (VOS_NULL == VOS_NEW(pszDigestUri, ulDigestUriLen))
    {
        ERROR_LOG("Create Digest Uri Failed On Set Authorization Header.");
        return RET_CODE_FAIL;
    }
    memset(pszDigestUri, 0, ulDigestUriLen);

	//ժҪ�ַ����ĵ�ǰ����
	unsigned long ulStrlen = strlen(SIP_URI_SCHEME);
	//׷��SIP��URI�ķ�����
	strncpy(pszDigestUri, SIP_URI_SCHEME, ulStrlen);

	//Register��Request-URIû��UserName�ֶ�
	if (0 != ulReqUriUserNameLen)
	{
		//׷���û���
		strncpy(pszDigestUri + ulStrlen, stReqUri.pstrUserName->pcData, ulReqUriUserNameLen);
		strncpy(pszDigestUri+ ulStrlen+ulReqUriUserNameLen ,"@",1);
		//�����û�����@�ĳ���
		ulStrlen += ulReqUriUserNameLen + 1;
	}

	//׷���û���
	strncpy(pszDigestUri + ulStrlen, stReqUri.stHostPort.stHost.uHostContent.pstrHostName->pcData, ulReqUriDomainLen);

    return RET_CODE_OK;
}

/**
* Description:  SetContact(). ����Contactͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  objContactAddr  Contact��ַ
* @param  [in]  pszDisplayName  ��ʾ����
* @return       long.       ������
*/
long SipStack::SIP::SetContact
(
    SipMsgS&            stSipMsg,
    const CNetworkAddr& objContactAddr,
    const char*         pszDisplayName
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    //�Զ��庯������ֵ
    long lResult = RET_CODE_OK;

    //����Contact�б�
    SipContactListHdl pContactList = (SipContactListHdl)SipDsmCreateHdrInMsg(SIP_BASIC_HDR_ID_CONTACT_LIST, &stSipMsg);

    //����Contact
    SipContact* pstContact = (SipContact*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipContact));
    if (SS_NULL_PTR == pstContact)
    {
        //ERROR_LOG("Create Contact Header Failed On Set Contact Header.");
        return RET_CODE_FAIL;
    }

    //���Contact���б�
    nResult = VppListInsert(pContactList, 0, pstContact);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Insert Header To Extend Header List Failed On Set Contact Header.");
        return RET_CODE_FAIL;
    }

    SipContact& stContact = *pstContact;

    //����Contact��URI
    stContact.pstContact = (SipUriHeader*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipUriHeader));
    if (SS_NULL_PTR == stContact.pstContact)
    {
        //ERROR_LOG("Create Contact URI Failed On Set Contact Header.");
        return RET_CODE_FAIL;
    }

    //����Contact URI
	//string dispalyName = "34020000002000000001";    
	//if (SIP_METHOD_INVITE == stSipMsg.pstSipMethod->usTokenId)
	//{
	//	lResult = SetUriByAddr(stSipMsg.hdlMemCp, stContact.pstContact->stUri, dispalyName.c_str(), objContactAddr);
	//	if (RET_CODE_OK != lResult)
	//	{
	//		//ERROR_LOG("Set Contact URI Failed On Set Contact Header.");
	//		return lResult;
	//	}
	//}
	//else
	//{
	lResult = SetUriByAddr(stSipMsg.hdlMemCp, stContact.pstContact->stUri, NULL, objContactAddr);//lint !e838
	if (RET_CODE_OK != lResult)
	{
		//ERROR_LOG("Set Contact URI Failed On Set Contact Header.");
		return lResult;
	}
	//}

    //Contact DisplayName
	if (SIP_METHOD_INVITE != stSipMsg.pstSipMethod->usTokenId && 0 != strlen(pszDisplayName))
	{
		/*string dispalyName = "34020000001230011";*/
		lResult = CreateSipString(stSipMsg.hdlMemCp, stContact.pstContact->pstrDisplayName, pszDisplayName);
		if (RET_CODE_OK != lResult)
		{
			//ERROR_LOG("Create Contact Display Name Failed On Set Contact Header.");
			return lResult;
		}
	}

    //stContact.bIsStarSet = TRUE;

    return RET_CODE_OK;
}

/**
* Description:  SetExpires().   ����Expiresͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  nExpireTime     ��ʱʱ��
* @return       long.       ������
*/
long SipStack::SIP::SetExpires
(
    SipMsgS&    stSipMsg,
    SS_UINT32   nExpireTime
)
{
    //����Expiresͷ��
    SipExpires* pstExpires = (SipExpires*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipExpires));
    if (SS_NULL_PTR == pstExpires)
    {
        //ERROR_LOG("Create Expires Header Failed On Set Expires Header.");
        return RET_CODE_FAIL;
    }

    //����ֵ
    *pstExpires = nExpireTime;

    //�����չͷ��
    long lResult = AddExHeader(stSipMsg, EX_HDR_ID_EXPIRES, pstExpires);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Add Expires Header Failed On Set Expires Header.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetDate(). �����Զ���Dateͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszDate          ʱ��
* @return       long.       ������
*/
long SipStack::SIP::SetDate
    (
    SipMsgS&            stSipMsg,
    const char*         pszDate
    )
{
    SipHdrNameValue* sipHdrNameValue = (SipHdrNameValue*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipHdrNameValue));

    long lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrName, "Date");
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Name Failed On Set Date Header.");
        return lResult;
    }

    lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrValue, pszDate);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Value Failed On Set Date Header.");
        return lResult;
    }

    lResult = SipDsmCloneUnknownHdrToMsg(sipHdrNameValue, &stSipMsg);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Failed On Set Date Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetContentEncoding(). �����Զ���Dateͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszDate          ʱ��
* @return       long.       ������
*/
long SipStack::SIP::SetContentEncoding
    (
    SipMsgS&            stSipMsg,
    const char*         pszEncoding
    )
{
    SipHdrNameValue* sipHdrNameValue = (SipHdrNameValue*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipHdrNameValue));

    long lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrName, "Content-Encoding");
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Name Failed On Set Date Header.");
        return lResult;
    }

    lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrValue, pszEncoding);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Value Failed On Set Date Header.");
        return lResult;
    }

    lResult = SipDsmCloneUnknownHdrToMsg(sipHdrNameValue, &stSipMsg);
    if (RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Date Header Failed On Set Date Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetServer()     ���÷�����ͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  pszServer       �������
* @return       long.           ������
*/
long SipStack::SIP::SetServer
(
    SipMsgS&            stSipMsg,
    const char*         pszServer
)
{
    SipHdrNameValue* sipHdrNameValue = (SipHdrNameValue*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipHdrNameValue));

    long lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrName, "Server");

    if(RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Name Failed On Set Server Header.");
        return lResult;
    }

    lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrValue, pszServer);
    if(RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Date Header Value Failed On Set Server Header.");
        return lResult;
    }

    lResult = SipDsmCloneUnknownHdrToMsg(sipHdrNameValue, &stSipMsg);
    if(RET_CODE_OK != lResult)
    {
        ERROR_LOG("Create Date Header Failed On Set Server Header.");
        return lResult;
    }

    return RET_CODE_OK;
}

/**
 * Description:  SetEvent(). �����Զ���Eventͷ��
 * @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
 * @param  [in]  pszEvent         �¼�
 * @return       long.       ������
 */
long SipStack::SIP::SetEvent
(
 SipMsgS&         stSipMsg,
 const char*         pszEvent
 )
{
	SipHdrNameValue* sipHdrNameValue = (SipHdrNameValue*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipHdrNameValue));

	long lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrName, "Event");
	if (RET_CODE_OK != lResult)
	{
		//ERROR_LOG("Create Event Header Name Failed On Set Event Header.");
		return lResult;
	}

	lResult = SipStack::SIP::CreateSipStringData(stSipMsg.hdlMemCp, sipHdrNameValue->strHdrValue, pszEvent);
	if (RET_CODE_OK != lResult)
	{
		//ERROR_LOG("Create Event Header Value Failed On Set Event Header.");
		return lResult;
	}

	lResult = SipDsmCloneUnknownHdrToMsg(sipHdrNameValue, &stSipMsg);
	if (RET_CODE_OK != lResult)
	{
		ERROR_LOG("Create Event Header Failed On Set Event Header.");
		return lResult;
	}

	return RET_CODE_OK;
}

/**
* Description:  SetExpires().   ����Expiresͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  szSubMediaType       ��ý������
* @return       long.       ������
*/
long SipStack::SIP::SetContentType
(
    SipMsgS&    stSipMsg,
    const char* szSubMediaType
)
{
    //����Content-Typeͷ��
    SipContentType* pstContentType = (SipContentType*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipContentType));
    if (SS_NULL_PTR == pstContentType)
    {
        //ERROR_LOG("Create Content-Type Header Failed On Set Content-Type Header.");
        return RET_CODE_FAIL;
    }

    //Content-Type��MediaType
    SetSipString(pstContentType->strMediaType, MEDIA_TYPE_APPLICATION);
    //Content-Type��SubMediaType
    SetSipString(pstContentType->strSubMediaType, szSubMediaType);

    //�����չͷ��
    long lResult = AddExHeader(stSipMsg, EX_HDR_ID_CONTENT_TYPE, pstContentType);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Add Content-Type Header Failed On Set Content-Type Header.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  SetSubject().   ����Subjectͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  strSubject      Subject�����ַ���
* @return       long.       ������
*/
long SipStack::SIP::SetSubject
(
    SipMsgS&        stSipMsg,
    const string&   strSubject
)
{
    if (strSubject.empty())
    {
        //ERROR_LOG("Set Subject Header Failed. Subject Content is Empty.");
        return RET_CODE_PARA_INVALIDATE;
    }
    
    //����Subjectͷ��
    SipSubject* pstSubject = (SipSubject*)SipMemCpMalloc(stSipMsg.hdlMemCp, sizeof(SipSubject));
    if (SS_NULL_PTR == pstSubject)
    {
        //ERROR_LOG("Create Subject Header Failed On Set Subject Header.");
        return RET_CODE_FAIL;
    }

    //����Subject
    long lResult = CreateSipStringData(stSipMsg.hdlMemCp, *pstSubject, strSubject.c_str());
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Create Subject Data Failed On Set Subject Header.");
        return RET_CODE_FAIL;
    }

    //�����չͷ��
    lResult = AddExHeader(stSipMsg, EX_HDR_ID_SUBJECT, pstSubject);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Add Subject Header Failed On Set Subject Header.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}



/**
* Description:  GetSubject().   ��ȡSubjectͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  szSubject       Subject�����ַ�����������ַ���������
* @param  [in]  ulSubjectLen    Subject�����ַ������ȣ������ַ���������
* @return       long.       ������
*/
long SipStack::SIP::GetSubject
(
    SipMsgS&        stSipMsg,
    char*&          pSubject,
    unsigned long&  ulSubjectLen
)
{

    SipSubject* pstSubject = (SipSubject*)SipDsmGetHdrFromMsg(EX_HDR_ID_SUBJECT, &stSipMsg);
    if (VOS_NULL == pstSubject)
    {
        //ERROR_LOG("Get Subject Header Failed On Get Subject.");
        return RET_CODE_FAIL;
    }

    if (0 == pstSubject->ulLen)
    {
        //ERROR_LOG("Get Subject Failed. Subject Content Length is 0.");
        return RET_CODE_FAIL;
    }

	if(ULONG_MAX-pstSubject->ulLen > 1)
	{
		ulSubjectLen = pstSubject->ulLen + 1;
	}
	else
	{
		ERROR_LOG("pstSubject->ulLen is too large.");
		return RET_CODE_FAIL;
	}

    if (VOS_NULL == VOS_NEW(pSubject, ulSubjectLen))
    {
        //ERROR_LOG("Create Subject Buffer Failed On Get Subject.");
        return RET_CODE_FAIL;
    }
    memset(pSubject, 0, ulSubjectLen);

    strncpy(pSubject, pstSubject->pcData, ulSubjectLen - 1);

    return RET_CODE_OK;
}


/**
* Description:  GetReason().   ��ȡReasonͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  ulReason        ulReason ����text�Ĵ�����
* @return       long.       ������
*/
long SipStack::SIP::GetReason( SipMsgS& stSipMsg, unsigned long& ulReason )
{
    SipReasonListHdl* pstReasonListHdl = (SipReasonListHdl*)SipDsmGetHdrFromMsg(EX_HDR_ID_REASON, &stSipMsg);
    if (VOS_NULL == pstReasonListHdl)
    {
        //INFO_LOG("SipStack::SIP::GetReason Get Reason Header SipReasonListHdl Failed On Get Reason.");//��ȡ����Reason��������Ϊ�������ÿ�ζ������
        return RET_CODE_FAIL;
    }

    const char* pszText="text";
    SipString stPar;
    stPar.pcData = (char*)pszText;//lint !e1773
    stPar.ulLen = strlen(pszText);

    SipTokenGenParams  *pstReason;
    SS_UINT16 usCount = SS_NULL_UINT16;
    SS_INT iTemp =0;
    SipGenParNameValue  *pstNameValue = SS_NULL_PTR;
    
    usCount = (SS_UINT16)VppListGetCount(pstReasonListHdl);//lint !e838
    for (iTemp = 0; iTemp < usCount; iTemp++)//lint !e838
    {
        if (VPP_SUCCESS != VppListGetData(pstReasonListHdl, (SS_UINT16)iTemp,
            (SS_VOID**)&pstReason))
        {
            SIP_RETURN(SS_NULL_PTR)
        }
        else
        {
            pstNameValue = SipDsmGetGenericParamByName(pstReason->hdlGenericParamList, &stPar);  //����ͻ�ȡ���� Text,�����������
            if(NULL != pstNameValue)//�ҵ���
            {
                char* pText = NULL;
				unsigned int uiLen = pstNameValue->pstrGenParValue->ulLen;
                if(NULL == VOS_NEW(pText,uiLen))
                {
                    //ERROR_LOG("SipStack::SIP::GetReason new pText failed.");
                    return RET_CODE_FAIL;
                }

                memset(pText,0,uiLen);
                memcpy(pText,pstNameValue->pstrGenParValue->pcData+1,uiLen);
                ulReason = (unsigned long)strtol(pText,NULL,10);//10���ƣ�ת��Ϊ����
                VOS_DELETE(pText,uiLen);

                INFO_LOG("SipStack::SIP::GetReason Get Text Reason:%d",ulReason);//��ȡ����Reason��������Ϊ�������ÿ�ζ������

                return RET_CODE_OK; 
            }
        }
        
    }

    //ERROR_LOG("SipStack::SIP::GetReason get Reason text failed.");
    return RET_CODE_FAIL;

}

/**
* Description:  ResetVarHdr().  ���ÿɱ�ͷ��
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [in]  ulSequence      CSeqֵ
* @param  [in]  pszFromTag      From��Tag
* @param  [in]  pszToTag        To��Tag
* @return       long.       ������
*/
long SipStack::SIP::ResetVarHdr
(
    SipMsgS&        stSipMsg,   
    unsigned long   ulSequence,
    const std::string& strFromTag,
    const char*     pszToTag
)
{
    //Minisip��������ֵ
    EN_SIP_RESULT enResult = SIP_RET_SUCCESS;
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    //����ֵ
    long lResult = RET_CODE_OK;

    //CSeq
    stSipMsg.stHeaders.pstCseq->ulSequence = ulSequence;

    //From Tag
    if (strFromTag.empty())
    {
        enResult = SipTxnUtilGenTag(stSipMsg.hdlMemCp, &stSipMsg.stHeaders.pstFrom->strTag);
        if (SIP_RET_SUCCESS != enResult)
        {
            //ERROR_LOG("Create From Tag Failed On Reset Variable Header. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
            return RET_CODE_FAIL;
        }
    }
    else
    {
        lResult = CreateSipStringData(stSipMsg.hdlMemCp, stSipMsg.stHeaders.pstFrom->strTag, strFromTag.c_str());
        if (RET_CODE_OK != lResult)
        {
            //ERROR_LOG("Set From Tag Failed On Reset Variable Header.");
            return RET_CODE_FAIL;
        }
    }

    //To Tag
    if (VOS_NULL != pszToTag)
    {
        lResult = CreateSipStringData(stSipMsg.hdlMemCp, stSipMsg.stHeaders.pstTo->strTag, pszToTag);
        if (RET_CODE_OK != lResult)
        {
            //ERROR_LOG("Set To Tag Failed On Reset Variable Header.");
            return RET_CODE_FAIL;
        }
    }

    //Via
    SipVia* pstSipVia = NULL;
    nResult = VppListGetData(stSipMsg.stHeaders.hdlViaList, 0, (void**)&pstSipVia);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Get Via Header Failed On Reset Variable Header. ErrorCode = %d.", nResult);
        return RET_CODE_FAIL;
    }

    //Via Branch
    enResult = SipTxnUtilGenBranchId(stSipMsg.hdlMemCp, &pstSipVia->strBranch);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("Create Branch Failed On Reset Variable Header. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}//lint !e1764

/**
* Description:  SetMsgBody().   ������Ϣ��
* @param  [in]  stSipMsg                 Sip��Ϣ�ṹ������
* @param  [in]  szSubMediaType       ��ý������
* @param  [in]  ulMsgBodyLen        ��Ϣ�峤��
* @param  [in]  szMsgBody           ��Ϣ���ַ���
* @return       long.       ������
*/
long SipStack::SIP::SetMsgBody
(
    SipMsgS&        stSipMsg,
    const char*     szSubMediaType,
    unsigned long   ulMsgBodyLen,
    char*           szMsgBody
)
{
    //����Ϣ�壬ֱ�ӷ���
    if (VOS_NULL == szMsgBody || 0 == ulMsgBodyLen)
    {
        //ERROR_LOG("Message Body is Empty On Set Message Body.");
        return RET_CODE_PARA_INVALIDATE;
    }

    //��Ϣ�����
    if (MAX_LEN_UDP_PACKET < ulMsgBodyLen)
    {
        //ERROR_LOG("Message Body Length(%d) is Too Big On Set Message Body.", ulMsgBodyLen);
        return RET_CODE_FAIL;
    }

    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    //������Ϣ��
    nResult = VppStrBufCreate(stSipMsg.hdlMemCp, (VPP_UINT16)ulMsgBodyLen, szMsgBody, ulMsgBodyLen, &stSipMsg.hdlBody);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Create String Buffer Failed On Set Message Body. ErrorCode = %d.", nResult);
        return RET_CODE_FAIL;
    }

    //ContentLength
    SipContentLength* pstContentLength = (SipContentLength*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_CONTENT_LENGTH, &stSipMsg);
    if (VOS_NULL == pstContentLength)
    {
        pstContentLength = (SipContentLength*)SipDsmCreateHdrInMsg(SIP_BASIC_HDR_ID_CONTENT_LENGTH, &stSipMsg);
    }

    //���¸�ֵ
	if (VOS_NULL != pstContentLength)
	{
		*pstContentLength = ulMsgBodyLen;
	}

    //����Content-Typeͷ��
    long lResult = SetContentType(stSipMsg, szSubMediaType);
    if (RET_CODE_OK != lResult)
    {
        //ERROR_LOG("Set Content-Type Header Failed On Set Message Body.");
        return RET_CODE_FAIL;
    }

    return RET_CODE_OK;
}

/**
* Description:  GetPlatAuthInfo().   ��ȡƽ̨��Ȩ��Ϣ
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] stPaltAuthInfo  ��ȡ��ƽ̨��Ȩ��Ϣ
* @return       long.       ������
*/
long SipStack::SIP::GetPlatAuthInfo
(
    SipMsgS&        stSipMsg,
    PLAT_AUTH_INFO& stPaltAuthInfo
)
{
    SipWWWAuthenticateListHdl stWwwAuthListHdl = SipDsmGetHdrFromMsg(EX_HDR_ID_WWW_AUTHENTICATE, &stSipMsg);
    if (VOS_NULL == stWwwAuthListHdl)
    {
        ERROR_LOG("Get WWW-Authenticate Header Failed On Get Plat Authentication Info.");
        return RET_CODE_FAIL;
    }

    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    SipWWWAuthenticate* pstSipWwwAuth = NULL;
    
    nResult = VppListGetData(stWwwAuthListHdl, 0, (void**)&pstSipWwwAuth);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        ERROR_LOG("Get WWW-Authenticate Header Value On Get Plat Authentication Info. ErrorCode = %d.", nResult);
        return RET_CODE_FAIL;
    }

    //strNonce strOpaque strRealm strAlgorithm�ĸ��ֶζ������
    if (VOS_NULL == pstSipWwwAuth->pstrNonce)
    {
        ERROR_LOG("Get Plat Authentication Info Failed. Nonce is Empty.");
        return RET_CODE_FAIL;
    }

    //if (VOS_NULL == pstSipWwwAuth->pstrOpaque)
    //{
        //ERROR_LOG("Get Plat Authentication Info Failed. Opaque is Empty.";
        //return RET_CODE_FAIL;
    //}

    if (VOS_NULL == pstSipWwwAuth->pstrRealm)
    {
        ERROR_LOG("Get Plat Authentication Info Failed. Realm is Empty.");
        return RET_CODE_FAIL;
    }

    if (VOS_NULL == pstSipWwwAuth->pstrAlgorithm)
    {
		ERROR_LOG("Get Plat Authentication Info Failed. Algorithm is Empty.");
		return RET_CODE_FAIL;
    }

    //ȥ��˫����
    pstSipWwwAuth->pstrNonce->pcData[pstSipWwwAuth->pstrNonce->ulLen - 1]   = '\0';
	if (VOS_NULL != pstSipWwwAuth->pstrOpaque)
	{
		pstSipWwwAuth->pstrOpaque->pcData[pstSipWwwAuth->pstrOpaque->ulLen - 1] = '\0';
	}
    pstSipWwwAuth->pstrRealm->pcData[pstSipWwwAuth->pstrRealm->ulLen - 1]   = '\0';

    //����Ȩ��Ϣ�Ƿ��뱾�ص���ͬ
    if (   0 == strncmp(stPaltAuthInfo.strNonce.c_str(),  pstSipWwwAuth->pstrNonce->pcData + 1,  pstSipWwwAuth->pstrNonce->ulLen - 2)
        //&& 0 == strncmp(stPaltAuthInfo.strOpaque.c_str(), pstSipWwwAuth->pstrOpaque->pcData + 1, pstSipWwwAuth->pstrOpaque->ulLen - 2)
        && 0 == strncmp(stPaltAuthInfo.strRealm.c_str(),  pstSipWwwAuth->pstrRealm->pcData + 1,  pstSipWwwAuth->pstrRealm->ulLen - 2))
    {
        WARN_LOG("Get Plat Authentication Info Failed. Authentication Failed Because Challege Info is The Same.");
        return RET_CODE_OK;//�޸����ⵥ DTS2011070502986 ����ֹ��;���޸ģ���Ϊ�ô�������
    }

    //����buffer����󳤶�
    unsigned long ulBufLen = pstSipWwwAuth->pstrNonce->ulLen;
	if(VOS_NULL != pstSipWwwAuth->pstrOpaque)
	{
		ulBufLen = ulBufLen > pstSipWwwAuth->pstrOpaque->ulLen ? ulBufLen : pstSipWwwAuth->pstrOpaque->ulLen;
	}
    ulBufLen = ulBufLen > pstSipWwwAuth->pstrRealm->ulLen  ? ulBufLen : pstSipWwwAuth->pstrRealm->ulLen;
	if (VOS_NULL != pstSipWwwAuth->pstrQOPOptions)
	{
		ulBufLen = ulBufLen > pstSipWwwAuth->pstrQOPOptions->ulLen  ? ulBufLen : pstSipWwwAuth->pstrQOPOptions->ulLen;
	}    
    ulBufLen += 1;//���Ͻ������ĳ���

    char* pBuffer = VOS_NEW(pBuffer, ulBufLen);
    if (VOS_NULL == pBuffer)
    {
        ERROR_LOG("Create Buffer Failed On Get Plat Authentication Info.");
        return RET_CODE_FAIL;
    }

    //������ַ���������
    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstSipWwwAuth->pstrNonce->pcData + 1, pstSipWwwAuth->pstrNonce->ulLen - 2);
    stPaltAuthInfo.strNonce = pBuffer;
	if(VOS_NULL != pstSipWwwAuth->pstrOpaque)
	{
		memset(pBuffer, 0, ulBufLen);
		strncpy(pBuffer, pstSipWwwAuth->pstrOpaque->pcData + 1, pstSipWwwAuth->pstrOpaque->ulLen - 2);
		stPaltAuthInfo.strOpaque = pBuffer;
	}
	else
	{
		stPaltAuthInfo.strOpaque = "";
	}

    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstSipWwwAuth->pstrRealm->pcData + 1, pstSipWwwAuth->pstrRealm->ulLen - 2);
    stPaltAuthInfo.strRealm = pBuffer;

    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstSipWwwAuth->pstrAlgorithm->pcData, pstSipWwwAuth->pstrAlgorithm->ulLen);
    stPaltAuthInfo.stAlgorithm = pBuffer;

	//���Qop�ֶ�
	if(VOS_NULL == pstSipWwwAuth->pstrQOPOptions)
	{
		WARN_LOG("Get Plat Authentication Info Failed. QOPOptions is Empty.");
		stPaltAuthInfo.strQop = "";
	}
	else
	{
		memset(pBuffer, 0, ulBufLen);
		strncpy(pBuffer, pstSipWwwAuth->pstrQOPOptions->pcData, pstSipWwwAuth->pstrQOPOptions->ulLen);
		stPaltAuthInfo.strQop = pBuffer;	
	}

    //�ͷ�buffer
    VOS_DELETE(pBuffer, MULTI);

    return RET_CODE_OK;
}

/**
* Description:  GetRedirectInfo().  ��ȡ�ض�����Ϣ
* @param  [in]  stSipMsg            SIP��Ϣ����
* @param  [out] vectorServerInfo    ��������Ϣ����
* @return       long.       ������
*/
long SipStack::SIP::GetRedirectInfo
(
    SipMsgS&            stSipMsg,
    VECTOR_SERVER_INFO& vectorServerInfo
)
{
    //���ԭ����Ϣ
    vectorServerInfo.clear();
    
    SipContactListHdl stContactListHdl = SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_CONTACT_LIST, &stSipMsg);
    if (VOS_NULL == stContactListHdl)
    {
        //ERROR_LOG("Get Contact Header Failed On Get Redirect Info.");
        return RET_CODE_FAIL;
    }

    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    SipContact* pstSipContact = NULL;

    unsigned long ulContactNum = VppListGetCount(stContactListHdl);

    //��������Contact�б�
    for (unsigned long ulIndex = 0; ulIndex < ulContactNum; ++ulIndex)
    {
        nResult = VppListGetData(stContactListHdl, (VPP_UINT16)ulIndex, (void**)&pstSipContact);
        if (VPP_SUCCESS != nResult)
        {
            //ERROR_LOG("Get Contact Header Value On Get Redirect Info. ErrorCode = %d.", nResult);
            return RET_CODE_FAIL;
        }
        
        SS_UCHAR* pIp = pstSipContact->pstContact->stUri.uri.pstSipUri->stHostPort.stHost.uHostContent.ipv4;

        //���캯���г�ʼ��
        SERVER_INFO stServerInfo;
		(void)snprintf(stServerInfo.szServerIP,MAX_LEN_IP,"%d.%d.%d.%d", pIp[0], pIp[1], pIp[2], pIp[3]);
        stServerInfo.usServerPort = (unsigned short)pstSipContact->pstContact->stUri.uri.pstSipUri->stHostPort.iPort;

        vectorServerInfo.push_back(stServerInfo);
    }
    
    return RET_CODE_OK;
}

/**
* Description:  GetPlatAuthInfo().   ��ȡע�ᳬʱʱ��
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] ulExpires       ��ȡ��ע�ᳬʱʱ��
* @return       long.       ������
*/
long SipStack::SIP::GetExpires
(
    SipMsgS&        stSipMsg,
    unsigned long&  ulExpires
)
{
    SipExpires* stExpires = (SipExpires*)SipDsmGetHdrFromMsg(EX_HDR_ID_EXPIRES, &stSipMsg);
    if (VOS_NULL == stExpires)
    {
        //WARN_LOG("Get Expires Header Failed On Get Expires.");
        return RET_CODE_FAIL;
    }

    ulExpires = *stExpires;

    return RET_CODE_OK;
}

/**
* Description:  GetContact().   ��ȡ�豸�����ַ��Ϣ
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] devIP              ��ȡ�豸IP
* @param  [out] devPort          ��ȡ�豸Port
* @return       long.       ������
*/
long SipStack::SIP::GetContact
(
 SipMsgS&        stSipMsg,
 string&          devIP,
 unsigned short&  devPort
 )
{
	SipContactListHdl* pContactListl = (SipContactListHdl*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_CONTACT_LIST, &stSipMsg);
	if (VOS_NULL == pContactListl)
	{
		WARN_LOG("SipStack::SIP::GetContact Get Contact Header SipContactListHdl Failed On Get ContactList.");
		return RET_CODE_FAIL;
	}

	SipContact* pstContact;
	if (VPP_SUCCESS != VppListGetData(pContactListl, 0,(SS_VOID**)&pstContact))
	{
		SIP_RETURN(SS_NULL_PTR)
	}
	else
	{
		SipURI *pstSipUri = pstContact->pstContact->stUri.uri.pstSipUri;
		if (VOS_NULL == pstSipUri)
		{
			ERROR_LOG("Get Contact URI Info Failed. SIP URI is Empty.");
			return RET_CODE_FAIL;
		}

		SipHostPort& sipHostPort = pstSipUri->stHostPort;
		ACE_INET_Addr addr;
		devPort = (unsigned short&)sipHostPort.iPort;
		if (SIP_ADDR_TYPE_IPV4 == sipHostPort.stHost.enHostType)
		{
			SS_UINT8 pIP4 [SS_IPV4_ADDR_LEN];
			for (int i = 0;i<SS_IPV4_ADDR_LEN;i++)
			{
				pIP4[i] = sipHostPort.stHost.uHostContent.ipv4[(SS_IPV4_ADDR_LEN-1)-i];
			}
			addr.set_address((char*)pIP4, SS_IPV4_ADDR_LEN);
		}
		else if (SIP_ADDR_TYPE_IPV6 == sipHostPort.stHost.enHostType)
		{
			//SS_UINT8 pIP6[SS_IPV6_ADDR_LEN];
			//for (int i = 0; i<SS_IPV6_ADDR_LEN; i++)
			//{
			//	pIP6[i] = sipHostPort.stHost.uHostContent.ipv6[(SS_IPV6_ADDR_LEN-1)-i];
			//}
			addr.set_address((char*)sipHostPort.stHost.uHostContent.ipv6, SS_IPV6_ADDR_LEN);
		} 
		else
		{
			ERROR_LOG("Get Contact URI's IP Info Failed. SIP URI is Empty.");
			return RET_CODE_FAIL;
		}
		const char* chDevIP = addr.get_host_addr();
		if(NULL != chDevIP)
		{
			devIP = chDevIP;
			//INFO_LOG("SipStack::SIP::GetContact get Contact text scuessfully.");
			return RET_CODE_OK;
		}
		else
		{
			ERROR_LOG("get_host_addr Failed.");
			return RET_CODE_FAIL;
		}
	}
}

/**
* Description:  GetDate().   ��ȡDateʱ��
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] dateTime          ʱ��
* @return       long.       ������
*/
long SipStack::SIP::GetDate
(
    SipMsgS&        stSipMsg,
	string&       dateTime
)
{
     return RET_CODE_OK;
}//lint !e1764

/**
* Description:  GetCurrentTime().   ��ȡϵͳ��ǰʱ��
* @return       long.       ������
*/
string SipStack::SIP::GetCurrentTime()
{  
	time_t oCurrTime;
	struct tm *oTmCurrTime;

	ACE_OS::time( &oCurrTime );
	struct tm t_tmTmp;
	oTmCurrTime = ACE_OS::localtime_r( &oCurrTime,&t_tmTmp );

	// szCurrTime ���浱ǰʱ������� ��ʽ��:2005-06-07 15:36:41
	char szCurrTime[32] = "\0";//lint !e840
	ACE_OS::strftime( szCurrTime, 32,"%Y-%m-%dT%H:%M:%S", oTmCurrTime );

	return string(szCurrTime);
}

/**
* Description:  GetNextNonce().     ��ȡNextNonce
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] strNextNonce       ��ȡ��NextNonce
* @return       long.       ������
*/
long SipStack::SIP::GetNextNonce
(
    SipMsgS&    stSipMsg,
    string&     strNextNonce
)
{
    SipAuthenticationInfoListHdl stAuthInfoListHdl = (SipAuthenticationInfoListHdl*)SipDsmGetHdrFromMsg(EX_HDR_ID_AUTHENTICATION_INFO, &stSipMsg);
    if (VOS_NULL == stAuthInfoListHdl)
    {
        INFO_LOG("Get Authentication-Info Header Failed On Get Next Nonce.");
        return RET_CODE_FAIL;
    }

    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;

    SipAuthenticationInfoS* pstSipAuthInfo = NULL;
    
    nResult = VppListGetData(stAuthInfoListHdl, 0, (void**)&pstSipAuthInfo);//lint !e838
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("Get Authentication-Info Header Value On Get Next Nonce. ErrorCode = %d.", nResult);
        return RET_CODE_FAIL;
    }

    SipString* pstNextNonce = pstSipAuthInfo->pstrNextNonce;
    if (VOS_NULL == pstNextNonce)
    {
        INFO_LOG("Get Next Nonce Failed. Next Nonce is Empty.");
        return RET_CODE_FAIL;
    }

    if (0 == pstNextNonce->ulLen)
    {
        //ERROR_LOG("Get Next Nonce Failed. Next Nonce Length is 0.");
        return RET_CODE_FAIL;
    }

    //ȥ���������ţ��滻Ϊ������
    pstNextNonce->pcData[pstNextNonce->ulLen - 1] = '\0';

    //���NextNonce��ȥ��ǰ�������
    strNextNonce = pstNextNonce->pcData + 1;
    return RET_CODE_OK;
}

/**
* Description:  GetMsgBody().   ��ȡ��Ϣ������
* @param  [in]  stSipMsg        SIP��Ϣ����
* @param  [out] pMsgBody       ��Ϣ��ָ��
* @param  [out] ulMsgBodyLen       ��Ϣ�����ݳ���
* @return       long.       ������
*/
long SipStack::SIP::GetMsgBody
(
    SipMsgS&        stSipMsg,
    char*&          pMsgBody,
    unsigned long&  ulMsgBodyLen
)
{
    //��ȡ��Ϣ��
    SipBody stBody = SipDsmGetBody(&stSipMsg);
    unsigned int nBodyLen = VppStrBufGetLength(stBody);
    char* pBody = NULL;

	if (0 == nBodyLen)
    {
        ERROR_LOG("Get Message Body Failed. Message Body Length (%d)Must be Greater than 0.",nBodyLen);
        return RET_CODE_FAIL;
    }

    if (VOS_NULL == VOS_NEW(pBody, nBodyLen+1))
    {
        ERROR_LOG("Create Message Body Buffer(length = %d) Failed On Get Message Body.",nBodyLen);
        return RET_CODE_FAIL;
    }

    memset(pBody, 0, nBodyLen+1);
    VPP_UINT32 nResult = VppStrBufCopyDataOut(stBody, 0, pBody, &nBodyLen);
    if (VPP_SUCCESS != nResult)
    {
        ERROR_LOG("Copy Data Out Failed On Get Message Body.");
		VOS_DELETE(pBody,MULTI);
        return RET_CODE_FAIL;
    }

    //�������
    pMsgBody        = pBody;
    ulMsgBodyLen    = nBodyLen;

    return RET_CODE_OK;
}

/**
* Description:  CopyHeader().   ��SIP��Ϣ�п���ͷ��
* @param  [in]  usHeaderId      ͷ��ID
* @param  [in]  stInSipMsg      ������SIP��Ϣ����
* @param  [out] stOutSipMsg     �����SIP��Ϣ����
* @return       long.       ������
*/
long SipStack::SIP::CopyHeader
(
    unsigned short  usHeaderId,
    SipMsgS&        stInSipMsg,
    SipMsgS&        stOutSipMsg
)
{
    //Minisip��������ֵ
    EN_SIP_RESULT enResult = SIP_RET_SUCCESS;
    
    SS_VOID* pvHdr = SipDsmGetHdrFromMsg(usHeaderId, &stInSipMsg);
    if (VOS_NULL == pvHdr)
    {
        //ERROR_LOG("Get Header Value Failed On Copy Header.");
        return RET_CODE_FAIL;
    }

    enResult = SipDsmCopyHdrToMsg(usHeaderId, &stOutSipMsg, pvHdr);//lint !e838
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipDsmCopyHdrToMsg Message Failed On Copy Header. Error = %s.", 
            //STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }
        
    return RET_CODE_OK;
}

/**
* Description:  GetDialogInfo().  ��ȡ�Ի���Ϣ
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [out]  stDiagInfo     ����ĶԻ���Ϣ
* @param  [in]  bFromLocal      ��Ϣ�Ƿ����Ա���
* @return       long.       ������
*/
long SipStack::SIP::GetDialogInfo
(
    SipMsgS&        stSipMsg,
    DIALOG_INFO&    stDiagInfo,
    VOS_BOOLEAN     bFromLocal
)
{
    SipCSeq* pstCSeq = (SipCSeq*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_CSEQ, &stSipMsg);
    if (VOS_NULL == pstCSeq)
    {
        ERROR_LOG("dsm header - cseq struct is null");
        return RET_CODE_FAIL;
    }
    stDiagInfo.ulCSeq = pstCSeq->ulSequence; //����stDiagInfo�е�ulCSeq

    SipFrom* pstFrom = (SipFrom*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_FROM, &stSipMsg);
    if (VOS_NULL == pstFrom)
    {
        ERROR_LOG("dsm header - from struct is null");
        return RET_CODE_FAIL;
    }

    SipTo* pstTo = (SipTo*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_TO, &stSipMsg);
    if (VOS_NULL == pstTo)
    {
        ERROR_LOG("dsm header - to struct is null");
        return RET_CODE_FAIL;
    }

    SipCallID* pstCallId = (SipCallID*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_CALLID, &stSipMsg);
    if (VOS_NULL == pstCallId)
    {
        ERROR_LOG("dsm header - call id struct is null");
        return RET_CODE_FAIL;
    }

    //�����ֶζ������
    if (0 == pstFrom->strTag.ulLen || 0 == pstTo->strTag.ulLen || 0 == pstCallId->ulLen )
    {
		ERROR_LOG("dsm header - invalid tag or call id - From_Tag_Len=%d,To_Tag_Len=%d,Call_ID_Tag_Len=%d.",
			pstFrom->strTag.ulLen,
			pstTo->strTag.ulLen,
			pstCallId->ulLen);
        return RET_CODE_FAIL;
    }

    //����buffer����󳤶�
    unsigned long ulBufLen = pstFrom->strTag.ulLen;
    ulBufLen = ulBufLen > pstTo->strTag.ulLen ? ulBufLen : pstTo->strTag.ulLen;
    ulBufLen = ulBufLen > pstCallId->ulLen    ? ulBufLen : pstCallId->ulLen;

    //���Ͻ������ĳ���
    ulBufLen += 1;

    char* pBuffer = VOS_NEW(pBuffer, ulBufLen);
    if (VOS_NULL == pBuffer)
    {
        //ERROR_LOG("Create Buffer Failed On Get Dialog Info.");
        return RET_CODE_FAIL;
    }

    //Ĭ�����Ա���
    string* pstrFromTag = &stDiagInfo.strLocalTag;
    string* pstrToTag   = &stDiagInfo.strRemoteTag;

    //����Զ�ˣ��򽻻�Local��Remote��Tag
    if (!bFromLocal)
    {
        pstrFromTag = &stDiagInfo.strRemoteTag;
        pstrToTag   = &stDiagInfo.strLocalTag;
    }

    //������ַ���������
    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstFrom->strTag.pcData, pstFrom->strTag.ulLen);
    *pstrFromTag = pBuffer;

    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstTo->strTag.pcData, pstTo->strTag.ulLen);
    *pstrToTag = pBuffer;

    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstCallId->pcData, pstCallId->ulLen);
    stDiagInfo.strCallId = pBuffer;

    //�ͷ�buffer
    VOS_DELETE(pBuffer, MULTI);

    return RET_CODE_OK;
}

/**
* Description:  GetFromTag().  ��ȡFrom��Tag
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [out]  strFromTag     �����From��Tag
* @return       long.       ������
*/
long SipStack::SIP::GetFromTag
(
    SipMsgS&    stSipMsg,
    string&     strFromTag
)
{

    SipFrom* pstFrom = (SipFrom*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_FROM, &stSipMsg);
    if (VOS_NULL == pstFrom)
    {
        //ERROR_LOG("Get From Header Failed On Get From Tag.");
        return RET_CODE_FAIL;
    }

    if (0 == pstFrom->strTag.ulLen)
    {
        //ERROR_LOG("Get From Tag Failed. From Tag is Empty.");
        return RET_CODE_FAIL;
    }

    //����buffer����󳤶ȣ����Ͻ������ĳ���
    unsigned long ulBufLen = pstFrom->strTag.ulLen + 1;

    char* pBuffer = VOS_NEW(pBuffer, ulBufLen);
    if (VOS_NULL == pBuffer)
    {
        //ERROR_LOG("Create Buffer Failed On Get From Tag.");
        return RET_CODE_FAIL;
    }

    //������ַ���������
    memset(pBuffer, 0, ulBufLen);
    strncpy(pBuffer, pstFrom->strTag.pcData, pstFrom->strTag.ulLen);
    strFromTag = pBuffer;

    //�ͷ�buffer
    VOS_DELETE(pBuffer, MULTI);

    return RET_CODE_OK;
}

/**
* Description:  GetFromUriInfo().  ��ȡFrom��Uri��Ϣ
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [out]  stFromUri     �����From��Uri��Ϣ
* @return       long.       ������
*/
long SipStack::SIP::GetFromUriInfo
(
    SipMsgS&        stSipMsg,
    PEER_URI_INFO&  stFromUri
)
{
    SipFrom* pstFrom = (SipFrom*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_FROM, &stSipMsg);
    if (VOS_NULL == pstFrom)
    {
        ERROR_LOG("Get From Header Failed On Get From URI Info.");
        return RET_CODE_FAIL;
    }

    if (VOS_NULL == pstFrom->stUri.uri.pstSipUri)
    {
        ERROR_LOG("Get From URI Info Failed. SIP URI is Empty.");
        return RET_CODE_FAIL;
    }

    SipURI& stSipUri = *pstFrom->stUri.uri.pstSipUri;

    //����UserName
    if (VOS_NULL == stSipUri.pstrUserName)
    {
        ERROR_LOG("Get From URI Info Failed. SIP URI User Name is Empty.");
        return RET_CODE_FAIL;
    }

    //ȡ��С�ĳ���
    unsigned long ulUserNameLen = sizeof(stFromUri.szUriUserName) - 1;
    ulUserNameLen = ulUserNameLen > stSipUri.pstrUserName->ulLen ? stSipUri.pstrUserName->ulLen : ulUserNameLen;
    strncpy(stFromUri.szUriUserName, stSipUri.pstrUserName->pcData, ulUserNameLen);

	INFO_LOG("Get From URI Info Success. SIP URI User Name= %s",stFromUri.szUriUserName);
    return RET_CODE_OK;
}
/**
* Description:  GetFromValue().  ��ȡFrom��ֵ
* @param  [in]  stSipMsg        Sip��Ϣ�ṹ������
* @param  [out]  pszFromValue   �����From��Value������������ڴ�
* @return       long.       ������
*/
/*
long SipStack::SIP::GetFromValue
(
    SipMsgS&    stSipMsg,
    char*&      pszFromValue
)
{
    //VPP��������ֵ
    VPP_UINT32 nResult = VPP_SUCCESS;
    
    SipFrom* pstFrom = (SipFrom*)SipDsmGetHdrFromMsg(SIP_BASIC_HDR_ID_FROM, &stSipMsg);
    if (VOS_NULL == pstFrom)
    {
        //ERROR_LOG("Get From Header Failed On Get From Value.");
        return RET_CODE_FAIL;
    }
    
    SipStrBufHdl hdlHdrBuffer = NULL;
    nResult = VppStrBufCreate(NULL, MAX_LEN_REQUEST_URI, NULL, 0, &hdlHdrBuffer);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("VppStrBufCreate Failed On Get From Value. ErrorCode = 0x%04X.", nResult);
        return RET_CODE_FAIL;
    }

    EN_SIP_RESULT enResult = SipEncHeader(SIP_BASIC_HDR_ID_FROM, pstFrom, hdlHdrBuffer);
    if (SIP_RET_SUCCESS != enResult)
    {
        //ERROR_LOG("SipEncHeader Failed On Get From Value. Error = %s.", STR_ARR_EN_SIP_RESULT[enResult]);
        return RET_CODE_FAIL;
    }

    VPP_UINT32 nFromValueLen = VppStrBufGetLength(hdlHdrBuffer);
    if (0 == nFromValueLen)
    {
        //ERROR_LOG("VppStrBufGetLength Failed On Get From Value.");
        return RET_CODE_FAIL;
    }

    char* pFromValue = VOS_NEW(pFromValue, nFromValueLen + 1);
    memset(pFromValue, 0, nFromValueLen + 1);

    nResult = VppStrBufCopyDataOut(hdlHdrBuffer, 0, pFromValue, &nFromValueLen);
    if (VPP_SUCCESS != nResult)
    {
        //ERROR_LOG("VppStrBufCopyDataOut Failed On Get From Value. ErrorCode = 0x%04X.", nResult);
        return RET_CODE_FAIL;
    }

    VppStrBufDestroy(&hdlHdrBuffer);

    //���ָ��
    pszFromValue = pFromValue;

    //INFO_LOG("Get From Value Success. From Value: %s.", pszFromValue);
    return RET_CODE_OK;
}
*/

//}}//end namespace

//lint +e818
//lint +e438

