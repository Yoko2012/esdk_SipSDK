#ifndef __CMEDIADATAQUEUE_H__
#define __CMEDIADATAQUEUE_H__
#include "stdafx.h"
#include "ace_header.h"

#ifndef WIN32
#include "Atomic.h"
#else
#define MAX_DATA_BLOCK_SIZE    (2 * 1024)
#endif

/**
* @class   CMediaDataQueue
* @brief   ý�����ݶ���
* Description: ����������Ƶ�ý�����ݶ��У�������̲߳���
*/
class CMediaDataQueue
{
public:
    CMediaDataQueue();

    virtual ~CMediaDataQueue();

    /// ��ʼ�����У���������ܹ��������Ϣ��������
    int init(unsigned int unQueueSize);

    /// �رն��У�����������������������ʧ��
    void close();

#ifndef WIN32
    /// ��ȡ��������Ϣ�����
    unsigned int message_count() const;

    /// �������Ƿ�Ϊ��
    bool empty() const;

    /// �������Ƿ�����
    bool full() const;
#else
    /// ��ȡ��������Ϣ�����
    unsigned int message_count();

    /// �������Ƿ�Ϊ��
    bool empty();

    /// �������Ƿ�����
    bool full();
#endif

    /// �����β��������ݿ�
    int enqueue_tail(ACE_Message_Block* mb, const ACE_Time_Value *timeout = NULL);

    /// �Ӷ���ͷȡ��һ�����ݿ�
    int dequeue_head(ACE_Message_Block*& mb, const ACE_Time_Value *timeout = NULL);
private:
#ifdef WIN32
    ACE_Message_Queue<ACE_SYNCH>    m_DataQueue;
#else
    // �����ĸ�����˳���ܱ��޸Ļ�����������壬CAS2��ȷ��Ҫ���¶���
    volatile unsigned int  m_WriteIndex;           /// ʼ��ָ����һ��Ҫ����λ�õ�����
    volatile unsigned int  m_WriteTag;
    volatile unsigned int  m_ReadIndex;            /// ָ��Ҫ��ȡ�����ݵ�λ��
    volatile unsigned int  m_ReadTag;

    volatile int           m_ActiveFlag;           /// ���м����־��Close�����ȥ����
    unsigned int           m_unMaxQueueSize;       /// ������󻺳����ݿ���
    unsigned int           m_unArraySize;          /// ������ʵ�ʳ���: m_unMaxQueueSize + 1

    volatile unsigned int  m_QueueSize;            /// ��ǰ���ݿ���
    ACE_Message_Block**    m_pDataArray;           /// ���ݶ���
#endif
};

#endif // __CMEDIADATAQUEUE_H__
