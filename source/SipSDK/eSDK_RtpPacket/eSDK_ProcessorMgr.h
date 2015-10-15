
#include <map>
#include "Ps2EsProcessor.h"

class CProcessorMgr
{
private:
	// ��ʵ�������캯����Ϊ˽��;
	CProcessorMgr(void); 


public:
	   /******************************************************************
     function   : instance
     description: ��ʵ��
     output     : NA
     return     : CProcessorMgr& ��ʵ������;
    *******************************************************************/
	static CProcessorMgr& instance()
	{
		static CProcessorMgr ProcessorMgr;
		return ProcessorMgr;
	}

	    /******************************************************************
     function   : ~CProcessorMgr
     description: ��������;
     input      : void
     output     : NA
     return     : NA
    *******************************************************************/
    ~CProcessorMgr(void);

	  /******************************************************************
     function   : Init
     description: ��ʼ��
     input      : void
     output     : NA
     return     : void
    *******************************************************************/
    void Init(void);

	  /******************************************************************
     function   : UnInit
     description: ���;
     input      : void
     output     : NA
     return     : void
    *******************************************************************/
    void UnInit(void);

	  /******************************************************************
     function   : GetProcessor
     description: ��ȡ������
     input      : NA
     output     : unsigned long & ulServiceId
     return     : int �ɹ�:IVS_SUCCEED ʧ��:IVS_FAIL;
    *******************************************************************/
    int GetProcessor(unsigned long & ulServiceId);

	  /******************************************************************
     function   : FreeProcessor
     description: �ͷ�ָ��������
     input      : unsigned long ulServiceId
     output     : NA
     return     : int �ɹ�:IVS_SUCCEED ʧ��:IVS_FAIL;
    *******************************************************************/
    int FreeProcessor(unsigned long ulServiceId);


private:
	  
	//ulServiceId��ת��������;
	typedef std::map<unsigned long, CPs2EsProcessor*> ServiceProcessorMap;
	typedef ServiceProcessorMap::iterator ServiceProcessorMapIter;
	ServiceProcessorMap m_HanleRealPlayMap;
}