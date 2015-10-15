/********************************************************************
 filename    :    RtpFrameOrganizer.h
 author      :    shilei 00194833
 created     :    2013-02-21
 description :    ʵ��һ��PS��ת����ES���Ĵ�����
 copyright   :    Copyright (c) 2012-2016 Huawei Tech.Co.,Ltd
 history     :    2013-02-21 ��ʼ�汾
*********************************************************************/
#ifndef __RTPFRAMEORGANIZER_H__
#define __RTPFRAMEORGANIZER_H__

#include <deque>
#include <list>
using namespace std;

// ACEϵͳͨ��ͷ
#include "ace_header.h"
#include "eSDK_RtpPacket.h"

#define MAX_RTP_FRAME_CACHE_NUM     (3)                // ����������֡��
#define MAX_RTP_SEQ                 65535              // RTPseq���ֵ
#define RTP_FRAME_CACHE_SIZE        (1500 * 1024)      // һ֡���ݵ���󳤶�1.5M
#define MAX_PACKETS_PER_SEND        10
#define MIN_TIME_INTERVAL           5
#define MAX_LEFT_PACKET_COUNT       200
#define SELECT_TIME_INTERVAL        (8 * 1000)
#define MAX_AUDIO_LENGTH            (10 * 1024)
#define MAX_RTP_PACKET_COUNT        (1024)
#define MAX_RTP_QUEUE_NUM           (MAX_RTP_PACKET_COUNT * MAX_RTP_FRAME_CACHE_NUM)

typedef std::list<ACE_Message_Block*>       RTP_FRAME_LIST_T;
typedef RTP_FRAME_LIST_T::iterator            RTP_FRAME_LIST_T_ITER;

typedef struct _stRTP_PACK_INFO_S
{
    unsigned short      usSeq;
    unsigned int        unTimestamp;
    bool                bMarker;
    ACE_Message_Block*  pRtpMsgBlock;
}RTP_PACK_INFO_S;
typedef std::deque<RTP_PACK_INFO_S>         RTP_PACK_QUEUE;

// ý�����ص�
typedef  void (*fafterPacketProtockChangeDataCallBack)(void  *pUser,ACE_Message_Block *pRtpBlock,ST_FRAME_DATA* pFreameData);

// RTP��������
class CPacketChangeProcessor
{
private:
    CPacketChangeProcessor(const CPacketChangeProcessor&);
    CPacketChangeProcessor& operator=(const CPacketChangeProcessor&);
public:
    CPacketChangeProcessor():m_ucAudioEncodeMode(0),
		 m_bAppendExtInfo(false),
		 m_fDataCallBack(NULL),
		 m_pUser(NULL),
		 m_Channel(0),
		 m_CallBack(NULL)
    {}
    virtual ~CPacketChangeProcessor()
	{
		m_ucAudioEncodeMode = 0;
		m_bAppendExtInfo = false;
		m_fDataCallBack = NULL;
		m_pUser = NULL;
		m_Channel = 0;
		m_CallBack = NULL;
	}

    // ���ͽӿ�
    virtual int init(std::string strServiceID) = 0;
    virtual void receviePacket(const ACE_Message_Block* pMb)=0;
	virtual int  setIsNeedAudioFrame(bool isNeedAudioFrame)=0;
    void setDataCallBack(fafterPacketProtockChangeDataCallBack fDataCallBack, void *pUser)
    {
        m_fDataCallBack =  fDataCallBack;
        m_pUser = pUser;
    }

    void setAudioEncodeMode(unsigned char ucAudioEncodeMode);
    void setAppednExtInfo(bool bAppendExtInfo){ m_bAppendExtInfo = bAppendExtInfo; };
	void SetChannel(unsigned int channelID) {m_Channel = channelID;}
	unsigned int GetChannel(void)const  {return m_Channel;}
	void SetFrameDataCallBack(FrameDataCallBack pCallBack) {m_CallBack = pCallBack;}
	const FrameDataCallBack GetFrameDataCallBack(void)const  {return m_CallBack;}
protected:
    unsigned char           m_ucAudioEncodeMode;           // ��Ƶ�����ʽ
    bool                    m_bAppendExtInfo;              // �ص�����֮ǰ����Ӹ�����Ϣ

    fafterPacketProtockChangeDataCallBack m_fDataCallBack; //һ֡���ݴ�����ص�����
    void *m_pUser; //���ݴ������
private:
	unsigned int m_Channel;
	FrameDataCallBack m_CallBack; 

};

// ����һ֡���ݽӿ�
class IRtpFrameHandler
{
public:
    IRtpFrameHandler(){}

    virtual ~IRtpFrameHandler(){}

    /*****************************************************************************
     Function:    handleMediaFrame
     Description: ƴ��һ֡���ݺ���ý������
     Input:       rtpFrameList ES���ݰ��б�
     Output:      
     Return:      N/A
    *****************************************************************************/
    virtual void handleMediaFrame(RTP_FRAME_LIST_T &rtpFrameList) = 0;

};

// ��֡��
class CRtpFrameOrganizer
{
public:
    CRtpFrameOrganizer();
    virtual ~CRtpFrameOrganizer();

    int init(IRtpFrameHandler* pHandler, const std::string& strServiceID, unsigned int unMaxFrameCache = MAX_RTP_FRAME_CACHE_NUM);

    int insertRtpPacket(const ACE_Message_Block* pRtpBlock);

    bool checkLostPacket()const;

    void release();

    int startThread();

    //���÷����߳�����״̬
    void SetSendMediaFunFlag(const bool bRunFlag);

    void acquireSendThreadSemaphore();

private:
    int insert(const RTP_PACK_INFO_S &info);

    int insertRange(const RTP_PACK_INFO_S &info, unsigned int unStartPos, unsigned int unStopPos);

    void checkFrame();

    void handleFinishedFrame(unsigned int unEndPos);

    void releaseRtpPacket(unsigned int unEndPos);
private:
    unsigned int            m_unMaxCacheFrameNum;       // ��󻺳�֡��(����������̫��׼��)
    IRtpFrameHandler*       m_pRtpFrameHandler;         // RTP֡�ص�����ӿ�

    RTP_PACK_QUEUE          m_RtpQueue;
    RTP_PACK_QUEUE          m_RtpAudioQueue;
    unsigned int            m_unCurFrameNum;            // �ѻ����֡��
    unsigned int            m_unLastDequeueSeq;         // ���һ�γ����е�RTP�����
    bool                    m_bSeqReversal;             // ���кŷ�ת��ʶ
    unsigned int            m_unSeqReversalPos;         // ���кŷ�����ת��λ��
    std::string             m_strServiceID;
    int                     m_iFrameNumber;
    bool                    m_bIsLostPacket;

    bool    m_bRunFlag;
    bool    m_bThreadExit;

    ACE_Thread_Semaphore * m_pSendThreadSemaphore;     //PSתES�߳��ź���

    int                    m_iRtpQueueLogThreshold;
};

#endif /* __RTPFRAMEORGANIZER_H__ */

