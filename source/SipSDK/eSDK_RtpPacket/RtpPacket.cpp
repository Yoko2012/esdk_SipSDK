/*
 * RtpPacket.cpp
 *
 *  Created on: 2010-12-28
 *      Author: x00106873
 */
#include "stdafx.h"
#include "ace_header.h"
#include "RtpPacket.h"
#include "Common_Def.h"
//#include "bp_log.h"

const unsigned int RTP_VALID_LEN = 2000;
//lint -e1763
CRtpPacket::CRtpPacket()
{
    m_pRtpData      = NULL;
    m_ulPacketLen   = 0;
    m_ulHeadLen     = 0;

    m_pFixedHead    = NULL;
    m_pExtHead      = NULL;
    m_pExtData      = NULL;
    m_pMuExtData    = NULL;
    m_ulFixedHeadLen = 0;
    m_pExtDataHaveFillIn = NULL;
}

CRtpPacket::~CRtpPacket()
{
    m_pRtpData      = NULL;
    m_pFixedHead    = NULL;
    m_pExtHead      = NULL;

    m_pExtData      = NULL;
    m_pMuExtData    = NULL;
    m_pExtDataHaveFillIn = NULL;
}

int CRtpPacket::ParsePacket
(
    char*     pRtpData, 
    unsigned int   ulLen
)
{
    if (NULL == pRtpData)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "Rtp packet parse fail rtp data is null."," ");
        return NRU_FAIL;
    }
 
    // ����С�ڹ̶�ͷ�ĳ���, �Ҳ��ܳ���2000 (�ݶ�)
    if (ulLen < sizeof(RTP_FIXED_HEADER)
        || RTP_VALID_LEN < ulLen)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "Rtp packet parse fail rtp data.","len=%u is shorter than fixed head[%d] len.",			ulLen, sizeof(RTP_FIXED_HEADER));
        return NRU_FAIL;
    }
    m_pRtpData = (char*)pRtpData;
    m_ulPacketLen = ulLen;

    // ��ָ���̶�ͷ
    m_pFixedHead = (RTP_FIXED_HEADER*)(void*)m_pRtpData;

    unsigned int ulHeadLen = sizeof(RTP_FIXED_HEADER) + m_pFixedHead->csrc_len * RTP_CSRC_LEN;
    if (ulLen < ulHeadLen)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "Rtp packet parse fail rtp data len is shorter than fixed head len."," ");
        return NRU_FAIL;
    }

    // ���汾��
    if (NRU_SUCCESS != CheckVersion())
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "Rtp packet parse fail check version fail."," ");
        return NRU_FAIL;
    }

    //����Ƿ����������
    if (1 == m_pFixedHead->padding) 		
    {		
		if (m_ulPacketLen > (unsigned int)pRtpData[ulLen - 1]) //lint !e571
		{
			m_ulPacketLen -= pRtpData[ulLen - 1];//lint !e737
		}
		else 
		{
			//BP_RUN_LOG_WAR("Rtp packet has padding data, but padding data len illegal.", 				"packet len=%u padding data len=%u", m_ulPacketLen, pRtpData[ulLen - 1]);
		}
    }

    m_ulFixedHeadLen = ulHeadLen;
    
    // �ж��Ƿ�����չͷ
    if (1 != m_pFixedHead->extension)
    {
        // û����չͷ��ֱ�ӷ���
        m_ulHeadLen = ulHeadLen;
        return NRU_SUCCESS;
    }

    //m_ulFixedHeadLen = ulHeadLen;
    if (ulLen < ulHeadLen + sizeof(RTP_EXTEND_HEADER))
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "Rtp packet parse fail packet len is too short to contain extend head."," ");
        return NRU_FAIL;
    }

    // ָ����չͷ
    m_pExtHead = (RTP_EXTEND_HEADER*)(void*)(m_pRtpData + ulHeadLen);

    // ����չͷ���жϰ��ĳ����Ƿ��㹻������չͷ
    ulHeadLen += sizeof(RTP_EXTEND_HEADER) + ACE_NTOHS(m_pExtHead->usLength) * RTP_EXTEND_PROFILE_LEN;
    if (ulLen < ulHeadLen)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "Rtp packet parse fail packet len is too short."," ");
        return NRU_FAIL;
    }

    if (m_pExtHead->usProfile == ACE_HTONS(0x4857))
    {     
        m_pExtData = (RTP_EXTENSION_DATA_S*)(void*)(m_pRtpData + m_ulFixedHeadLen + sizeof(RTP_EXTEND_HEADER));

        //������չͷ�����ж��Ƿ�ǰ�˴�����rtp��չͷ���Լ��Ƿ��в�����Ϣ�ֶ�.swx164794 add. 2013-03-09
        if (sizeof(RTP_EXTENSION_DATA_S_EX) == ACE_NTOHS(m_pExtHead->usLength)* RTP_EXTEND_PROFILE_LEN)
        {
            m_pExtDataHaveFillIn = (RTP_EXTENSION_DATA_S_EX*)(void*)(m_pRtpData + m_ulFixedHeadLen + sizeof(RTP_EXTEND_HEADER));
        }
        else if (sizeof(RTP_EXTENSION_DATA_MU_S) == ACE_NTOHS(m_pExtHead->usLength) * RTP_EXTEND_PROFILE_LEN)
        {
            m_pExtDataHaveFillIn = (RTP_EXTENSION_DATA_S_EX*)(void*)(m_pRtpData + m_ulFixedHeadLen + sizeof(RTP_EXTEND_HEADER));
            m_pMuExtData = (RTP_EXTENSION_DATA_MU_S*)(void*)(m_pRtpData + m_ulFixedHeadLen + sizeof(RTP_EXTEND_HEADER));
        }    
    }

    // ���ˣ����е�ͷ�����Ѿ����������
    m_ulHeadLen = ulHeadLen;

    return NRU_SUCCESS;
}

int CRtpPacket::GeneratePacket(char* pRtpPacket, unsigned int ulLen)
{
    if (NULL == pRtpPacket)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GeneratePacket","packet is null.");
        return NRU_FAIL;
    }

    memset(pRtpPacket, 0x0, ulLen);
    m_pRtpData = pRtpPacket;
    m_ulPacketLen = ulLen;

    // ����������һ��Ĭ��ֵ����㲻��ı�Ĭ��ֵ�Ŀ��Բ���������
    m_pFixedHead = (RTP_FIXED_HEADER*)(void*)m_pRtpData;

    m_pFixedHead->version = RTP_PACKET_VERSION;
    m_pFixedHead->marker  = 0;
    m_pFixedHead->payload = 96;
    m_pFixedHead->extension = 0;

    m_ulHeadLen = sizeof(RTP_FIXED_HEADER);

    // �����ݰ����ͣ��ٵ��þ�������ú�������������

    return NRU_SUCCESS;
}

// ������չ�ֶ� y00196893 add
int CRtpPacket::GenerateExtensionPacket(char* pRtpPacket,  unsigned int ulLen)
{
    if ((NULL == pRtpPacket) || (sizeof(RTP_EXTEND_HEADER) > ulLen))
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GeneratePacket","Rtp packet generate packet fail packet is null.");
        return NRU_FAIL;
    }

    memset(pRtpPacket, 0x0, ulLen);
    m_pRtpData = pRtpPacket;

    m_ulPacketLen += ulLen;

    // ����������һ��Ĭ��ֵ����㲻��ı�Ĭ��ֵ�Ŀ��Բ���������
    m_pExtHead = (RTP_EXTEND_HEADER*)(void*)m_pRtpData;

    m_pExtHead->usProfile = ACE_HTONS(0x4857);//RTP_WARTERMARK_TYPE;
    m_pExtHead->usLength = ACE_HTONS((ulLen - sizeof(RTP_EXTEND_HEADER)) / RTP_EXTEND_PROFILE_LEN);

    m_pExtData = (RTP_EXTENSION_DATA_S*)(void*)(pRtpPacket + sizeof(RTP_EXTEND_HEADER));    
    //�˽ṹ��m_pExtData���Զ��ȡһ���ֶ���Ϣ.������ͷ��ʹ��m_pExtData��չͷ�ṹ���ʱ���
    m_pExtDataHaveFillIn = (RTP_EXTENSION_DATA_S_EX*)(void*)(pRtpPacket + sizeof(RTP_EXTEND_HEADER));
    
    m_pExtDataHaveFillIn->ulEncryptExLen = 0; //��Ĭ��ֵ.Ĭ�ϲ��볤��Ϊ0    

    //�����չ����������ƽ̨��չ����һ�£���ָ����չ����
    if(sizeof(RTP_EXTEND_HEADER) + sizeof(RTP_EXTENSION_DATA_MU_S) == ulLen)
    {
        m_pMuExtData = (RTP_EXTENSION_DATA_MU_S*)(void*)(pRtpPacket + sizeof(RTP_EXTEND_HEADER));
        m_pMuExtData->ulEncryptExLen = 0; //��Ĭ��ֵ.Ĭ�ϲ��볤��Ϊ0
    }

    m_ulHeadLen += ulLen;
    
    return NRU_SUCCESS;
}

int CRtpPacket::GetVersion(char& cVersion)const
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GetVersion","Rtp packet get version fail fixed head is null.");
        return NRU_FAIL;
    }

    cVersion = m_pFixedHead->version;

    return NRU_SUCCESS;
}

int CRtpPacket::CheckVersion()const
{
    char cVersion = 0;
    if (NRU_SUCCESS != GetVersion(cVersion))
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::CheckVersion","Rtp packet check version fail get packet version fail.");
        return NRU_FAIL;
    }

    if (RTP_PACKET_VERSION != cVersion)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::CheckVersion","Rtp packet check version fail version =%d is invalid.",cVersion);
        return NRU_FAIL;
    }

    return NRU_SUCCESS;
}

unsigned short CRtpPacket::GetSeqNum()const
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GetSeqNum","Rtp packet get seqnum fail packet is null.");
        return 0;
    }

    return ACE_NTOHS(m_pFixedHead->seq_no);
}

unsigned int  CRtpPacket::GetTimeStamp()const
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GetTimeStamp","Rtp packet get timestamp fail packet is null.");
        return 0;
    }

    return ACE_NTOHL(m_pFixedHead->timestamp);
}

char  CRtpPacket::GetPayloadType()const
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GetPayloadType","Rtp packet get payload type fail packet is null.");
        return 0;
    }

    return m_pFixedHead->payload;
}

bool CRtpPacket::GetMarker()const
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GetMarker","Rtp packet get marker fail packet is null.");
        return false;
    }

    return (bool)m_pFixedHead->marker;
}

// ��ȡExtension�ֶ� y00196893 add
char CRtpPacket::GetExtension()const
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::GetExtension","Rtp packet get marker fail packet is null.");
        return 0;
    }
    return m_pFixedHead->extension;
}
unsigned int CRtpPacket::GetHeadLen()const
{
    return m_ulHeadLen;
}

unsigned int CRtpPacket::GetPacketLen()const
{
    return m_ulPacketLen;
}

unsigned int CRtpPacket::GetFixedHeadLen()const
{
    return m_ulFixedHeadLen;
}

int CRtpPacket::SetVersion(unsigned char ucVersion)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetVersion","Rtp packet set version fail packet is null.");
        return NRU_FAIL;
    }

    m_pFixedHead->version = ucVersion;

    return NRU_SUCCESS;
}

int CRtpPacket::SetSeqNum(unsigned short usSeqNum)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetSeqNum","Rtp packet set seqnum fail packet is null.");
        return NRU_FAIL;
    }

    m_pFixedHead->seq_no = ACE_HTONS(usSeqNum);

    return NRU_SUCCESS;
}

int CRtpPacket::SetTimeStamp(unsigned int ulTimeStamp)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetTimeStamp","Rtp packet set timestamp fail packet is null.");
        return NRU_FAIL;
    }

    m_pFixedHead->timestamp = ACE_HTONL(ulTimeStamp);

    return NRU_SUCCESS;
}

int CRtpPacket::SetPayloadType(unsigned char ucPayloadType)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetPayloadType","Rtp packet set payload type fail packet is null.");
        return NRU_FAIL;
    }

    m_pFixedHead->payload = ucPayloadType;

    return NRU_SUCCESS;
}

int CRtpPacket::SetMarker(bool bMarker)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetMarker","Rtp packet set marker fail packet is null.");
        return NRU_FAIL;
    }

    m_pFixedHead->marker = bMarker;

    return NRU_SUCCESS;
}

// ����Extension�ֶ� y00196893 add
int CRtpPacket::SetExtension(bool bExtension)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetExtension","Rtp packet set extension fail packet is null.");
        return NRU_FAIL;
    }

    m_pFixedHead->extension = bExtension;

    return NRU_SUCCESS;
}

// ��ȡ"����16�ֽڶ���Ķ���������"��Ϣ.
unsigned int CRtpPacket::GetEncryptExLen() const
{    
    if (NULL == m_pExtDataHaveFillIn)
    {
        return 0;
    }
    
    if (NULL != m_pExtDataHaveFillIn) //lint !e774
    {        
        return m_pExtDataHaveFillIn->ulEncryptExLen;
    }

	return 0;
}

void CRtpPacket::SetSSRC(unsigned int unSsrc)
{
    if (NULL == m_pFixedHead)
    {
        //BP_RUN_LOG_ERR(UNDEF_ERRCODE, "CRtpPacket::SetSSRC","SetTimeStamp fail packet is null.");
        return;
    }

    m_pFixedHead->ssrc = unSsrc;
    return;
}

RTP_EXTENSION_DATA_S* CRtpPacket::GetExtData() const
{
    return m_pExtData;
}

RTP_EXTENSION_DATA_S_EX* CRtpPacket::GetExtHaveFillInData() const
{
    return m_pExtDataHaveFillIn;
}

RTP_EXTENSION_DATA_MU_S* CRtpPacket::GetMuExtData() const
{
    return m_pMuExtData;
}

char* CRtpPacket::GetRtpData() const
{
    return m_pRtpData;
}

RTP_FIXED_HEADER* CRtpPacket::GetFixedHead() const
{
    return m_pFixedHead;
}

//lint +e1763

