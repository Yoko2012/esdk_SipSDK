#ifndef __CLOCKFREEMEDIABUFFER_H__
#define __CLOCKFREEMEDIABUFFER_H__

#include "MediaDataQueue.h"
#include "Common_Def.h"

#define MEDIA_BLOCK_FREE_NUM_DLT 1000
#define  MEDIA_MAX_QUEUE_DELAY  (100*1000)

//�����ж�ʱ�����
#define MEDIA_BLOCK_BUFFER_CHECK_INTERVAL  (60)

class CBlockCheckTimer : public ACE_Event_Handler
{
public:
	virtual int handle_timeout(const ACE_Time_Value &tv, const void *arg);
};

class CMediaBlockBuffer
{
private:
	CMediaBlockBuffer(const CMediaBlockBuffer& src)
	{
		m_unSize    = 0;
		m_unCount   = 0;
		m_iLastFreeNum = 0;
		m_FailureTimes = 0;
		m_unAllocBuffFailTimes = 0;
		m_pMediaQueue  = NULL;
		m_lQueueTimerId = -1;
		m_ulQueueFreeFailNum = 0;
		m_pBlockCheckTimer = NULL;
		UNUSED_PARA(src);
	}

	CMediaBlockBuffer& operator=(const CMediaBlockBuffer& srcBuf)
	{
		if(&srcBuf == this)
		{
			return *this;
		}
		m_unSize    = 0;
		m_unCount   = 0;
		m_iLastFreeNum = 0;
		m_FailureTimes = 0;
		m_unAllocBuffFailTimes = 0;
		m_pMediaQueue  = NULL;
		m_lQueueTimerId = -1;
		m_ulQueueFreeFailNum = 0;
		m_pBlockCheckTimer = NULL;
		return *this;
	}

public:
	CMediaBlockBuffer();

	virtual ~CMediaBlockBuffer();

	static CMediaBlockBuffer& instance()
	{
		if (NULL == g_MediaBuffer)
		{
			try
			{
				g_MediaBuffer = new CMediaBlockBuffer;
			}
			catch(...)
			{
				static CMediaBlockBuffer buffer;
				g_MediaBuffer = &buffer;
			}
		}
		//if(NULL == g_MediaBuffer)
		//{
		//    IVS_RUN_LOG_ERR("create Buffer fail.");
		//    return NULL;
		//}

		return (*g_MediaBuffer);
	}

	int init(unsigned int unBlockSize, unsigned int unBlockCount);

	void close();

	ACE_Message_Block* allocMediaBlock();

	void freeMediaBlock(ACE_Message_Block *&pMb);

	// ��ȡϵͳ�ܵĻ�������
	unsigned int getMaxBlockNum() const;

	// ��ȡʣ�໺������
	unsigned int getRemainBlockNum() const;

	// ��ȡ�ڴ���С
	unsigned int getBlockSize()const;

	unsigned int getFailureTimes()const;

	unsigned long getMediaBlockFailNum()const
	{
		return m_ulQueueFreeFailNum;
	}

	void setMediaBlockFailNum(unsigned long ulQueueFreeFailNum)
	{
		m_ulQueueFreeFailNum = ulQueueFreeFailNum;
	}

	CMediaDataQueue* getMediaQueue()
	{
		return m_pMediaQueue;
	}

	CBlockCheckTimer* getBlockCheckTimer() 
	{
		return m_pBlockCheckTimer;
	}

	unsigned int getBufferSize()const
	{
		return m_unSize;
	}

	// ��ʱ����ʱ�ص�����
	//static void timer_callback(void *pArg, int eventId, long /*timerId*/, int /*nDummy*/);

	ACE_Message_Block* createMediaBlock(unsigned int unBlockSize)const;

private:
	static CMediaBlockBuffer* g_MediaBuffer;

	// ���ݿ鱣��Ķ���
	CMediaDataQueue* m_pMediaQueue;

	unsigned int     m_unSize;
	unsigned int     m_unCount;
	int              m_iLastFreeNum;     // �ϴ˴�ӡ��־ʱ ���л���ĸ���
	unsigned int     m_FailureTimes;
	unsigned int     m_unAllocBuffFailTimes;

	long m_lQueueTimerId;

	unsigned long m_ulQueueFreeFailNum;

	CBlockCheckTimer* m_pBlockCheckTimer;
};


#endif // __CLOCKFREEMEDIABUFFER_H__
