
/******************************************************************************
   ��Ȩ���� (C), 2008-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �ļ���          : CLockGuard.cpp
  �汾��          : 1.0
  ����            : 
  ��������        : 2008-8-17
  ����޸�        : 
  ��������        : ʵ������������
  �����б�        : 
  �޸���ʷ        : 
  1 ����          : 
    ����          : 
    �޸�����      : 
*******************************************************************************/


#include "vos.h"
#include "CLockGuard.h"
namespace SipStack{  
CLockGuard::CLockGuard(VOS_Mutex *pMutex)
{
    m_pMutex = NULL;

    if(NULL == pMutex)
    {
        return;
    }

    m_pMutex = pMutex;

    (void)VOS_MutexLock(m_pMutex);
}

CLockGuard::~CLockGuard()
{
    if(NULL == m_pMutex)
    {
        return;
    }
    (void)VOS_MutexUnlock(m_pMutex);
    
    m_pMutex = NULL;
}

void CLockGuard::lock(VOS_Mutex *pMutex)
{
    if(NULL == pMutex)
    {
        return;
    }
    (void)VOS_MutexLock(pMutex);
}

void CLockGuard::unlock(VOS_Mutex *pMutex)
{
    if(NULL == pMutex)
    {
        return;
    }
    (void)VOS_MutexUnlock(pMutex);
}
}//end namespace

