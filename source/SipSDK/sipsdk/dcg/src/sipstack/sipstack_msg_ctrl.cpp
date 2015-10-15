#include "tinyxml.h"
#include "sipstack_msg_ctrl.h"
#include "Log.h"

namespace SipStack{

#define SIPSTACK_DEQUE_LENGTH_DFLT                       (3000)

CSipStackCtrlMsg::CSipStackCtrlMsg( unsigned int uiID, const std::string strName ):m_uiID(uiID),m_strName(strName),m_pCtrlMsgBody(NULL)
{
}
CSipStackCtrlMsg::~CSipStackCtrlMsg()
{
    if ( NULL != m_pCtrlMsgBody )
    {
        delete m_pCtrlMsgBody;
        m_pCtrlMsgBody = NULL;
    }
}
unsigned int CSipStackCtrlMsg::GetID() const
{
    return m_uiID;
}
CSipStackCtrlMsgBody* CSipStackCtrlMsg::GetBody() const
{
    return m_pCtrlMsgBody;
}
void CSipStackCtrlMsg::SetCtrlMsgBody( CSipStackCtrlMsgBody* pCtrlMsgBody )
{
    m_pCtrlMsgBody = pCtrlMsgBody;
}
const std::string& CSipStackCtrlMsg::GetName() const
{
    return m_strName;
}

CSipStackCtrlMsgBody::CSipStackCtrlMsgBody( unsigned int uiBodyType )
    : m_uiBodyType(uiBodyType)
{

}
CSipStackCtrlMsgBody::~CSipStackCtrlMsgBody()
{

}
unsigned int CSipStackCtrlMsgBody::GetBodyType() const
{
    return m_uiBodyType;
}

/*******************************************************/
//���ڽ�����Ϣ��ΪXML��ʽ�Ķ���
CSipStackCtrlMsgBodyXML::CSipStackCtrlMsgBodyXML()
    : CSipStackCtrlMsgBody(SIP_MSG_BODY_TYPE_XML)
{
}
CSipStackCtrlMsgBodyXML::~CSipStackCtrlMsgBodyXML()
{
}
void CSipStackCtrlMsgBodyXML::SetElement( const std::string& strElem )
{
    m_strElem = strElem;
}
void CSipStackCtrlMsgBodyXML::SetAttribute( const std::string& strAttr )
{
    m_strAttr = strAttr;
}
void CSipStackCtrlMsgBodyXML::SetValue( const std::string& strValue )
{
    m_strValue = strValue;
}
bool CSipStackCtrlMsgBodyXML::IsFound( const std::string& strBody )
{
    std::string strValue("");
    if ( !ParseBody2Find(strBody, strValue) )
    {
        return false;
    }

    if ( 0 != m_strValue.compare(strValue) )
    {
        return false;
    }

    return true;
}
bool CSipStackCtrlMsgBodyXML::ParseBody2Find( const std::string& strBody, std::string& strRet )
{
    if ( strBody.empty() || m_strElem.empty() )
    {
        ERROR_LOG("find value in xml - invalid param");
        return false;
    }

    //1.����XML
    TiXmlDocument aXmlDoc;
    aXmlDoc.Parse(strBody.c_str(),0,TIXML_ENCODING_LEGACY);

    //2.���ҽڵ�
    std::string strTmp("");

    TiXmlHandle aXmlHandle(&aXmlDoc);
    for ( unsigned int nposB = 0, nposE = 0;
          std::string::npos != ( nposE = m_strElem.find('/', nposB) ) || ( std::string::npos != nposB );//lint !e838
          nposB = (std::string::npos == nposE) ? std::string::npos : ( nposE + 1 )
        )
    {
        strTmp = m_strElem.substr(nposB, nposE - nposB );
        if ( strTmp.empty() )
        {
            continue;
        }
        aXmlHandle = aXmlHandle.Child(strTmp.c_str(), 0);
    }

    //3.���ֵ
    TiXmlElement* pElem = aXmlHandle.ToElement();
    if ( NULL == pElem )
    {
        ERROR_LOG("find value in xml - failure to find element - Element=%s",m_strElem.c_str());
        return false;
    }

    const char* pchTmp = NULL;

    if ( m_strAttr.empty() )
    {
        if ( NULL != ( pchTmp = pElem->GetText() ) )
        {
            strRet = pchTmp;
        }
        else
        {
            ERROR_LOG("find value in xml - failure to get element value - Element=%s",m_strElem.c_str());
			return false;
        }
    }
    else
    {
        if ( NULL != ( pchTmp = pElem->Attribute(m_strAttr.c_str()) ) )
        {
            strRet = pchTmp;
        }
        else
        {
            ERROR_LOG("find value in xml - failure to get attribute value - Element=%s",m_strElem.c_str());
			return false;
        }
    }

    return true;
}

/*******************************************************/

SipStack::CSipStackCtrlMsgMgr CSipStackCtrlMsgMgr::m_instance;

CSipStackCtrlMsgMgr::CSipStackCtrlMsgMgr()
    :m_nQueueLength(SIPSTACK_DEQUE_LENGTH_DFLT)
    ,m_itCurrentCondition(m_mapCtrlMsg.end())
{
    m_mapCtrlMsg[0] = m_mapCtrlMsg[m_nQueueLength];   //ͬʱ����һ�����г���Ϊ����ﵽ���ʱ����������
}

CSipStackCtrlMsgMgr::CSipStackCtrlMsgMgr( CSipStackCtrlMsgMgr&)
	:m_nQueueLength(SIPSTACK_DEQUE_LENGTH_DFLT)
{

}
CSipStackCtrlMsgMgr::~CSipStackCtrlMsgMgr()
{
    m_itCurrentCondition = m_mapCtrlMsg.end();
    if ( !m_mapCtrlMsg.empty() )
    {
        for ( SS_CTRL_MSG_MAP_T::iterator iterCtrlMap = m_mapCtrlMsg.begin();
              m_mapCtrlMsg.end() != iterCtrlMap;
              iterCtrlMap++
            )
        {
			SS_CTRL_MSG_VEC_T& SSVec = (SS_CTRL_MSG_VEC_T)iterCtrlMap->second;
            if ( SSVec.empty() )
            {
                continue;
            }

            for ( SS_CTRL_MSG_VEC_T::iterator iterCtrlQueue = SSVec.begin();
                  SSVec.end() != iterCtrlQueue;
                  iterCtrlQueue++
                )
            {
                if ( NULL != (*iterCtrlQueue) )
                {
                    delete (*iterCtrlQueue);
                    (*iterCtrlQueue) = NULL;
                }
            }

            SSVec.clear();
        }

        m_mapCtrlMsg.clear();
    }
}
CSipStackCtrlMsgMgr& CSipStackCtrlMsgMgr::operator=( CSipStackCtrlMsgMgr& )
{
    return *this;
}

CSipStackCtrlMsgMgr& CSipStackCtrlMsgMgr::Instance()
{
    return m_instance;
}

SS_CTRL_MSG_MAP_T::key_type CSipStackCtrlMsgMgr::GetQueueLength() const
{
    return m_nQueueLength;
}

void CSipStackCtrlMsgMgr::UpdateCondition ( const SS_CTRL_MSG_MAP_T::key_type nCondition )
{
    if ( m_mapCtrlMsg.empty() )
    {
		INFO_LOG("update condition - control message map is empty.");
		return ;
    }

    if ( m_mapCtrlMsg.end() == m_itCurrentCondition )
    {
        m_itCurrentCondition = m_mapCtrlMsg.begin();
		WARN_LOG("update condition - exception - current condition is invalid.");
	}

    //if (  ( m_mapCtrlMsg.end() == m_itCurrentCondition + 1 )
    //   && ( m_mapCtrlMsg.begin() == m_itCurrentCondition )
    //   )
    //{
    //    INFO_LOG("update condition - only one control message queue." ;
    //    return ;
    //}

    SS_CTRL_MSG_MAP_T::iterator itOther = m_itCurrentCondition;
    if ( nCondition > m_itCurrentCondition->first )
    {
        itOther++;
        if ( m_mapCtrlMsg.end() == itOther )
        {
            //�Ѿ����������һ��������Ϣ����
            return ;
        }

        if ( nCondition < itOther->first )
        {
            //��һ��������Ϣ���е�����δ����
            return ;
        }
        else
        {
            //�ﵽ��һ��������Ϣ��������
            m_itCurrentCondition = itOther;
            return ;
        }
    }
    else
    {
        if ( m_mapCtrlMsg.begin() == itOther )
        {
            //�Ѿ���������һ��������Ϣ����
            return ;
        }

        if ( nCondition >= itOther->first )
        {
            //��һ��������Ϣ���е�����δ����
            return ;
        }
        else
        {
            //�ﵽ��һ��������Ϣ��������
            m_itCurrentCondition = itOther;
            return ;
        }
    }
}

bool CSipStackCtrlMsgMgr::IsControl2Drop( SipMsgS& stSipMsg )
{
    if ( m_mapCtrlMsg.empty() )
    {
        //�ر�أ��޿���ӳ����Ϣ������Ϊ�����п��ơ�
        INFO_LOG("judge message whether to drop - control message map is empty.") ;
        return false;
    }

    if ( m_mapCtrlMsg.end() == m_itCurrentCondition )
    {
        //������ָ��Ƿ���Ĭ�ϲ����п��ơ�
        WARN_LOG("judge message whether to drop - exception - current condition is invalid.");
        return false;
    }

	SS_CTRL_MSG_VEC_T& SSVec = (SS_CTRL_MSG_VEC_T)m_itCurrentCondition->second;
    if (  ( 0 == m_itCurrentCondition->first )
       && ( SSVec.empty() )
       )
    {
        //�ر�أ�����Ϣ���г���Ϊ0ʱ���������Ϣ����ҲΪ�գ���������Ϣ�����п��ơ�
        INFO_LOG("judge message whether to drop - needless control.");
        return false;
    }

    for ( SS_CTRL_MSG_MAP_T::iterator iterCtrlMap = m_itCurrentCondition;
          m_mapCtrlMsg.end() != iterCtrlMap;
          iterCtrlMap ++
        )
    {
		SS_CTRL_MSG_VEC_T& SSCMsgVec= (SS_CTRL_MSG_VEC_T)iterCtrlMap->second;
        if ( SSCMsgVec.empty() )
        {
            //�������Ϣ����Ϊ�գ���ʾ������Ϣ�����붪��
            continue;
        }

        for ( SS_CTRL_MSG_VEC_T::iterator iterCtrlQueue = SSCMsgVec.begin();
              SSCMsgVec.end() != iterCtrlQueue;
              iterCtrlQueue++
            )
        {
            if ( FindCtrlMsg(*iterCtrlQueue, stSipMsg) )
            {
                //�ҵ�����Ϣ,����Ϣ�����ж�����
                return false;
            }
        }
    }

    //����������Ҳ�������Ϣ������
    return true;
}

bool CSipStackCtrlMsgMgr::FindCtrlMsg( const CSipStackCtrlMsg* pSipStackCtrlMsg, SipMsgS& stSipMsg )
{
    if ( NULL == pSipStackCtrlMsg )
    {
        //��Ч��ָ��
        return false;
    }

    if ( stSipMsg.enMsgType != pSipStackCtrlMsg->GetID() )
    {
        //��Ϣ���Ͳ�ͬ
        return false;
    }

    do 
    {
        if ( NULL == pSipStackCtrlMsg->GetBody() )
        {
            break;
        }

        SipBody    stBody = SipDsmGetBody(&stSipMsg);
        unsigned int nLen = VppStrBufGetLength(stBody);
        if ( 0 == nLen )
        {
            return false;
        }

        std::vector<char> vecBuf(nLen + 1, '\0');

        if ( VPP_SUCCESS != VppStrBufCopyDataOut(stBody, 0, &vecBuf[0], &nLen) )
        {
            return false;
        }

        if ( !pSipStackCtrlMsg->GetBody()->IsFound(&vecBuf[0]) )
        {
            return false;
        }

    } while (false);
	INFO_LOG("message control manager - find message - Name=%s",pSipStackCtrlMsg->GetName().c_str());
	return true;
}

}

