/********************************************************************
 filename    :    Es2PsProcessor.cpp
 author      :    shilei 00194833
 created     :    2013-02-22
 description :    ʵ��һ��ES��ת����PS���Ĵ�����
 copyright   :    Copyright (c) 2012-2016 Huawei Tech.Co.,Ltd
 history     :    2013-02-22 ��ʼ�汾
*********************************************************************/
#include "stdafx.h"
#include "ace_header.h"
// ������ͷ�ļ�
#include "ivs_error.h"
#include "NssMedia.h"
#include "RtpPacket.h"
#include "Es2PsProcessor.h"
#include "Log.h"
#include "eSDKTool.h"
#include "eSDKMem.h"
//lint -e438
CEs2PsProcessor::CEs2PsProcessor()
{
    m_strServiceID = "";
    m_pRtpFrameCache    = NULL;
    m_pWritePos         = NULL;
    m_pExtendHeader = NULL;	
    m_curPacketList.clear();
    m_usecLastSendTime = 0;
    m_tmLastSendTime = 0;
	m_VideoStreamType = IVS_PT_H264;
	m_bIsNeedAudioFrame = false;
}

CEs2PsProcessor::~CEs2PsProcessor()
{
	try
	{	
		HW_DELETE_A(m_pRtpFrameCache);
	}
	catch(...)
	{	//lint !e1775
		m_pRtpFrameCache = NULL;
	}
		
	m_pWritePos      = NULL;
		
	if(NULL != m_pExtendHeader)
	{
		m_pExtendHeader->release();
		m_pExtendHeader = NULL;
	}
	
	ReleasePktList();
}

void CEs2PsProcessor::ReleasePktList()
{
    ACE_Message_Block* pMb = NULL;

    ACE_GUARD(ACE_Recursive_Thread_Mutex, pObj, m_listMutex);
    // �ͷŵ�ǰ֡�ڵ��ڴ�
    while (!m_curPacketList.empty())
    {
        pMb = m_curPacketList.front();
        CMediaBlockBuffer::instance().freeMediaBlock(pMb);
        m_curPacketList.pop_front();
    }

    return;
}

int CEs2PsProcessor::init(std::string strServiceID)
{
    try
    {
        m_pExtendHeader   = new ACE_Message_Block(2048);
    }
    catch(...)
    {
        m_pExtendHeader   = NULL;
    }
	if(NULL == m_pExtendHeader)
	{
		return IVS_PARA_INVALID;
	}
    m_pExtendHeader->reset();

	// ����Ҫʹ��EsתPs�������θù���
    //if (IVS_SUCCEED != m_PsRtpEncap.init(this, NULL))
    //{
        //BP_RUN_LOG_ERR(IVS_FAIL, "Ps rtp encap fail", "");
    //    return IVS_FAIL;
    //}

    if (IVS_SUCCEED != m_RtpFrameOrganizer.init((IRtpFrameHandler*)this, strServiceID.c_str()))
    {
        ERROR_LOG("Rtp frame organizer fail");
        return IVS_FAIL;
    }

    HW_NEW_A(m_pRtpFrameCache, char, RTP_FRAME_CACHE_SIZE);  //lint !e774 !e831
    //CHECK_POINTER(m_pRtpFrameCache, IVS_ALLOC_MEMORY_ERROR);
    m_pWritePos = m_pRtpFrameCache;

    m_strServiceID = strServiceID;
    //BP_RUN_LOG_INF("Init ES to PS processor success", "serviceID=%s.",  m_strServiceID.c_str());
    return IVS_SUCCEED;
}

/*****************************************************************************
 Function:    receviePacket
 Description: ����һ��ESý������
 Input:       pMb ý�������ݿ�
 Output:      
 Return:      N/A
*****************************************************************************/
void CEs2PsProcessor::receviePacket (const ACE_Message_Block* pMb)
{
    //CHECK_POINTER_VOID(pMb);

    if (IVS_SUCCEED != m_RtpFrameOrganizer.insertRtpPacket(pMb))
    {
        ERROR_LOG("Insert rtp packet to frame organizer fail");
        return;
    }

    return;
}

void CEs2PsProcessor::SetFrameOrganiaerSendMediaFunFlag(const bool bFunFlag)
{
    m_RtpFrameOrganizer.SetSendMediaFunFlag(bFunFlag);
}
void CEs2PsProcessor::acquireFrameOrganiaerSendThreadSemaphore()
{
    m_RtpFrameOrganizer.acquireSendThreadSemaphore();
}

/*****************************************************************************
 Function:    sendPsRtpPacket
 Description: ����һ��ESý������
 Input:       pRtpData  ������PS�����һ������
              unDataSize PS����һ�����ݵĳ���
 Output:      
 Return:      N/A
*****************************************************************************/
void CEs2PsProcessor::sendPsRtpPacket(const char* pRtpData, unsigned int unDataSize,
        void* /*pUserData*/)
{
    //CHECK_POINTER_VOID(pRtpData);

    if (0 == unDataSize)
    {
        //BP_RUN_LOG_ERR(IVS_FAIL, "Send ps packet fail",
            //"serviceID=%s.", m_strServiceID.c_str());
        return;
    }

    ACE_Message_Block *pRtpBlock = CMediaBlockBuffer::instance().allocMediaBlock();
    if (NULL == pRtpBlock)
    {
        //BP_RUN_LOG_ERR(IVS_ALLOC_MEMORY_ERROR, "Alloc media block fail.",
            //"serviceID=%s.", m_strServiceID.c_str());
        return;
    }

    pRtpBlock->reset();
    pRtpBlock->copy(pRtpData, unDataSize);
	
	m_curPacketList.push_back(pRtpBlock);
    return;
}

/*****************************************************************************
 Function:    handleMediaFrame
 Description: ƴ��һ֡ES���ݺ���ý������
 Input:       rtpFrameList ES���ݰ��б�
 Output:      
 Return:      N/A
*****************************************************************************/
void CEs2PsProcessor::handleMediaFrame(RTP_FRAME_LIST_T &rtpFrameList)
{
    if (rtpFrameList.empty())
    {
        ERROR_LOG("Handle ES frame fail, frame list is empty");
        return;
    }

    // �µ�һ֡�������Ӧ���ǿյ�
    if (m_pWritePos != m_pRtpFrameCache)
    {
        m_pWritePos = m_pRtpFrameCache;
        //BP_RUN_LOG_INF("Handle ES media frame abnormal",
            //"Write postion not in cache head, serviceID=%s.", m_strServiceID.c_str());
    }

    CRtpPacket rtpPacket;
    ACE_Message_Block* pRtpBlock = rtpFrameList.front();
    (void)rtpPacket.ParsePacket(pRtpBlock->rd_ptr(), pRtpBlock->length());
    char cPt = rtpPacket.GetPayloadType();
    //�ж��Ƿ�Ϊ��Ƶ��Ƶ
    if ((cPt == IVS_PT_G711U) || (cPt == IVS_PT_G711A)
        || (cPt == IVS_PT_G723) || (cPt == IVS_PT_G722)||(cPt == ELTE_PT_AMR))
    {
        handleAudioRtpFrame(rtpFrameList);
    }
    else
    {		
		if(IVS_PT_H264 == cPt)
		{
			m_VideoStreamType = IVS_PT_H264;
		}
		else if(IVS_PT_MPEG4 == cPt)
		{
			m_VideoStreamType = IVS_PT_MPEG4;
		}
        handleVideoRtpFrame(rtpFrameList);
    }

    return;
}

/*****************************************************************************
 Function:    handleAudioRtpFrame
 Description: ƴ��һ֡ES���ݺ���ý������
 Input:       rtpFrameList ES���ݰ��б���Ƶ
 Output:      
 Return:      N/A
*****************************************************************************/
void CEs2PsProcessor::handleAudioRtpFrame(RTP_FRAME_LIST_T &rtpFrameList)
{
	if(NULL == m_pExtendHeader)
	{
		return;
	}

    m_pExtendHeader->reset();

    ACE_Message_Block* pRtpBlock = NULL;
    CRtpPacket rtpPacket;
    unsigned int unCacheSize = RTP_FRAME_CACHE_SIZE;
    bool bFirst = true;
    for (RTP_FRAME_LIST_T_ITER iter = rtpFrameList.begin(); iter != rtpFrameList.end(); iter++)
    {
        pRtpBlock = *iter;
        (void)rtpPacket.ParsePacket(pRtpBlock->rd_ptr(), pRtpBlock->length());


        // �Ƴ�RTP��Ϣͷ
        pRtpBlock->rd_ptr(rtpPacket.GetHeadLen());

        if(bFirst && (1 == rtpPacket.GetExtension()))
        {
            if(NULL != rtpPacket.GetMuExtData())
            {
                m_pExtendHeader->copy((char *)rtpPacket.GetMuExtData(), sizeof(RTP_EXTENSION_DATA_MU_S));
            }
            else if(NULL != rtpPacket.GetExtData())
            {
                m_pExtendHeader->copy((char *)rtpPacket.GetExtData(), sizeof(RTP_EXTENSION_DATA_S));
            }
            else
            {
                //BP_RUN_LOG_ERR(IVS_FAIL, "Error extension label", "");
            }
            bFirst = false;
        }

        if (unCacheSize >= pRtpBlock->length())
        {
            memcpy(m_pWritePos, pRtpBlock->rd_ptr(), pRtpBlock->length());
            m_pWritePos += pRtpBlock->length();
            unCacheSize -= pRtpBlock->length();
        }
        else
        {
            // ���������Ȳ���
            //BP_RUN_LOG_ERR(IVS_FAIL, "Current frame is too big exceed cache size 1.5M",
                                     //"serviceID=%s rtp package list size=%d curr rtp package length=%d.",
                                     //m_strServiceID.c_str(), rtpFrameList.size(), rtpPacket.GetPacketLen());
            break;
        }
    }

    //m_PsRtpEncap.setExtendHeader(m_pExtendHeader);
    //(void)m_PsRtpEncap.insertAudioFrame(m_pRtpFrameCache, (unsigned int)(m_pWritePos - m_pRtpFrameCache),
        //rtpPacket.GetTimeStamp());

    // ���û�����дλ��
    m_pWritePos = m_pRtpFrameCache;
    return ;
}

/*****************************************************************************
 Function:    handleVideoRtpFrame
 Description: ƴ��һ֡ES���ݺ���ý������
 Input:       rtpFrameList ES���ݰ��б���Ƶ
 Output:      
 Return:      N/A
*****************************************************************************/
void CEs2PsProcessor::handleVideoRtpFrame(RTP_FRAME_LIST_T &rtpFrameList)
{
	if(NULL == m_pExtendHeader)
	{
		return;
	}
    m_pExtendHeader->reset();
    int iRet = parseSingleNalu(rtpFrameList);
    if (IVS_SUCCEED != iRet)
    {
        ERROR_LOG("Handle sps�� pps�� or sei fail");
        return;
    }

    // ���ڵ���
    if (rtpFrameList.empty())
    {
        return ;
    }

    ACE_Message_Block* pRtpBlock = NULL;
    CRtpPacket rtpPacket;
    unsigned int unCacheSize = RTP_FRAME_CACHE_SIZE - H264_START_CODE_LENGTH;

    *m_pWritePos++ = 0x0; //lint !e613
    *m_pWritePos++ = 0x0;
    *m_pWritePos++ = 0x0;
    *m_pWritePos++ = 0x1;

    bool  bRemoveHead = true;
    bool  bCheckFrame = false;
    bool  bFuStart = false;
    bool  bFuStop  = false;
    bool bFirst = true;
    for (RTP_FRAME_LIST_T_ITER iter = rtpFrameList.begin(); iter != rtpFrameList.end(); iter++)
    {
        pRtpBlock = *iter;
        (void)rtpPacket.ParsePacket(pRtpBlock->rd_ptr(), pRtpBlock->length());

        // �Ƴ�RTP��Ϣͷ
        pRtpBlock->rd_ptr(rtpPacket.GetHeadLen());

        if(bFirst && (1 == rtpPacket.GetExtension()))
        {
            if(NULL != rtpPacket.GetMuExtData())
            {
                m_pExtendHeader->copy((char *)rtpPacket.GetMuExtData(), sizeof(RTP_EXTENSION_DATA_MU_S));
            }
            else if(NULL != rtpPacket.GetExtData())
            {
                m_pExtendHeader->copy((char *)rtpPacket.GetExtData(), sizeof(RTP_EXTENSION_DATA_S));
            }
            else
            {
                ERROR_LOG("Error extension label");
            }
            bFirst = false;
        }

        FU_INDICATOR* pFu = (FU_INDICATOR*)(void*)pRtpBlock->rd_ptr();
        // ���˵�������֡��ֻ֧��FUA�ְ���ʽ
        if (H264_NALU_TYPE_FU_A == (long long)(pFu->TYPE))
        {
            pRtpBlock->rd_ptr(sizeof(FU_INDICATOR));

            bCheckFrame = true;
            FU_HEADER *pFuHeader = (FU_HEADER*)(void*)pRtpBlock->rd_ptr();
            if (pFuHeader->S)
            {
                bFuStart    = true;
            }

            if (pFuHeader->E)
            {
                bFuStop     = true;
            }

            if (bRemoveHead)
            {
                NALU_HEADER* pHead = (NALU_HEADER*) (void*) pRtpBlock->rd_ptr();
                NALU_HEADER* pWriteHead = (NALU_HEADER*) (void*)m_pWritePos;
                pWriteHead->F = pFu->F;
                pWriteHead->NRI = pFu->NRI;
                pWriteHead->TYPE = pHead->TYPE;

                pRtpBlock->rd_ptr(sizeof(NALU_HEADER));
                m_pWritePos += sizeof(NALU_HEADER);
                unCacheSize--;

                bRemoveHead = false;
            }
            else
            {
                pRtpBlock->rd_ptr(sizeof(FU_HEADER));
            }
        }

        // ����������Ϣ
        if ((H264_NALU_TYPE_FU_A != (long long)(pFu->TYPE))
            && (H264_NALU_TYPE_SEI != (long long)(pFu->TYPE))
            && (H264_NALU_TYPE_SPS != (long long)(pFu->TYPE))
            && (H264_NALU_TYPE_PPS != (long long)(pFu->TYPE)))

        {
            continue;
        }

        if (unCacheSize >= pRtpBlock->length())
        {
            memcpy(m_pWritePos, pRtpBlock->rd_ptr(), pRtpBlock->length());
            m_pWritePos += pRtpBlock->length();
            unCacheSize -= pRtpBlock->length();
        }
        else
        {
            // ���������Ȳ���
            ERROR_LOG("Current frame is too big exceed cache size 1.5M ,rtp package list size : %d. curr rtp package length: %d",rtpFrameList.size(),rtpPacket.GetPacketLen());
            break;
        }

		//����������֡������ż����֡�����ܻ���������Ƶ�����
		if (bFuStop)
		{
			RTP_FRAME_LIST_T_ITER tmpiter = iter;
			if ((++tmpiter) != rtpFrameList.end())
			{
				*m_pWritePos++ = 0x0;
				*m_pWritePos++ = 0x0;
				*m_pWritePos++ = 0x0;
				*m_pWritePos++ = 0x1;
				unCacheSize -= H264_START_CODE_LENGTH;
				bRemoveHead = true;
				bFuStop = false;
			}
		}
    }

    // ���֡�Ƿ�����
    if (bCheckFrame && (!bFuStart || !bFuStop) )
    {
        // ���������Ȳ���
        ERROR_LOG("Current frame is too big exceed cache size 1.5M,will discard part data");
    }
	ACE_Message_Block *pMbF = new ACE_Message_Block(m_pRtpFrameCache,(unsigned int)(m_pWritePos - m_pRtpFrameCache),0);
	pMbF->length((unsigned int)(m_pWritePos - m_pRtpFrameCache));
	pMbF->rd_ptr(4);
	NALU_HEADER naluheader = *(NALU_HEADER*)pMbF->rd_ptr();
	
	ST_FRAME_DATA framedata;
	framedata.iFrameDataType = naluheader.TYPE;	
	framedata.iStreamType = m_VideoStreamType;	
	// ��ɽ�����ֱ�ӻص�����
	if(NULL != m_fDataCallBack)
	{
		m_fDataCallBack(m_pUser, pMbF,&framedata);
	}
	pMbF->release();
	
	// ����ʹ��EsתPsת��  
    //m_PsRtpEncap.setExtendHeader(m_pExtendHeader);
    //(void)m_PsRtpEncap.insertVideoFrame(m_pRtpFrameCache,
        //(unsigned int)(m_pWritePos - m_pRtpFrameCache),
        //rtpPacket.GetTimeStamp());

    // ���û�����дλ��
    m_pWritePos = m_pRtpFrameCache;
    return;
}

/*****************************************************************************
 Function:    parseSingleNalu
 Description: ������Nalu
 Input:       rtpFrameList ES���ݰ��б���Ƶ
 Output:      
 Return:      IVS_SUCCEED �ɹ�
              ���� ʧ��
*****************************************************************************/
int CEs2PsProcessor::parseSingleNalu(RTP_FRAME_LIST_T &rtpFrameList)
{
    ACE_Message_Block* pRtpBlock = NULL;
    CRtpPacket rtpPacket;
    unsigned int unCacheSize = RTP_FRAME_CACHE_SIZE - H264_START_CODE_LENGTH;

    while(!rtpFrameList.empty())
    {
        pRtpBlock = rtpFrameList.front();
        (void)rtpPacket.ParsePacket(pRtpBlock->rd_ptr(), pRtpBlock->length());
        FU_INDICATOR* pFu = (FU_INDICATOR*)(void*)(pRtpBlock->rd_ptr() + rtpPacket.GetHeadLen());
        
        // ����SPS��SEI��PPS�����ǵ���һ����ʱ��
        // SPS|PPS|SEI|IDR ���� SEI|P ��Щ����
        // ���ֻ��һ��ʱ�ڸ�IDR��P֡һ������
        if (((H264_NALU_TYPE_SEI == (long long)(pFu->TYPE))
            ||(H264_NALU_TYPE_SPS == (long long)(pFu->TYPE))
            ||(H264_NALU_TYPE_PPS == (long long)(pFu->TYPE)))
            &&(1 < rtpFrameList.size()))
        {            
            handleSingleNalu(pRtpBlock, rtpPacket, unCacheSize);
            rtpFrameList.pop_front();
            continue;
        }
        // ���⴦��Pֻ֡��һ���������
        else if ((H264_NALU_TYPE_SILCE == (long long)(pFu->TYPE))
            && (1 == rtpFrameList.size()))
        {
            handleSingleNalu(pRtpBlock, rtpPacket, unCacheSize);
            rtpFrameList.pop_front();
            continue;
        }
        // ��������ֱ�Ӷ���
        else if (((H264_NALU_TYPE_FU_A != (long long)(pFu->TYPE))
            && (H264_NALU_TYPE_SEI != (long long)(pFu->TYPE))
            && (H264_NALU_TYPE_SPS != (long long)(pFu->TYPE))
            && (H264_NALU_TYPE_PPS != (long long)(pFu->TYPE))))
        {
            rtpFrameList.pop_front();
            continue;
        }
        // �ҵ�SPS��SEI��PPS��IDR��P֡ͷֱ���˳������÷Ŵ���
        else
        {
            break;
        }
    }

    return IVS_SUCCEED;
}

/*****************************************************************************
 Function:    handleSingleNalu
 Description: ������Nalu
 Input:       pRtpBlock ���ݿ�
              rtpPacket rtp����Ϣ
              unCacheSize �����ʣ��ռ�
 Output:      
 Return:      
*****************************************************************************/
void CEs2PsProcessor::handleSingleNalu(ACE_Message_Block* pRtpBlock,const CRtpPacket &rtpPacket,
    unsigned int &unCacheSize)
{
	DEBUG_TRACE("");
    //CHECK_POINTER_VOID(pRtpBlock);
	if(NULL == m_pRtpFrameCache)
	{
		ERROR_LOG("m_pRtpFrameCache is NULL");
		return ;
	}
    
    // �Ƴ�RTP��Ϣͷ
    pRtpBlock->rd_ptr(rtpPacket.GetHeadLen());
    
    // ������ʼ��    
    *m_pWritePos++ = 0x0; //lint !e613
    *m_pWritePos++ = 0x0;
    *m_pWritePos++ = 0x0;
    *m_pWritePos++ = 0x1;
    
    if (unCacheSize >= pRtpBlock->length())
    {
        memcpy(m_pWritePos, pRtpBlock->rd_ptr(), pRtpBlock->length());
        m_pWritePos += pRtpBlock->length();
        
		unCacheSize -= pRtpBlock->length();
    }
    else
    {
        // ���������Ȳ���
        ERROR_LOG("Current frame is too big exceed cache size 1.5M,curr rtp package length:%d",rtpPacket.GetPacketLen());
    }

    //(void)m_PsRtpEncap.insertVideoFrame(m_pRtpFrameCache, (unsigned int)(m_pWritePos - m_pRtpFrameCache),
        //rtpPacket.GetTimeStamp());

	/*lint -save -e423*/
	ACE_Message_Block *pMbF = new ACE_Message_Block(m_pRtpFrameCache,(unsigned int)(m_pWritePos - m_pRtpFrameCache),0);
	pMbF->length((unsigned int)(m_pWritePos - m_pRtpFrameCache));
	pMbF->rd_ptr(4);
	NALU_HEADER naluheader = *(NALU_HEADER*)pMbF->rd_ptr();
	ST_FRAME_DATA framedata;
	framedata.iFrameDataType = naluheader.TYPE;
	framedata.iStreamType = m_VideoStreamType;
	if(NULL != m_fDataCallBack)
	{
		m_fDataCallBack(m_pUser, pMbF,&framedata);
	}	
	pMbF->release();
	pMbF = NULL;
	/*lint -restore*/
	 
    // ���û�����дλ��
    m_pWritePos = m_pRtpFrameCache;
    unCacheSize = RTP_FRAME_CACHE_SIZE - H264_START_CODE_LENGTH;
}//lint !e838


/*****************************************************************************
 Function:    setIsNeedAudioFrame
 Description: �����Ƿ񷵻���Ƶ֡����
              ����m_bIsNeedAudioFrameֵ
 Input:       isNeedAudioFrame �Ƿ���Ҫ������Ƶ֡���� true��ʾ��Ҫ����
 Output:      
 Return:      IVS_SUCCEED �ɹ�
*****************************************************************************/
int CEs2PsProcessor::setIsNeedAudioFrame(bool isNeedAudioFrame)
{
	m_bIsNeedAudioFrame = isNeedAudioFrame;
	return IVS_SUCCEED;
}

//lint +e438
