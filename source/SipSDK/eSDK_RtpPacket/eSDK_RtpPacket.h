#ifndef ESDK_RTP_PACKET_H_
#define ESDK_RTP_PACKET_H_
#ifdef ESDK_RTPPACKET_EXPORTS
#define ESDK_RTPPACKET  extern "C"  __declspec(dllexport)
#else
#define ESDK_RTPPACKET  extern "C"  __declspec(dllimport)
#endif

#ifdef WIN32
#define __SDK_CALL __stdcall
#else
#define __SDK_CALL
#endif

//�Զ�����������
typedef unsigned int	UINT32;
typedef int				INT32;

//�ӿڷ���ֵ
typedef enum _EM_RET_RESULT
{
	_RET_SUCCESS				=	0,	//�ɹ�
	_RET_FAILURE				=	1	//ʧ��
}EM_RET_RESULT;

//��������
typedef enum _EM_StreamType
{
	_PAY_LOAD_NO_TYPE = -1,
	_PAY_LOAD_TYPE_PCMU = 0,		// G711��u��
	_PAY_LOAD_TYPE_PCM = 1,
	_PAY_LOAD_TYPE_G723 = 4,		// G723
	_PAY_LOAD_TYPE_PCMA = 8,		// G711��a��
	_PAY_LOAD_TYPE_G722 = 9,		// G722

	_PAY_LOAD_TYPE_G726 = 16,	// G726
	_PAY_LOAD_TYPE_AAC = 17,		// AAC

	_PAY_LOAD_TYPE_MJPEG = 26,	// MJPEG
	_PAY_LOAD_TYPE_H264_1 = 96,	// H264(�Ǳ�׼����)
	_PAY_LOAD_TYPE_AMR_MB = 97,	// AMR_NB
	_PAY_LOAD_TYPE_MPEG4 = 98,	// MPEG4
	_PAY_LOAD_TYPE_H264 = 99,	// H264
	_PAY_LOAD_TYPE_AVS = 100,	// AVS
	_PAY_LOAD_TYPE_MP3 = 101,	// MP3
	_PAY_LOAD_TYPE_3GPP = 102,	// 3GPP
	_PAY_LOAD_TYPE_TRACK = 107  // �켣
}EM_StreamType;

// H264֡����
typedef enum _EM_H264NaluType
{
	_H264_NALU_TYPE_UNDEFINED = 0,
	_H264_NALU_TYPE_SLICE = 1,
	_H264_NALU_TYPE_DPA = 2,
	_H264_NALU_TYPE_DPB = 3,
	_H264_NALU_TYPE_DPC = 4,
	_H264_IDR_NALU_TYPE = 5,
	_H264_SEI_NALU_TYPE = 6,
	_H264_SPS_NALU_TYPE = 7,
	_H264_PPS_NALU_TYPE = 8,
	_H264_NALU_TYPE_AUD = 9,
	_H264_NALU_TYPE_EOSEQ = 10,
	_H264_NALU_TYPE_EOSTREAM = 11,
	_H264_NALU_TYPE_FILL = 12,

	_H264_NALU_TYPE_STAP_A = 24,
	_H264_NALU_TYPE_STAP_B = 25,
	_H264_NALU_TYPE_MTAP16 = 26,
	_H264_NALU_TYPE_MTAP24 = 27,
	_H264_NALU_TYPE_FU_A = 28,
	_H264_NALU_TYPE_FU_B = 29,
	_H264_NALU_TYPE_END = 30
}EM_H264NaluType;

//�ص���֡��������
typedef struct
{
	INT32 iStreamType;		//������,�ο�EM_StreamType
	INT32 iFrameDataType;	//֡����,�ο�EM_H264NaluType
}ST_FRAME_DATA;

//֡���ݻص���������
typedef void (__SDK_CALL * FrameDataCallBack)( void* pBuf, UINT32 uiBufSize,ST_FRAME_DATA* pFrameData,UINT32 uiChannel);

#ifdef __cplusplus
extern "C"
{
#endif
	/**
	 *��ʼ��
	 * 
	 *�ú������ڳ�ʼ��
	 *
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	���ȿ�ʼ���õĺ���
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_Init(void);

	/**
	 *��ͨ������ps����
	 * 
	 *�ú������ڴ�ͨ����������
	 *
	 *@param[in]	frameDataCallBack	�ص��ӿ�
	 *@param[in]	uiChannel			ͨ����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_OpenChannel(FrameDataCallBack frameDataCallBack,UINT32 uiChannel);

	/**
	 *��ͨ������es����
	 * 
	 *�ú������ڴ�ͨ����������
	 *
	 *@param[in]	frameDataCallBack	�ص��ӿ�
	 *@param[in]	uiChannel			ͨ����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_OpenESChannel(FrameDataCallBack frameDataCallBack,UINT32 uiChannel);

	/**
	 *��ͨ����������
	 * 
	 *�ú������ڴ�ͨ����������
	 *
	 *@param[in]	pBuf				���������ĵ�ַ
	 *@param[in]	uiBufSize			�������ݵĴ�С
	 *@param[in]	uiChannel			ͨ����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	�յ������ص�����ô˲���
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_ProcessPacket(char* pBuf, UINT32 uiBufSize,UINT32 uiChannel);  

	/**
	 *�����Ƿ���Ҫ��Ƶ֡�ص�
	 * 
	 *�ú������������Ƿ���Ҫ��Ƶ֡�ص�
	 *
	 *@param[in]	bIsNeedAudioFrame	ͨ���ţ�0��ʾ����Ҫ����0��ʾ��Ҫ
	 *@param[in]	uiChannel			ͨ����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_SetIsNeedAudioFrame(int bIsNeedAudioFrame,UINT32 uiChannel);

	/**
	 *�ر�ͨ����������
	 * 
	 *�ú������ڹر�ͨ����������
	 *
	 *@param[in]	uiChannel			ͨ����
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_CloseChannel(UINT32 uiChannel);

	/**
	 *ȥ��ʼ��
	 * 
	 *�ú�������ȥ��ʼ��
	 *
	 *@return		0	�ɹ�
	 *@return		��0	ʧ�ܣ��ο����󷵻��룩
	 *@attention	��
	 *@par			��
	**/
	ESDK_RTPPACKET INT32  __SDK_CALL ESDK_RTP_UnInit(void);

#ifdef __cplusplus
}
#endif 

#endif
