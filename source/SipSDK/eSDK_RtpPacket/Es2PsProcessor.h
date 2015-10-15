/********************************************************************
 filename    :    Es2PsProcessor.h
 author      :    shilei 00194833
 created     :    2013-02-22
 description :    ʵ��һ��ES��ת����PS���Ĵ�����
 copyright   :    Copyright (c) 2012-2016 Huawei Tech.Co.,Ltd
 history     :    2013-02-22 ��ʼ�汾
*********************************************************************/

#ifndef _ES_TO_PS_PROCESSOR_H_
#define _ES_TO_PS_PROCESSOR_H_

#include <string>

#include "MediaBlockBuffer.h"
#include "RtpFrameOrganizer.h"
//#include "PsRtpEncap.h"
//CPsRtpEncapHandle
class CEs2PsProcessor: public CPacketChangeProcessor, public IRtpFrameHandler
{
public:
    CEs2PsProcessor();

    virtual ~CEs2PsProcessor();

    int init(std::string strServiceID);
    
	int setIsNeedAudioFrame(bool isNeedAudioFrame);
    /*****************************************************************************
     Function:    receviePacket
     Description: ����һ��ESý������
     Input:       pMb ý�������ݿ�
     Output:      
     Return:      N/A
    *****************************************************************************/
    void receviePacket (const ACE_Message_Block* pMb);

    /*****************************************************************************
     Function:    handleMediaFrame
     Description: ƴ��һ֡ES���ݺ���ý������
     Input:       rtpFrameList ES���ݰ��б�
     Output:      
     Return:      N/A
    *****************************************************************************/
    void handleMediaFrame(RTP_FRAME_LIST_T &rtpFrameList);


    void SetFrameOrganiaerSendMediaFunFlag(const bool bFunFlag);
    void acquireFrameOrganiaerSendThreadSemaphore();

private:
    
    /*****************************************************************************
     Function:    handleAudioRtpFrame
     Description: ƴ��һ֡ES���ݺ���ý������
     Input:       rtpFrameList ES���ݰ��б���Ƶ
     Output:      
     Return:      N/A
    *****************************************************************************/
    void handleAudioRtpFrame(RTP_FRAME_LIST_T &rtpFrameList);

    /*****************************************************************************
     Function:    handleVideoRtpFrame
     Description: ƴ��һ֡ES���ݺ���ý������
     Input:       rtpFrameList ES���ݰ��б���Ƶ
     Output:      
     Return:      N/A
    *****************************************************************************/
    void handleVideoRtpFrame(RTP_FRAME_LIST_T &rtpFrameList);

    /*****************************************************************************
     Function:    parseSingleNalu
     Description: ������Nalu
     Input:       rtpFrameList ES���ݰ��б���Ƶ
     Output:      
     Return:      IVS_SUCCEED �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    int parseSingleNalu(RTP_FRAME_LIST_T &rtpFrameList);

    /*****************************************************************************
     Function:    sendPsRtpPacket
     Description: ����һ��ESý������
     Input:       pRtpData  ������PS�����һ������
                  unDataSize PS����һ�����ݵĳ���
     Output:      
     Return:      N/A
    *****************************************************************************/
    void sendPsRtpPacket(const char* pRtpData, unsigned int unDataSize,
            void* pUserData);

    /*****************************************************************************
     Function:    handleSingleNalu
     Description: ������Nalu
     Input:       pRtpBlock ���ݿ�
                  rtpPacket rtp����Ϣ
                  unCacheSize �����ʣ��ռ�
     Output:      
     Return:      
    *****************************************************************************/
    void handleSingleNalu(ACE_Message_Block* pRtpBlock,const CRtpPacket &rtpPacket,
            unsigned int &unCacheSize);

    void ReleasePktList();
private:    
    std::string             m_strServiceID;         // ҵ��ID����Ҫ���ڴ�ӡ��־

    CRtpFrameOrganizer      m_RtpFrameOrganizer;
    //CPsRtpEncap             m_PsRtpEncap;

    ACE_Recursive_Thread_Mutex       m_listMutex;             //RTSP�ỰMAP����
    std::list <ACE_Message_Block*>                  m_curPacketList;
    char*                   m_pRtpFrameCache;       // �洢һ֡���ݵĻ��棬ע���ڴ��ͷ�
    char*                   m_pWritePos;            // ����m_pRtpFrameCache�ڴ�ָ��

    ACE_Message_Block*      m_pExtendHeader;                
    suseconds_t             m_usecLastSendTime;
    time_t                  m_tmLastSendTime;
    std::string             m_strSessionID;

	bool                    m_bIsNeedAudioFrame;   // �Ƿ������Ƶ֡
	char					m_VideoStreamType;

};

#endif //_ES_TO_PS_PROCESSOR_H_
