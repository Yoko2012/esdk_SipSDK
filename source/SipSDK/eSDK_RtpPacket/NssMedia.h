#ifndef __NSS_MEDIA_H__
#define __NSS_MEDIA_H__

//#include "nssH/nss_mt_old.h"
//#include "nssH/nss_adt.h"
//#include "nssH/rtvideo/stream_m.h"

#define SAFETY_INFO_LEN 16       // ��������Ϣ����

typedef unsigned long long uint64_nss;

#define MAX_PACKET_NUM_PER_FRAME        1024
#define WATER_MARK_LEN                  4
#define NSS_HEAD_EXTEND_LEN             4
#define FRAME_CACHE_NUM                 5
#define FRAME_ALLOC_NUM                 (FRAME_CACHE_NUM + 10)   // ����SPS��PPS��IDR

#pragma pack(push, 1)
/*
 * ֡��� FrameType
 * 1:��Ƶ֡��2:��Ƶ֡��3:NAT������Ϣ��4:NAT��Ӧ��Ϣ��5,6:�طſ��ƣ�7,8:EOS/BOS��Ϣ;����:����
 */
enum FRAME_TYPE
{
    FRAME_TYPE_VIDEO                    = 0x01, // ��Ƶ֡
    FRAME_TYPE_AUDIO                    = 0x02, // ��Ƶ֡
    FRAME_TYPE_NAT_REQ                  = 0x03, // NAT������Ϣ
    FRAME_TYPE_NAT_RESP                 = 0x04, // NAT��Ӧ��Ϣ
    FRAME_TYPE_PLAYBACK_CONTROL_REQ     = 0x05, // �طſ���������Ϣ
    FRAME_TYPE_PLAYBACK_CONTROL_RESP    = 0x06, // �طſ���Ӧ����Ϣ
    FRAME_TYPE_EOS                      = 0x07, // EOS��Ϣ
    FRAME_TYPE_BOS                      = 0x08, // BOS��Ϣ
    FRAME_TYPE_CLOSE                    = 0x09  // �쳣������Ϣ
};

/*
 * �Ƿ��ǹؼ�֡��־�� 
 * 0���ǹؼ�֡��1���ؼ�֡��2���ؼ�֡���н��������������������
 */
enum KEY_FRAME_FLAG
{
    KEY_FRAME_FLAG_COMMON               = 0x00, // P֡
    KEY_FRAME_FLAG_KEY                  = 0x01, // �ؼ�֡IDR
    KEY_FRAME_FLAG_SPS                  = 0x02, // �ؼ�֡SPS
    KEY_FRAME_FLAG_PPS                  = 0x03, // �ؼ�֡ PPS
    KEY_FRAME_FLAG_SEI_IFRAME           = 0x04, // SEI(I֡)
    KEY_FRAME_FLAG_SEI_PFRAME           = 0x05, // SEI(P֡)
    KEY_FRAME_FLAG_WATER_MARK           = 0x06  // ˮӡ֡

};

#define WATER_MARK_VALUE 0xF0
#define BIT_LOW_VALUE    0x0F
#define WATER_MARK_LABEL 0x0001

#define H264_START_CODE_LENGTH 0x04
#define H264_START_CODE 0x00000001
#define H264_START_CODE_SHORT_LENGTH 0x03
#define H264_START_CODE_SHORT 0x00000100
#define H264_START_CODE_VALUE 0xFFFFFF00

/*
 * ��Ƶ�����ʽ VIDEO_CODEC_TYPE
 */
enum VIDEO_CODEC_TYPE
{
    VIDEO_CODEC_TYPE_MIN = 0,
    CODEC_TYPE_H264      = 0x01,
    CODEC_TYPE_MJPEG     = 0x02,
    CODEC_TYPE_MPEG4     = 0x03,
    CODEC_TYPE_MPEG2     = 0x04,
    CODEC_TYPE_AVS       = 0x05,
    VIDEO_CODEC_TYPE_MAX
};

/*
 * ��Ƶ�����ʽ AUDIO_CODEC_TYPE
 */
enum AUDIO_CODEC_TYPE
{
    CODEC_TYPE_NULL      = 0x00, // PU��֧����Ƶ
    CODEC_TYPE_G711A     = 0x01, // G.711A
    CODEC_TYPE_G711U     = 0x02, // G.711U
    AUDIO_CODEC_TYPE_MAX
};

// ��չ֡ͷ
typedef struct _NSS_EXTEND_FRAME_HEADER
{
    unsigned long    FrameSize;      // ֡����,������չ����֡ͷ��֡ͷ��ý������
    unsigned long    AdjustFrameSize;// У��֡����, ������չ����֡ͷ��֡ͷ��ý�����ݡ�4K���벹������(����еĻ�) 
    unsigned char    ReserveData[4]; // �����ֽ�
}NSS_EXTEND_FRAME_HEADER;

// �ؼ�֡ʱ��Ҫ��֡ͷ�󸽼ӹؼ�֡��Ϣ
typedef struct _H264_KEY_FRAME_HEADER
{
    unsigned long    SPSOffset;
    unsigned long    PPSOffset;
    unsigned long    SEIOffset;
    unsigned long    IDROffset;
}H264_KEY_FRAME_HEADER;

// ��������ͷ�ඨ��Ķ���������, �����Ǵ洢�ֲ���Ϣ�������
// ����ǹؼ�֡MULTI_LAYER_CODE_0�洢��0��,����洢��1��,MULTI_LAYER_CODE_1��MULTI_LAYER_CODE_2�ֱ�洢�ڵ�2��3��
typedef enum  MULTI_LAYER_CODE_TYPE
{
    MULTI_LAYER_CODE_0      = 0,   //����������0��������
    MULTI_LAYER_CODE_1      = 1,   //����������1����չ��
    MULTI_LAYER_CODE_2      = 2,   //����������2����ǿ��
    MULTI_LAYER_CODE_NULL   = 3,   //��ʹ�ö����룬�����ֲ�
    MULTI_LAYER_CODE_MAX
}MULTI_LAYER_CODE_TYPE_E;

typedef struct NSS_EXTEND_WATER
{
    unsigned long ulSize;  //�ṹ�峤��
    unsigned short usWaterMarkType;       //����ˮӡ���� //>>WATER_MARK_TYPE_E
    unsigned long  ulWaterMarkValue;      //����ˮӡֵ
}NSS_EXTEND_WATER_S;


// ����չ֡ͷ��֡ͷ��MRU�ô�¼���ļ���
/*typedef struct _FILE_MEDIA_FRAME_HEADER
{
    NSS_EXTEND_FRAME_HEADER ExtendFrameHeader;  // ��չ֡ͷ
    NSS_MEDIA_FRAME_HEADER  FrameHeader;        // ֡ͷ
}FILE_MEDIA_FRAME_HEADER;
*/

// NAT��Խ������Ϣ
/*typedef struct _NSS_MEDIA_NAT_REQ
{
    NSS_MEDIA_FRAME_HEADER FrameHeader;         // ֡ͷ��ֻ�� FrameType = FRAME_TYPE_NAT_REQ
    char            StreamID[ID_LEN];           // ��ID
    unsigned long   MsgReqSeq;                  // ������Ϣ���к�
    char            SafetyInfo[SAFETY_INFO_LEN];// ��֤��Ϣ��16�ֽ�
    unsigned long   LocalIP;                    // ý������NAT��Խǰ�ı���IP��ַ
    unsigned short  LocalPort;                  // ý������NAT��Խǰ�ı��ض˿�
}NSS_MEDIA_NAT_REQ;
*/
// NAT��Խ��Ӧ��Ϣ
/*
typedef struct _NSS_MEDIA_NAT_RESP
{
    NSS_MEDIA_FRAME_HEADER FrameHeader;        // ֡ͷ��ֻ�� FrameType = FRAME_TYPE_NAT_RESP
    uint64_nss      StreamID;                  // ��ID
    unsigned long   MsgReqSeq;                 // ������Ϣ���к�
    unsigned long   LocalIP;                   // ý������NAT��Խǰ�ı���IP��ַ
    unsigned short  LocalPort;                 // ý������NAT��Խǰ�ı��ض˿�
    unsigned long   SrcIP;                     // ý������NAT��Խ���IP��ַ
    unsigned short  SrcPort;                   // ý������NAT��Խ��Ķ˿�
}NSS_MEDIA_NAT_RESP;
*/

typedef enum
{
    H264_NALU_TYPE_UNDEFINED    =0,
    H264_NALU_TYPE_SILCE        =1,
    H264_NALU_TYPE_IDR          =5,
    H264_NALU_TYPE_SEI          =6,
    H264_NALU_TYPE_SPS          =7,
    H264_NALU_TYPE_PPS          =8,    
    H264_NALU_TYPE_AUD          =9,
    H264_NALU_TYPE_STAP_A        =24,
    H264_NALU_TYPE_STAP_B        =25,
    H264_NALU_TYPE_MTAP16        =26,
    H264_NALU_TYPE_MTAP24        =27,
    H264_NALU_TYPE_FU_A         =28,
    H264_NALU_TYPE_FU_B         =29,
    H264_NALU_TYPE_END
}H264_NALU_TYPE;//lint !e751

typedef struct
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char NRI:2;
    unsigned char F:1;
} NALU_HEADER; /**//* 1 BYTES */

typedef struct 
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char NRI:2; 
    unsigned char F:1;    
} FU_INDICATOR; /**//* 1 BYTES */

typedef struct
{
    //byte 0
    unsigned char TYPE:5;
    unsigned char R:1;
    unsigned char E:1;
    unsigned char S:1;    
} FU_HEADER; /**//* 1 BYTES */

#pragma pack(pop)

// ƴ֡����������Ļص�����
typedef   int (*fAfterCombinFrameCallBack) (void* pUser, 
                                            ACE_Message_Block **pMbArray, 
                                            unsigned int MsgCount, unsigned short usLayerNo);
#endif /*__NSS_MEDIA_H__*/


