#include "stdafx.h"
#include "MediaDataQueue.h"
#include "Common_Def.h"

/**
* Description: ���캯��
* @param ��
* @return ��
*/
CMediaDataQueue::CMediaDataQueue()
{
#ifndef WIN32
    atomic_set(m_ActiveFlag, 1); // ���б�־Ϊ0ʱ�����в�����Ϊ����ʧ��
    m_unArraySize    = 0;
    m_unMaxQueueSize = 0;

    atomic_set(m_QueueSize, 0);
    m_pDataArray  = NULL;

    atomic_set(m_WriteIndex, 0);
    atomic_set(m_WriteTag, 0);
    atomic_set(m_ReadIndex, 0);
    atomic_set(m_ReadTag, 0);
#endif
}

CMediaDataQueue::~CMediaDataQueue()
{
#ifndef WIN32
    if (NULL != m_pDataArray)
    {
        delete[] m_pDataArray;
        m_pDataArray = NULL;
    }
#endif
}

/**
* Description: ��ʼ������
* @param [in] unQueueSize: ������󻺳���Ϣ����
* @return NRU_SUCCESS - �ɹ���Others - ʧ��
*/
int CMediaDataQueue::init(unsigned int unQueueSize)
{
#ifndef WIN32
    atomic_set(m_ActiveFlag, 1); // ���б�־Ϊ0ʱ�����в�����Ϊ����ʧ��
    m_unMaxQueueSize = unQueueSize;
    m_unArraySize    = unQueueSize + 1;

    atomic_set(m_QueueSize, 0);
    m_pDataArray  = NULL;

    atomic_set(m_WriteIndex, 0);
    atomic_set(m_WriteTag, 0);
    atomic_set(m_ReadIndex, 0);
    atomic_set(m_ReadTag, 0);

    m_unMaxQueueSize = unQueueSize;
    m_unArraySize    = unQueueSize + 1;

    try
    {
        // ������һ����λ
        m_pDataArray = new ACE_Message_Block*[m_unArraySize];
    }
    catch (...)
    {
        return NRU_FAIL;
    }

    memset(m_pDataArray, 0x0, sizeof(ACE_Message_Block*) * m_unArraySize);
    return NRU_SUCCESS;
#else
    return m_DataQueue.open(unQueueSize * MAX_DATA_BLOCK_SIZE);
#endif
}

/**
* Description: �رն��У��ᵼ�³�����еĲ�����������ʧ��
* @param ��
* @return ��
*/
void CMediaDataQueue::close()
{
#ifndef WIN32
    atomic_set(m_ActiveFlag, 0); // ���б�־Ϊ0ʱ�����в�����Ϊ����ʧ��
#else
    (void)m_DataQueue.close();
#endif
}

/**
* Description: ��ȡ��ǰ���������ݿ���
* @param ��
* @return ���ض������ݿ����
*/
#ifndef WIN32
unsigned int CMediaDataQueue::message_count() const
{
    return (unsigned int) atomic_read(m_QueueSize);
}

#else
unsigned int CMediaDataQueue::message_count()
{
    return m_DataQueue.message_count();
}

#endif
/**
* Description: �������Ƿ�Ϊ��
* @param ��
* @return true - ���п�  false - ������
*/
#ifndef WIN32

bool CMediaDataQueue::empty() const
{
    return (atomic_read(m_QueueSize) == 0);
}
#else

bool CMediaDataQueue::empty()
{
    return m_DataQueue.is_empty();
}
#endif

/**
* Description: �������Ƿ�����
* @param ��
* @return true - ������  false - ���в���
*/
#ifndef WIN32

bool CMediaDataQueue::full() const
{
    return ((unsigned int) atomic_read(m_QueueSize) >= m_unMaxQueueSize);
}
#else

bool CMediaDataQueue::full()
{
    return m_DataQueue.is_full();
}

#endif
/**
* Description: �����β��������ݿ�
* @param [in] mb : Ҫ��ӵ����ݿ�
* @param [in] timeout : ��ʱʱ�䣬Ҫ�����Ϊ���ʱ�䣬���ΪNULL��������ܷ�������
* @return NRU_SUCCESS - �ɹ���Others - ʧ��
*/
int CMediaDataQueue::enqueue_tail(ACE_Message_Block* mb, const ACE_Time_Value *timeout)
{
    if (NULL == mb)
    {
        return NRU_FAIL;
    }
#ifndef WIN32
    if ((NULL == m_pDataArray) || (0 == m_unArraySize))
    {
        return NRU_FAIL;
    }
    bool bFlag               = false;
    bool bTimeOut            = false;
    unsigned int unHeadIndex = 0;

    ACE_Time_Value tvTimeOut;
    ACE_Time_Value startTime;
    if (NULL != timeout)
    {
        startTime = ACE_OS::gettimeofday();
        bTimeOut  = true;
        tvTimeOut = *timeout;
    }

    bool bHaveTry = false;      // �Ƿ���й����Է���Ϣ

    // �ȳ��԰�Ҫ�ӵ�����λռ��ס
    while (0 != atomic_read(m_ActiveFlag))
    {
        if (bTimeOut && bHaveTry && (ACE_OS::gettimeofday() - startTime > tvTimeOut))
        {
            return NRU_FAIL;
        }

        bHaveTry = true;

        unHeadIndex = (unsigned int)atomic_read(m_WriteIndex);
        unsigned int unTag       = (unsigned int)atomic_read(m_WriteTag);
        if ((unHeadIndex + 1) % m_unArraySize == (unsigned int)atomic_read(m_ReadIndex))
        {
            // ���������ȴ�1ms
            //ACE_Time_Value timeout(0, 0);
            //ACE_OS::sleep(timeout);
            (void)usleep(0);

            continue;
        }

        if (NULL != m_pDataArray[unHeadIndex])
        {
            // ��������
            continue;
        }

        if (compare_and_swap2(&m_WriteIndex, unHeadIndex, unTag, (unHeadIndex + 1) % m_unArraySize, unTag + 1))
        {
            bFlag = true;
            break;
        }

    }

    if (bFlag)
    {
        // ֱ�Ӹ�ֵ
        m_pDataArray[unHeadIndex] = mb;
        atomic_inc(&m_QueueSize);

        return NRU_SUCCESS;
    }

    return NRU_FAIL;
#else
    ACE_Time_Value enTime;

    if (timeout)
    {
        enTime = ACE_OS::gettimeofday();
        enTime += *timeout;
        ACE_OS::last_error(0);
        if (-1 == m_DataQueue.enqueue_tail(mb, &enTime))
        {
            int errno;
            ACE_OS::last_error(errno);
            //IVS_NEW_DBG_LOG ("enqueue fail, now count=%d, errno=%d.", m_DataQueue.message_count(),errno);
            return NRU_FAIL;
        }
    }
    else if (-1 == m_DataQueue.enqueue_tail(mb))
    {
        //IVS_NEW_DBG_LOG ("enqueue fail, now count=%d", m_DataQueue.message_count());
        return NRU_FAIL;
    }

    return NRU_SUCCESS;
#endif
}

/**
* Description: �Ӷ���ͷ��ȡ�����ݿ�
* @param [out] mb : ���ݿ�ָ��
* @param [in] timeout : ��ʱʱ�䣬Ҫ�����Ϊ���ʱ�䣬���ΪNULL��������ܷ�������
* @return NRU_SUCCESS - �ɹ���Others - ʧ��
*/
int CMediaDataQueue::dequeue_head(ACE_Message_Block* &mb, const ACE_Time_Value *timeout)
{
#ifndef WIN32
    if ((NULL == m_pDataArray) || (0 == m_unArraySize))
    {
        return NRU_FAIL;
    }

    bool bFlag               = false;
    bool bTimeOut            = false;
    unsigned int unTailIndex = 0;
    ACE_Time_Value tvTimeOut;
    ACE_Time_Value startTime;
    
    if (NULL != timeout)
    {
        startTime = ACE_OS::gettimeofday();
        bTimeOut  = true;
        tvTimeOut = *timeout;
    }

    bool bHaveTry = false;      // �Ƿ���й�����ȡ��Ϣ

    while (0 != atomic_read(m_ActiveFlag))
    {
        if (bTimeOut && bHaveTry && (ACE_OS::gettimeofday() - startTime > tvTimeOut))
        {
            return NRU_FAIL;
        }

        bHaveTry = true;

        unTailIndex = (unsigned int)atomic_read(m_ReadIndex);
        unsigned int unTag = (unsigned int)atomic_read(m_ReadTag);

        if (unTailIndex == (unsigned int)atomic_read(m_WriteIndex))
        {
            // ����Ϊ�գ��ȴ�1ms
            ACE_Time_Value timeout(0, 1000);
            ACE_OS::sleep(timeout);
            //(void)usleep(0);

            continue;
        }


        if (NULL == m_pDataArray[unTailIndex])
        {
            // ���в�Ϊ�յ�Ҫȡ��ֵΪNULL�����ִ�����Ҫ����
            continue;
        }

        if (compare_and_swap2(&m_ReadIndex, unTailIndex, unTag, (unTailIndex + 1) % m_unArraySize, unTag + 1))
        {
            bFlag = true;
            break;
        }

    }

    if (bFlag)
    {
        mb = m_pDataArray[unTailIndex];
        m_pDataArray[unTailIndex] = 0;
        atomic_dec(&m_QueueSize);

        return NRU_SUCCESS;
    }

    return NRU_FAIL;

#else
    ACE_Time_Value enTime;

    if (timeout)
    {
        enTime = ACE_OS::gettimeofday();
        enTime += *timeout;
        if (-1 == m_DataQueue.dequeue_head(mb, &enTime))
        {
            return NRU_FAIL;
        }
    } 
    else if(-1 == m_DataQueue.dequeue_head(mb))
    {
        return NRU_FAIL;
    }

    return NRU_SUCCESS;
#endif
}

