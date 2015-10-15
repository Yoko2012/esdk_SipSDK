/******************************************************************************
   ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �ļ���          : CNVSTimer.cpp
  �汾��          : 1.0
  ����            : duijiying 24362
  ��������        : 2007-4-02
  ����޸�        : 
  ��������        : 
  �����б�        : 
  �޸���ʷ        : 
  1 ����          : 2007-4-02
    ����          : duijiying
    �޸�����      : ����
*******************************************************************************/

#include <stdarg.h>

#include "vos_config.h"
#include "vos_basetype.h"
#include "vos_common.h"
#include "vos.h"
#include "CConnMgr.h"
#include "CNVSTimer.h"

#pragma  warning(push)
#pragma  warning(disable:4996)

//lint -e438

namespace SipStack{
ITimerLog *g_pTimerLog = NULL;
#define MAX_TIMER_LOG_LENTH 512

#ifndef WIN32   //��PC-LINT
#define TIMER_SECOND_IN_MS 1000
#define TIMER_MS_IN_US 1000
#endif

#define _TIMER_FL_ "CNVSTimer.cpp", __LINE__
/*******************************************************************************
  Function:       TIMER_WRITE_LOG()
  Description:    ��־��ӡ����
  Calls:            
  Called By:      
  Input:          ��printfһ��
  Output:         �� 
  Return:         ��
*******************************************************************************/
void TIMER_WRITE_LOG(long lLevel, const char *format, ...)
{
    if(NULL == g_pTimerLog)
    {
        return;
    };
    
    char buff[MAX_TIMER_LOG_LENTH + 1];
    buff[0] = '\0';
    
    va_list args;
    va_start (args, format);
    long lPrefix = snprintf (buff, MAX_TIMER_LOG_LENTH, "errno:%d.thread(%u):", 
        errno, (unsigned int)VOS_pthread_self());
    if(lPrefix < MAX_TIMER_LOG_LENTH)
    {
        (void)vsnprintf (buff + lPrefix,
            (unsigned long)(MAX_TIMER_LOG_LENTH - lPrefix), format, args);
    }
    buff[MAX_TIMER_LOG_LENTH] = '\0';
    g_pTimerLog->writeLog(TIMER_RUN_LOG, lLevel, buff, (long)strlen(buff));
    va_end (args);
    
};

CNVSTimer* CNVSTimer::m_pTimer = NULL;
/*******************************************************************************
  Function:       CNVSTimer::~CNVSTimer()
  Description:    ��������
  Calls:            
  Called By:      
  Input:          ��
  Output:         �� 
  Return:         ��
*******************************************************************************/
CNVSTimer::~CNVSTimer()
{
    try
    {
        if(NULL != m_plistTrigger)
        {
            ListOfTriggerIte itListOfTrigger = m_plistTrigger->begin();
            TIMER_WRITE_LOG(TIMER_DEBUG, "FILE(%s)LINE(%d): CNVSTimer::~CNVSTimer: thread = %u", 
                _TIMER_FL_, VOS_pthread_self());
            while(itListOfTrigger != m_plistTrigger->end())
            {
                VOS_DELETE(itListOfTrigger->second);
                ++itListOfTrigger;
            };
            m_plistTrigger->clear();
            VOS_DELETE(m_plistTrigger);
            m_plistTrigger = NULL;
        }
    
        if(m_pVosThread != NULL)
        {
            VOS_free(m_pVosThread);
        }
        m_pVosThread = NULL;

        if(m_pMutexListOfTrigger != NULL)
        {
            (void)VOS_DestroyMutex(m_pMutexListOfTrigger);
        }
        m_pMutexListOfTrigger = NULL;
    }
    catch (...)
    {
    }
};

/*******************************************************************************
  Function:       CNVSTimer::init()
  Description:    ��ʼ������
  Calls:            
  Called By:      
  Input:          ulTimerScale: ��ʱ������
  Output:         �� 
  Return:         
  VOS_SUCCESS: init success
  VOS_FAIL: init fail 
*******************************************************************************/
long CNVSTimer::init(ULONG ulTimerScale)
{
    //��ʼ��ʱ������Ϊ���˳�״̬�������ظ���ͣ��ʧ��
    m_bExit = VOS_FALSE;

    if (ulTimerScale < MinTimerScale)
    {
        m_ulTimerScale = MinTimerScale;
    }
    else
    {
        m_ulTimerScale = ulTimerScale;
    }

    m_pMutexListOfTrigger = VOS_CreateMutex();
    if(NULL == m_pMutexListOfTrigger)
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): CNVSTimer::init: create m_pMutexListOfTrigger fail.",
            _TIMER_FL_);
        return VOS_FAIL;
    }
    
    m_ullRrsAbsTimeScales = VOS_GetTicks()/m_ulTimerScale;

    (void)VOS_NEW( m_plistTrigger );
    if( NULL == m_plistTrigger )
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): m_plistTrigger is NULL.",
            _TIMER_FL_);
        return VOS_FAIL;
    }

    return VOS_SUCCESS;
};

/*******************************************************************************
  Function:       CNVSTimer::run()
  Description:    ������ʱ����߳�
  Calls:            
  Called By:      
  Input:          �� 
  Output:         �� 
  Return:         
  VOS_SUCCESS: init success
  VOS_FAIL: init fail 
*******************************************************************************/
long CNVSTimer::run()
{
    errno = 0;
    if (VOS_OK != VOS_CreateThread((VOS_THREAD_FUNC)invoke, (void *)this, 
                                    &m_pVosThread, VOS_DEFAULT_STACK_SIZE))
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): Create timer thread failed. error(%d):%s", 
            _TIMER_FL_, errno, strerror(errno));
        return VOS_FAIL;
    };
    TIMER_WRITE_LOG(TIMER_DEBUG, 
        "FILE(%s)LINE(%d): VOS_CreateThread: create timer thread(%u) OK.", 
        _TIMER_FL_, m_pVosThread->pthead);

    return VOS_SUCCESS;
};

/*******************************************************************************
  Function:       CNVSTimer::exit()
  Description:    �˳���ʱ����߳�
  Calls:            
  Called By:      
  Input:          �� 
  Output:         �� 
  Return:         �� 
*******************************************************************************/
void CNVSTimer::exit()
{
    if(NULL == m_pVosThread)
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): CNVSTimer::exit: m_pVosThread is null", _TIMER_FL_);
        return;
    }
        
    this->m_bExit = VOS_TRUE;

    clearTimer();

    
    errno = 0;   
    long ret_val = VOS_ThreadJoin(m_pVosThread);
    if (ret_val != VOS_OK)
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): Wait timer thread exit failed. ret_val(%d). error(%d):%s", 
            _TIMER_FL_, ret_val, errno, strerror(errno));
    }    

    TIMER_WRITE_LOG(TIMER_DEBUG, 
        "FILE(%s)LINE(%d): CNVSTimer::exit: exit complete. Thread = %u", 
        _TIMER_FL_, m_pVosThread->pthead);
  
    return;
};

/*******************************************************************************
  Function:       CNVSTimer::registerTimer()
  Description:    ע�ᶨʱ��
  Calls:            
  Called By:      
  Input:          pTrigger: ��ʱ�������ʵ��, void *pArg: ��ʱ�������, 
                  nScales: ��ʱ�¼�(��initʱָ����scaleΪ��λ)
                  enStyle: �������    enOneShot: ����һ��, enRepeated: ѭ������
  Output:         �� 
  Return:         
  VOS_SUCCESS: init success
  VOS_FAIL: init fail 
*******************************************************************************/
long CNVSTimer::registerTimer(ITrigger *pTrigger, void *pArg, ULONG nScales, 
    TriggerStyle enStyle)
{
    if (NULL == pTrigger )
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): CNVSTimer::registerTimer: pTrigger is NULL", 
            _TIMER_FL_);
        return VOS_FAIL;
    }

    if (0 == nScales )
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): CNVSTimer::registerTimer: nScales is zero", 
            _TIMER_FL_);
        return VOS_FAIL;
    }    

    if(  VOS_TRUE == m_bExit)
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
                        "FILE(%s)LINE(%d): CTimer::registerTimer: "
                        "m_bExit is VOS_TRUE, thread exit\n", 
            _TIMER_FL_);
        return VOS_FAIL;
    }
    
    CTimerItem *pTimerItem = NULL;
    (void)VOS_NEW(pTimerItem);
    if (NULL == pTimerItem )
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): CNVSTimer::registerTimer: new pTimerItem fail",
            _TIMER_FL_);
        return VOS_FAIL;
    }
    
    pTrigger->m_pTimerItem = pTimerItem;

    pTimerItem->m_pTrigger = pTrigger;
    pTimerItem->m_pArg = pArg;
    pTimerItem->m_ulInitialScales = nScales;
    pTimerItem->m_ullCurScales = m_ullRrsAbsTimeScales + nScales;
    pTimerItem->m_enStyle = enStyle;
    //BEGIN V100R001C05 ADD 2010-02-03 ligengqiang l00124479 for �������������Ļ�ʯ�������������� NP-231 
    pTimerItem->m_ulBaseTimeTicks = VOS_GetTicks();
    //END   V100R001C05 ADD 2010-02-03 ligengqiang l00124479 for �������������Ļ�ʯ�������������� NP-231 

    //����(�����mainloop����ͬһ�̲߳���Ҫ����)
    VOS_BOOLEAN bNeedLock = VOS_FALSE;
    VOS_BOOLEAN bLocked = VOS_FALSE;
    if (NULL == m_pVosThread)
    {
        bNeedLock = VOS_TRUE;
    }
    else
    {
        if(VOS_pthread_self() != m_pVosThread->pthead)
        {
            bNeedLock = VOS_TRUE;
        }
    }
    
    if(VOS_TRUE == bNeedLock)
    {
        if (VOS_OK != VOS_MutexLock(m_pMutexListOfTrigger))
        {
            TIMER_WRITE_LOG(TIMER_ERROR, 
                "FILE(%s)LINE(%d):CNVSTimer::registerTimer: get lock failed",
                _TIMER_FL_);
        }
        else
        {
            bLocked = VOS_TRUE;
        }
    }
   
    (void)(m_plistTrigger->insert(ListOfTriggerPair(pTimerItem->m_ullCurScales, pTimerItem)));

    TIMER_WRITE_LOG(TIMER_DEBUG, 
        "FILE(%s)LINE(%d): registerTimer success"
        "  pTimerItem(0x%x) pTrigger(0x%x).", 
        _TIMER_FL_, pTrigger->m_pTimerItem, pTrigger);


    //����
    if(VOS_TRUE == bLocked)
    {
        if (VOS_OK != VOS_MutexUnlock(m_pMutexListOfTrigger))
        {
            TIMER_WRITE_LOG(TIMER_ERROR, 
                "FILE(%s)LINE(%d): CNVSTimer::registerTimer: release lock failed",
                _TIMER_FL_);
        }
    } 
    
    return VOS_SUCCESS;
};

/*******************************************************************************
  Function:       CNVSTimer::clearTimer
  Description:    ɾ����ʱ���������е�TimerItem
  Calls:            
  Called By:      
  Input:         �� 
  Output:         �� 
  Return:         
*******************************************************************************/
void CNVSTimer::clearTimer( )
{
    CTimerItem *pTimerItem = NULL;

    if(VOS_OK != VOS_MutexLock(m_pMutexListOfTrigger))
    {
        return;
    };
    ListOfTriggerIte itListOfTrigger = m_plistTrigger->begin();
    while(itListOfTrigger != m_plistTrigger->end())
    {
        pTimerItem = itListOfTrigger->second;
        ++itListOfTrigger;

        TIMER_WRITE_LOG(TIMER_DEBUG, 
            "FILE(%s)LINE(%d): clearTimer erase pTimerItem(0x%x).",
                        _TIMER_FL_, pTimerItem);
        VOS_DELETE(pTimerItem);
    }
    m_plistTrigger->clear();
    
    (void)VOS_MutexUnlock(m_pMutexListOfTrigger);
    
}

/*******************************************************************************
  Function:       CNVSTimer::cancelTimer()
  Description:    ע����ʱ��
  Calls:            
  Called By:      
  Input:          pTrigger: ��ʱ�������ʵ��
  Output:         �� 
  Return:         
  VOS_SUCCESS: init success
  VOS_FAIL: init fail 
*******************************************************************************/
long CNVSTimer::cancelTimer(ITrigger *pTrigger)
{
    if(NULL == pTrigger)
    {
        TIMER_WRITE_LOG(TIMER_ERROR, 
            "FILE(%s)LINE(%d): CNVSTimer::cancelTimer: pTrigger is NULL",
            _TIMER_FL_);
        return VOS_FAIL;
    };

    //����(�����mainloop����ͬһ�̲߳���Ҫ����)
    VOS_BOOLEAN bNeedLock = VOS_FALSE;
    VOS_BOOLEAN bLocked = VOS_FALSE;
    if (NULL == m_pVosThread)
    {
        bNeedLock = VOS_TRUE;
    }
    else
    {
        if(VOS_pthread_self() != m_pVosThread->pthead)
        {
            bNeedLock = VOS_TRUE;
        }
    }
    
    if(VOS_TRUE == bNeedLock)
    {
        if (VOS_OK != VOS_MutexLock(m_pMutexListOfTrigger))
        {
            TIMER_WRITE_LOG(TIMER_ERROR, 
                "FILE(%s)LINE(%d): CNVSTimer::cancelTimer: get lock failed",
                _TIMER_FL_);
        }
        else
        {
            bLocked = VOS_TRUE;
        }
    }

    TIMER_WRITE_LOG(TIMER_DEBUG, 
            "FILE(%s)LINE(%d): cancelTimer set m_bRemoved=VOS_TRUE"
            "  pTimerItem(0x%x) pTrigger(0x%x).", 
            _TIMER_FL_, pTrigger->m_pTimerItem, pTrigger);
 
    if(pTrigger->m_pTimerItem != NULL)
    {
        pTrigger->m_pTimerItem->m_bRemoved = VOS_TRUE;
        pTrigger->m_pTimerItem->m_pTrigger = NULL;
        pTrigger->m_pTimerItem = NULL;
    }

    
    //����(�������ͬһ�߳�)
    if(VOS_TRUE == bLocked)
    {
        if (VOS_OK != VOS_MutexUnlock(m_pMutexListOfTrigger))
        {
            TIMER_WRITE_LOG(TIMER_ERROR, 
                "FILE(%s)LINE(%d): CNVSTimer::cancelTimer: release lock failed",
                _TIMER_FL_);
        }
    }        
    
    return VOS_SUCCESS;
};

/*******************************************************************************
  Function:       CNVSTimer::mainLoop()
  Description:    �߳�ִ�еĶ�ʱ�����ѭ��
  Calls:            
  Called By:      
  Input:          ��
  Output:         �� 
  Return:         ��
*******************************************************************************/
void CNVSTimer::mainLoop()
{
    ULONGLONG ullCurrentScales = 0;
    while(VOS_TRUE != m_bExit)
    {

#ifdef WIN32
        Sleep(m_ulTimerScale);
#else
        struct timeval tv;
        tv.tv_sec = (long)(m_ulTimerScale/TIMER_SECOND_IN_MS);
        tv.tv_usec = (m_ulTimerScale%TIMER_SECOND_IN_MS)*TIMER_MS_IN_US;
        (void)select(0, VOS_NULL, VOS_NULL, VOS_NULL, &tv);
#endif
        //����
        CTimerItem *pTimerItem = NULL;
        ITrigger *pTrigger = NULL;
        ++m_ullRrsAbsTimeScales ;//�ڲ�ʱ���׼����һ���̶�
        ullCurrentScales = m_ullRrsAbsTimeScales;

        if(VOS_OK != VOS_MutexLock(m_pMutexListOfTrigger))
        {
            break;
        };
        ListOfTriggerIte itListOfTrigger = m_plistTrigger->begin();
        ListOfTriggerIte itCurrentTrigger = m_plistTrigger->begin();
        while(itListOfTrigger != m_plistTrigger->end())
        {
            pTimerItem = (*itListOfTrigger).second;
            if(NULL == pTimerItem)
            {
                TIMER_WRITE_LOG(TIMER_ERROR, "FILE(%s)LINE(%d): pTimerItem is NULL.", _TIMER_FL_);
                itCurrentTrigger = itListOfTrigger;
                ++itListOfTrigger;
                
                (void)(m_plistTrigger->erase(itCurrentTrigger));
                // �������û��Ԫ����
                // ֱ������ѭ��
                if (0 == m_plistTrigger->size())
                {
                    TIMER_WRITE_LOG(TIMER_DEBUG, "FILE(%s)LINE(%d): no timer now.", _TIMER_FL_);
                    break;
                }
                
                continue;
            }
            
            pTrigger = (ITrigger *)pTimerItem->m_pTrigger;
            
            //�����ʱ����ע����ɾ����ʱ��������������һ��
            if((NULL == pTrigger) || (VOS_TRUE == pTimerItem->m_bRemoved))
            {
                TIMER_WRITE_LOG(TIMER_DEBUG, 
                    "FILE(%s)LINE(%d): Timer(0x%x) removed.", _TIMER_FL_, pTimerItem);
                itCurrentTrigger = itListOfTrigger;
                ++itListOfTrigger;

                (void)(m_plistTrigger->erase(itCurrentTrigger));
                VOS_DELETE(pTimerItem);

                // �������û��Ԫ����
                // ֱ������ѭ��
                if (0 == m_plistTrigger->size())
                {
                    TIMER_WRITE_LOG(TIMER_DEBUG, "FILE(%s)LINE(%d): no timer now.", _TIMER_FL_);
                    break;
                }
                continue;
            };

            //BEGIN V100R001C05 ADD 2010-02-03 ligengqiang l00124479 for �������������Ļ�ʯ�������������� NP-231 
            ULONG ulTimeTickInterval = VOS_GetTicks() - pTimerItem->m_ulBaseTimeTicks;
            //SipStack::sipStackLoger::getInstance()->debug(__FILE__, __LINE__, 
            //    "CNVSTimer::mainLoop, ulTimeTickInterval=%lu, VOS_GetTicks()=%lu, pTimerItem->m_ulBaseTimeTicks=%lu, pTimerItem->m_ulInitialScales=%lu, m_ulTimerScale=%lu",
            //    ulTimeTickInterval, VOS_GetTicks(), pTimerItem->m_ulBaseTimeTicks, pTimerItem->m_ulInitialScales, m_ulTimerScale);

            if (ulTimeTickInterval < pTimerItem->m_ulInitialScales * m_ulTimerScale)
            {
                ++itListOfTrigger;
                continue;
            }
            pTimerItem->m_ulBaseTimeTicks = VOS_GetTicks();
            //END   V100R001C05 ADD 2010-02-03 ligengqiang l00124479 for �������������Ļ�ʯ�������������� NP-231 
            
            itCurrentTrigger = itListOfTrigger;
            ++itListOfTrigger;
            (void)(m_plistTrigger->erase(itCurrentTrigger));

            //����trigger��onTrigger����
            pTrigger->OnTrigger(pTimerItem->m_pArg, 
                ullCurrentScales, pTimerItem->m_enStyle);

            //���ֻ����һ�Σ�ɾ����ʱ��
            if(enOneShot == pTimerItem->m_enStyle)
            {
                TIMER_WRITE_LOG(TIMER_DEBUG, 
                    "FILE(%s)LINE(%d): Timer(0x%x) remove trigger once timer.", 
                    _TIMER_FL_, pTimerItem);
                //��ֹpTrigger->onTrigger��ɾ����ʱ��
                pTrigger->m_pTimerItem = NULL;
                VOS_DELETE(pTimerItem);

                // �������û��Ԫ����
                // ֱ������ѭ��
                if (0 == m_plistTrigger->size())
                {
                    TIMER_WRITE_LOG(TIMER_DEBUG, "FILE(%s)LINE(%d): no timer now.", _TIMER_FL_);
                    break;
                }
                continue;
            }

            //�����ظ�������ʱ������Ҫ�޸��´γ�ʱ�¼������¼���multimap
            pTimerItem->m_ullCurScales = ullCurrentScales + pTimerItem->m_ulInitialScales;
            (void)(m_plistTrigger->insert(ListOfTriggerPair(pTimerItem->m_ullCurScales, 
                pTimerItem)));
        };
        (void)VOS_MutexUnlock(m_pMutexListOfTrigger);
    }

    return;
}
}
//lint +e438
#pragma  warning(pop)

