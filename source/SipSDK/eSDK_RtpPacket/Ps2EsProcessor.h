/********************************************************************
 filename    :    Ps2Es.h
 author      :    
 created     :    
 description :    ʵ��һ��PS��ת����ES���Ĵ�����
 copyright   :    
 history     :    
*********************************************************************/
#ifndef _PS_TO_ES_ 
#define _PS_TO_ES_ 

#include "RtpFrameOrganizer.h"
#include "HstPESParse.h"
#include "MediaBlockBuffer.h"
#include "RtpPacket.h"
#include "Lock.h"

class CPs2EsProcessor : public CPacketChangeProcessor, public IRtpFrameHandler
{
public:
    CPs2EsProcessor();

    ~CPs2EsProcessor();

    int  init(std::string strServiceID);

	int setIsNeedAudioFrame(bool isNeedAudioFrame);

    /*****************************************************************************
     Function:    receviePSPacket
     Description: ����һ��PSý������
     Input:       pMb ý�������ݿ�
     Output:      
     Return:      N/A
    *****************************************************************************/
    void receviePacket(const ACE_Message_Block* pMb);

    /*****************************************************************************
     Function:    handleMediaFrame
     Description: ƴ��һ֡PS���ݺ���ý������
     Input:       rtpFrameList PS���ݰ��б�
     Output:      
     Return:      N/A
    *****************************************************************************/
    void handleMediaFrame(RTP_FRAME_LIST_T &rtpFrameList);


private:
    /*****************************************************************************
     Function:    parseSingleNalu
     Description: 
     Input:       rtpFrameList ES���ݰ��б�
     Output:      
     Return:      N/A
    *****************************************************************************/
    int parseSingleNalu(char *&pReadPoint, char *&pCopyPoint, int &iLeftLength, unsigned char &ucType);

    /*****************************************************************************
     Function:    checkStartCode
     Description: У�����ݰ���ʼ��
     Input:       
     Output:      
     Return:      true �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    bool checkStartCode(char *pReadPoint, unsigned int &uiStartCodeLength);

    /*****************************************************************************
     Function:    HSPspkt2ESFrm
     Description: ƴ��һ֡PS���ݺ���ý������
     Input:       pPacket PS֡���ݻ���
                  iPktLen PS֡���ݳ���
     Output:      pFrame ת�������ݴ洢����
                  iVideoLen ��Ƶ����
                  iAudioLen ��Ƶ����
     Return:      IVS_SUCCEED �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    int  HSPspkt2ESFrm(unsigned char *pPacket, int iPktLen, unsigned char *&pFrame,
        int &iVideoLen, int &iAudioLen);

    /*****************************************************************************
     Function:    sendAudioFrame
     Description: ����ES��Ƶ֡
     Input:       pVideobuff ES֡���ݻ���
                  iVideobuffLen ES֡���ݳ���
     Output:      
     Return:      IVS_SUCCEED �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    int  sendAudioFrame(char *pAudiobuff, int iAudiobuffLen);

    /*****************************************************************************
     Function:    sendVideoFrame
     Description: ����ES��Ƶ֡
     Input:       pVideobuff ES֡���ݻ���
                  iVideobuffLen ES֡���ݳ���
     Output:      
     Return:      IVS_SUCCEED �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    int  sendVideoFrame(char *pVideobuff, int iVideobuffLen);

    /*****************************************************************************
     Function:    parseH264frame
     Description: ����H264֡
     Input:       pVideobuff ES֡���ݻ���
                  iBuffLen ES֡���ݳ���
     Output:      
     Return:      IVS_SUCCEED �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    int parseH264frame(char *pVideobuff, int iBuffLen);

    /*****************************************************************************
     Function:    cutSliceToRtp
     Description: ��һ��slice��ֳ�Rtp��
     Input:       pVideobuff ES֡���ݻ���
                  iBuffLen ES֡���ݳ���
     Output:      
     Return:      IVS_SUCCEED �ɹ�
                  ���� ʧ��
    *****************************************************************************/
    int cutSliceToRtp(char *pVideobuff, int iBuffLen);

    /*****************************************************************************
     Function:    setRtpInfo
     Description: ��һ��slice��ֳ�Rtp��
     Input:       pRtpHead RTPͷ
                  ucFrameType ֡����
                  bIsMarker mark��־�Ƿ���Ч
     Output:      
     Return:      
    *****************************************************************************/
    void setRtpInfo(char *pRtpHead, unsigned char ucFrameType, bool bIsMarker);

    /*****************************************************************************
     Function:    sendEsRtpPacket
     Description: 
     Input:       pRtpData  ������ES�����һ������
                  unDataSize ES����һ�����ݵĳ���
     Output:      
     Return:      N/A
    *****************************************************************************/
    void sendEsRtpPacket(const char* pRtpData, unsigned int unDataSize);

    void ReleasePktList();

    void ReleaseFrameList();
private:
    void appendExtInfo(ACE_Message_Block *pMsg);

private:
    std::string             m_strServiceID;         // ҵ��ID����Ҫ���ڴ�ӡ��־
    
    CRtpFrameOrganizer      m_RtpFrameOrganizer;    // ��֡����
    CHstPESParse*           m_pHstPESParse;         // PSת����ES������ע���ڴ��ͷ�

    char*                   m_pRtpFrameCache;       // �洢һ֡���ݵĻ��棬ע���ڴ��ͷ�
    char*                   m_pWritePos;            // ����m_pRtpFrameCache�ڴ�ָ��

    ACE_Recursive_Thread_Mutex       m_listMutex;             //RTSP�ỰMAP����
    std::list <ACE_Message_Block*>    m_curPacketList;
    std::list <ACE_Message_Block*>    m_curFrameList;  // ���list���ڽ���H264��ʹ��

    ACE_Message_Block*      m_pExtendHeader;                

    // RTP��ý������Ϣ
    unsigned long           m_ulVideoTimeTick;
    unsigned long           m_ulAudioTimeTick;
    unsigned short          m_usVideoRtpSeq;        
    unsigned short          m_usAudioRtpSeq;        
    unsigned char           m_ucVideoPayloadType;
    
    bool                    m_bIsFirstTime;

	bool                    m_bIsNeedAudioFrame;   // �Ƿ������Ƶ֡


    suseconds_t             m_usecLastSendTime;
    time_t                  m_tmLastSendTime;
    unsigned int            m_uiRealRecordSecond;
    unsigned int            m_uiRealRecordMSecond;
    unsigned int            m_uiReserved;

	CIVSMutex				m_Mutex;
};

#endif //_PS_TO_ES_

