/*
 * RtpPacket.h
 *
 *  Created on: 2010-12-28
 *      Author: x00106873
 */

#ifndef RTPPACKET_H_
#define RTPPACKET_H_

#include <map>

#define RTP_PACKET_VERSION      2
#define RTP_CSRC_LEN            4
#define RTP_EXTEND_PROFILE_LEN  4

// PayloadType����
#define IVS_PT_MJPEG         26
#define IVS_PT_MPEG4         98
#define IVS_PT_H264          99
#define IVS_PT_G711A          8
#define IVS_PT_G711U          0
#define IVS_PT_G723           4
#define IVS_PT_G722           9
#define IVS_PT_G726		     16
#define	IVS_PT_AAC 		     17
#define ELTE_PT_AMR          114

//NSSЭ�鶨����Ƶ����ֵ
#define NSS_AUDIO_G711A 0x0001
#define NSS_AUDIO_G711U 0x0002
#define NSS_AUDIO_G726  0x0003
#define NSS_AUDIO_AAC   0x0004

typedef struct
{
    /**//* byte 0 */
    unsigned char csrc_len:4;       /**//* expect 0 */
    unsigned char extension:1;      /**//* expect 1, see RTP_OP below */
    unsigned char padding:1;        /**//* expect 0 */
    unsigned char version:2;        /**//* expect 2 */
    /**//* byte 1 */
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
    unsigned char marker:1;         /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short seq_no;
    /**//* bytes 4-7 */
    unsigned int timestamp;
    /**//* bytes 8-11 */
    unsigned int ssrc;             /**//* stream number is used here. */
} RTP_FIXED_HEADER;

typedef struct
{
    unsigned short  usProfile;
    unsigned short  usLength;
}RTP_EXTEND_HEADER;

//����ˮӡ����
typedef enum  _enWATER_MARK_TYPE
{
    WATER_MARK   = 0,   //����ˮӡ
    WATER_MARK_NULL = 1,   //��ʹ������ˮӡ
    WATER_MARK_MAX
}WATER_MARK_TYPE_E;


//rtpǰ����չͷ.������ͷ��ʹ�ô���չͷ�ṹ���ʱ���
typedef struct _enRTP_EXTENSION_DATA
{
    unsigned short  usMultiLayerCodeType;  //���������� //>>MULTI_LAYER_CODE_TYPE_E
    unsigned short  usWaterMarkType;       //����ˮӡ���� //>>WATER_MARK_TYPE_E
    unsigned int    ulWaterMarkValue;      //����ˮӡֵ
}RTP_EXTENSION_DATA_S;

//��չͷ����һ���ֶ�"����16�ֽڶ���Ķ���������".swx164794 add/modify. 2013-03-06. begin
//rtpǰ����չͷ
typedef struct _stRTP_EXTENSION_DATA_AES_ALL_S
{
	unsigned short usMultiLayerCodeType;    //���������� //>>MULTI_LAYER_CODE_TYPE_E
	unsigned short usWaterMarkType;         //����ˮӡ���� //>>WATER_MARK_TYPE_E
	unsigned int   ulWaterMarkValue;        //����ˮӡֵ
	unsigned int   ulEncryptExLen;          //����16�ֽڶ���Ķ���������
}RTP_EXTENSION_DATA_S_EX;

//rtpƽ̨��չͷ,��ǰ�˶������ֶ�. MU���rtp�������в�����Ϣ�ֶ�
typedef struct _stRTP_EXTENSION_DATA_MU
{
    unsigned short  usMultiLayerCodeType;   //���������� //>>MULTI_LAYER_CODE_TYPE_E
    unsigned short  usWaterMarkType;        //����ˮӡ���� //>>WATER_MARK_TYPE_E
    unsigned int    ulWaterMarkValue;       //����ˮӡֵ
    unsigned int    ulEncryptExLen;         //����16�ֽڶ���Ķ���������
    unsigned short  usLostFrame;            //�Ƿ񶪰���0û�ж�����1��������ˮӡʱ��Ч
    unsigned short  usGmtMillsecond;        // ����ʱ����벿��
    unsigned int    uiGmtSecond;            // ����ʱ���벿��
}RTP_EXTENSION_DATA_MU_S;
//��չͷ����һ���ֶ�"����16�ֽڶ���Ķ���������".swx164794 add/modify. 2013-03-06. end

//ˮӡ֡֡��
typedef struct
{   
	unsigned short  usLostFrame;           //�Ƿ񶪰���0û�ж�����1��������ˮӡʱ��Ч
	unsigned short  usWaterMarkType;       //����ˮӡ���� //>>WATER_MARK_TYPE_E
	unsigned int    ulWaterMarkValue;      //����ˮӡֵ

}WarterMark_MEDIA_BODY;

class CRtpPacket
{
public:
    CRtpPacket();
    virtual ~CRtpPacket();

    int ParsePacket
    (
        char* pRtpData, 
        unsigned int ulLen
    );

    int GeneratePacket
    (
        char*           pRtpPacket, 
        unsigned int    ulLen
    );

    int GenerateExtensionPacket
    (
        char*           pRtpPacket, 
        unsigned int    ulLen
    );

    // ��ȡ����ֵ��������ʱ��
    unsigned short  GetSeqNum() const;
    unsigned int   GetTimeStamp() const;
    char            GetPayloadType() const;
    bool            GetMarker() const;
    char            GetExtension() const;       // �Ƿ���չ����    

    unsigned int    GetFixedHeadLen() const;
    unsigned int    GetHeadLen() const;
    unsigned short  GetPacketType() const;
    unsigned int    GetPacketLen() const;
	
	RTP_EXTENSION_DATA_S* GetExtData() const;
	RTP_EXTENSION_DATA_S_EX* GetExtHaveFillInData() const;
	RTP_EXTENSION_DATA_MU_S* GetMuExtData() const;
	char* GetRtpData() const;
	RTP_FIXED_HEADER* GetFixedHead() const;
	
    // ��������ֵ�����ʱ��
    int SetSeqNum(unsigned short usSeqNum);
    int SetTimeStamp(unsigned int ulTimeStamp);
    int SetPayloadType(unsigned char ucPayloadType);
    int SetMarker(bool bMarker);
    int SetExtension(bool bExtension);
    void SetSSRC(unsigned int unSsrc);

    // ��ȡ"����16�ֽڶ���Ķ���������"��Ϣ.
    unsigned int GetEncryptExLen() const;   

private:
    int GetVersion(char& cVersion)const;
    int CheckVersion()const;
    int SetVersion(unsigned char ucVersion);
    int ParsePacketType();
private:
    char*                   m_pRtpData;
    RTP_FIXED_HEADER*       m_pFixedHead;
    RTP_EXTEND_HEADER*      m_pExtHead;

    //��չͷ����һ���ֶ�"����16�ֽڶ���Ķ���������".swx164794 add. 2013-03-09
    RTP_EXTENSION_DATA_S_EX*       m_pExtDataHaveFillIn;   //��������Ϣ����չ����.
    
    RTP_EXTENSION_DATA_S*   m_pExtData;//��չ����    
    RTP_EXTENSION_DATA_MU_S*   m_pMuExtData;//mu�Զ�����չ����

    unsigned int            m_ulPacketLen;
    unsigned int            m_ulHeadLen;    // �������е�ͷ
    unsigned int            m_ulFixedHeadLen;  //�̶�ͷ����
};

#endif /* RTPPACKET_H_ */
