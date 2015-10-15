/******************************************************************************
   ��Ȩ���� (C), 2008-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �ļ���          : CLockGuard.h
  �汾��          : 1.0
  ����            : 
  ��������        : 2008-8-17
  ����޸�        : 
  ��������        : ʵ������������
  �����б�        : 
  �޸���ʷ        : 
    ����          : 
    ����          : 
    �޸�����      : 
*******************************************************************************/

#ifndef IVSCBB_SS_LOCKGUARD
#define IVSCBB_SS_LOCKGUARD

#include "vos.h"

namespace SipStack{
class CLockGuard
{
  public:
    CLockGuard(VOS_Mutex *pMutex);
    ~CLockGuard();
    
  public:
    static void lock(VOS_Mutex *pMutex);
    static void unlock(VOS_Mutex *pMutex);

 private:
    VOS_Mutex *m_pMutex;
};

}//end namespace
#endif /* CLockGuard_H_INCLUDE  */


