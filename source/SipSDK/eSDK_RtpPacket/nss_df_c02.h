#ifndef _NSS_DF_C02_HEADER_
#define _NSS_DF_C02_HEADER_

#include <vector>

#define	MAC_ADDR_LEN	      17

//lint -e830
#ifndef UNDEF_ERRCODE
#define UNDEF_ERRCODE 0
#endif

#define IVS_DATE_TIME_FORMAT  "YYYYMMDDThhmmssZ"  // ����&ʱ�䳤��

const unsigned int          ID_LEN                      = 32;                   // ͨ��ID
const unsigned int          TRANS_ID_LEN                = 64;                   // �����ų�
const unsigned int          RESERVE_ID_LEN              = 32;                   // Ԥ���ֶγ�
const unsigned int          DESCRIPTION_LEN             = 256;
const unsigned int          DEVICE_ID_LEN               = 32;                   // �豸ID ����
const unsigned int          SERVER_DEV_CODE_LEN         = 40;                   // ��Ԫ�豸code ���ȣ���MU#xxxx
const unsigned int          USER_ID_LEN                 = 40;                   // �û�ID  ��
const unsigned int          DOMAIN_CODE_LEN             = 32;                   // ��ID
const unsigned int          FILE_NAME_LEN               = 128;                  // �ļ�����
const unsigned int          IVS_DATE_TIME_LEN           = 20;                   // ʱ�����ڳ��� YYYYMMDDThhmmssZ
const unsigned int          IVS_TIME_LEN                = 15;                   // ʱ�䳤��
const unsigned int          HEARTBEAT_INTERVAL          = 10;                   // �������
const unsigned int          HEARTBEAT_TIMEOUT_TIMES     = 3;                    // �����������ʱ����
const unsigned int          IVS_NSS_URL_LEN             = 512;                  // URL ��
const unsigned int          IP_LEN                      = 32;                   // IP len
const unsigned int          SERVICE_ID_LEN              = 32;                   // ҵ��ID
const unsigned int			SESSION_ID_LEN			    = 32;					// ����ID����  
const unsigned int          IVS_SDP_LEN                 = 2048;                 // SDP��Ϣ����
const unsigned int          REC_FILE_NAME_LEN           = 64;                   // ¼���ļ���
const unsigned int          CAMERA_NAME_LEN             = 20;                   // ��������Ƴ���
const unsigned int          LABEL_ID_LEN                = 32;                   // ��ǩID����
const unsigned int          IVS_NVS_TIME_LEN            = 20;
const unsigned int          SSRC_CODE_LEN               = 32;                   // SSRC����


const unsigned int          NSS_DEV_CODE_LEN            = 32;                   // 
const unsigned int          LABEL_NAME_LEN              = 64;                   // ��ǩ���Ƴ���
const unsigned int          PRESET_ID_LEN               = 32;                   //Ԥ��λID
const unsigned int          PRESET_NAME_LEN             = 64;                   // Ԥ��λ���Ƴ���
const unsigned int          LOCK_ID_LEN                 = 32;                   // ����ID����
const unsigned int          LOCK_DESC_LEN               = 64;                   // ������������
const unsigned int          OPERATOR_NAME_LEN           = 32;                   // ����ID����

const unsigned int          SECURITY_KEY_PLAINTEXT_LEN  = 48;            		// ý�尲ȫ���ĳ���
const unsigned int          MEDIA_SECURITY_KEY_LEN      = 90;                   // ��Ƶ��ȫ�������ĳ���
const unsigned int          AUDIO_SECURITY_KEY_LEN      = 90;                   // ��Ƶ��ȫ�������ĳ���

const unsigned int          INT_MAX_LENGTH              = 10;                   // int����
	
const unsigned short        NSS_PROTOCOL_VERSION        = 0x0100;               // ����Э��汾

const unsigned short        RELIABLE_TRANSMISSION_LEN 	= 4;               		// �ɿ��������ͳ���

// tlv tag ����
enum _INVS_TLV_TAG
{
    TAG_XMLCONTENT                                      = 0x0001,               // ͨ��NSS����xml�ļ�ʱ��xml�����ֶε�Tagֵ
    TAG_USERID                                          = 0x0002,               // �û�ID
    TAG_ATTACHMENT                                      = 0x0003,               // �������ļ�
    TAG_ALARMDESC                                       = 0x0004,               // �澯������Ϣ
    TAG_EXTPARAM                                        = 0x0005,               // �澯��չ����
    TAG_OPERATEINFO                                     = 0x0006                // �澯ȷ��/����������Ϣ
        
};


typedef struct
_RTSP_OPTION_MSG_TIMESTAMP
{
	unsigned int                 TimeUTCSec;       // ��Ƶ��ӦUTCʱ�䣬��
    unsigned int                 TimeUTCMS;        // ��Ƶ��Ӧ��UTCʱ�䣬����
    unsigned int                 RTPTimeTick;      // ����Ƶ֡��Ӧ��RTPʱ���
}RTSP_OPTION_MSG_TIMESTAMP, *PRTSP_OPTION_MSG_TIMESTAMP;

//¼�����
typedef struct _RECORD_POLICY_PARA
{
    unsigned int    RecordMode;                            //�洢����,FULL-ȫ���洢,FRAME_EXTRACT-��֡�洢,MOVE_DECTION-�ƶ�������ܴ洢
    unsigned int    AllocMethod;                           //���䷽ʽ,TIME-������,CAPACITY-���� 
    unsigned int    Time;                                  //����,�����ڷ��䷽ʽʱ��ѡ
    unsigned int    FrameExtractRecordTime;                //��֡�洢����
    unsigned int    AlarmTime;                             //�и澯¼��洢����
    unsigned int    KeyframeTime;                          //�ؼ�֡����
	unsigned int    BeforeAlarm;                           //�ƶ���⾯ǰʱ��
	unsigned int    AfterAlarm;                            //�ƶ���⾯��ʱ��
	unsigned int    PlanStreamID;                          //�ƻ�¼��ʹ�õ���ID
	unsigned int    AlarmStreamID;                         //�澯¼��ʹ�õ���ID
	unsigned int    AlarmRecordTTL;                        //�澯¼��������
	unsigned int    ManualRecordTTL;                       //�ֶ�¼��������
	unsigned int    PreRecord;                         	   //Ԥ¼����
	unsigned int    PreRecordTime;                         //Ԥ¼ʱ��
	unsigned int    AssociatedAudio;                       //��·��Ƶ����ѡ��
}RECORD_POLICY_PARA;

//¼�����
typedef struct _RECORD_FAULT_UPDATE_PARA
{
	char 	lensId[DEVICE_ID_LEN];
	long 	currTime;
	long 	beginTime;
	long 	endTime;
	int 	update_flag;
    bool    bLoadStrateyFromDB;
}RECORD_FAULT_UPDATE_PARA;

//¼��ʱ��
typedef struct _RECORD_PLAN_SLICE
{
	long 	beginTime;
	long 	endTime;
}RECORD_PLAN_SLICE;

typedef struct _OMU_RECORD_MODULEINFO
{
    char devCode[DEVICE_ID_LEN + 1];
	unsigned int devType;
	char nodeCode[DOMAIN_CODE_LEN + 1];
	char devName[CAMERA_NAME_LEN + 1];
}OMU_RECORD_MODULEINFO;

// ¼�������ѯ��ʽ QueryType;
enum UPDATE_RECORD_FAULT_FLAG
{
    UPDATE_RECORD_FAULT_DELETE                  = 0x00,       // ���µ�ǰʱ����¼��ƻ�
    UPDATE_RECORD_FAULT_ADD                     = 0x01,       // ����¼��ƻ������¹�������
    UPDATE_RECORD_FAULT_MODIFY                  = 0x02,       // ɾ��������¼�񣬸��¹�������
    UPDATE_RECORD_FAULT_MAX
};

// ¼�������ѯ��ʽ QueryType;
enum QUERY_TYPE
{
    QUERY_TYPE_TIME              = 0x00,       // ��ʱ��
    QUERY_TYPE_BOOKMARK          = 0x01,       // ����ǩ
    QUERY_TYPE_PRESET            = 0x02,       // ��Ԥ��λ
    QUERY_TYPE_LOCK              = 0x03,       // ������״̬
    QUERY_TYPE_MAX
};

// ҵ������ ServiceType;
typedef enum
{
    IVS_SERVICE_TYPE_INVALID                = 0,
    IVS_SERVICE_TYPE_REALVIDEO              = 0x0001,       // ʵ��;
    IVS_SERVICE_TYPE_RECORD                 = 0x0002,       // ƽ̨¼��;
    IVS_SERVICE_TYPE_DOWNLOAD               = 0x0003,       // ƽ̨¼������;
    IVS_SERVICE_TYPE_PLAYBACK               = 0x0004,       // ƽ̨¼��ط�;
    IVS_SERVICE_TYPE_AUDIO_CALL             = 0x0005,       // ��Ƶ�Խ�;
    IVS_SERVICE_TYPE_AUDIO_BROADCAST        = 0x0006,       // ��Ƶ�㲥;
    IVS_SERVICE_TYPE_PU_DOWNLOAD	        = 0x0007,	    // ǰ��¼������;
    IVS_SERVICE_TYPE_PU_PLAYBACK            = 0x0008,	    // ǰ��¼��ط�;
    
    IVS_SERVICE_TYPE_REALVIDEO_BACKUP		= 0x0014,		// ʵ������; mbuʹ�ã�mu�յ���ת����ʵ������
    IVS_SERVICE_TYPE_PLATRECORD_BACKUP		= 0x0015,		// ƽ̨¼�񱸷�;mbuʹ�ã�mu�յ���ת����ƽ̨¼�����ش���
    IVS_SERVICE_TYPE_PURECORD_BACKUP		= 0x0016,		// ǰ��¼�񱸷�;mbuʹ�ã�mu�յ���ת����ǰ��¼�����ش���
    IVS_SERVICE_TYPE_BACKUPRECORD_PLAYBACK	= 0x0017,		// ¼�񱸷ݻط�; �յ���ת����ƽ̨¼��طŴ���
    IVS_SERVICE_TYPE_BACKUPRECORD_DOWNLOAD	= 0x0018,		// ¼�񱸷�����; �յ���ת����ƽ̨¼�����ش���
    
    IVS_SERVICE_TYPE_DISASTEBACKUP_PLAYBACK = 0x0019,       // ����¼��ط�
    IVS_SERVICE_TYPE_DISASTEBACKUP_DOWNLOAD = 0x001A,       // ����¼������
    IVS_SERVICE_TYPE_PU_UPLOAD              = 0x001B,       // ǰ�˻��油¼
    IVS_SERVICE_TYPE_PU_PRECORD				= 0x001C,		// ǰ��Ԥ¼
    
    IVS_SERVICE_TYPE_MAX
}NVS_SERVICE_TYPE;

/*
 * ý��ַ���ʽ TransType
 */
enum TRANS_TYPE
{
    TRANS_TYPE_INVALID      = -1,
    TRANS_TYPE_TRANS_MDU    = 0x0000,        // ͨ��MDU��ת
    TRANS_TYPE_DIRECT_CU    = 0x0001,        // ֱ��CU

    TRANS_TYPE_MAX
};

#if 0 // todo l00201416 ����Ҫɾ������ʹ��
enum PAY_LOAD_TYPE
{
    PAY_LOAD_TYPE_PCMU  = 0,    ///< G711��u��
    PAY_LOAD_TYPE_G723  = 4,    ///< G723
    PAY_LOAD_TYPE_PCMA  = 8,    ///< G711��a��
    PAY_LOAD_TYPE_G722  = 9,    ///< G722
    PAY_LOAD_TYPE_MJPEG = 24,   ///< MJPEG
    PAY_LOAD_TYPE_AMR_MB= 97,   ///< AMR_NB
    PAY_LOAD_TYPE_MPEG4 = 98,   ///< MPEG4
    PAY_LOAD_TYPE_H264  = 99,   ///< H264
    PAY_LOAD_TYPE_AVS   = 100,  ///< AVS
    PAY_LOAD_TYPE_MP3   = 101,  ///< MP3
    PAY_LOAD_TYPE_3GPP  = 102,  ///< 3GPP
};
#endif

/*
 * ���䷽��TRANS_DIRECTION
 */
enum TRANS_DIRECTION
{
    TRANS_DIRECTION_RECVONLY     = 0x1,     // ������
    TRANS_DIRECTION_SENDONLY     = 0x2,     // ������
    TRANS_DIRECTION_SENDRECV     = 0x3,     // �������ֽ���
    TRANS_DIRECTION_MAX
};

enum CONN_SETUP_TYPE
{
    CONN_SETUP_TYPE_ACTIVE       = 0x1,     // ������������
    CONN_SETUP_TYPE_PASSIVE      = 0x2,     // ������������
    CONN_SETUP_TYPE_MAX
};

/*
*��������־StreamReused
*/
enum STREAM_REUSED_FLAG
{
    STREAM_UNREUSED    = 0x0,       // ��������
    STREAM_REUSED      = 0x1      // ������
};

/*
 * ���紫��������� TransProtocol
 */

enum TRANS_PROTOCOL
{
    TRANS_PROTOCAL_MIN     = 0,
    TRANS_PROTOCAL_RTP_UDP = 0x01, // RTP over UDP
    TRANS_PROTOCAL_RTP_TCP = 0x02, // RTP over TCP
    TRANS_PROTOCAL_UDP     = 0x03, // UDP
    TRANS_PROTOCAL_TCP     = 0x04, // TCP
    TRANS_PROTOCAL_MAX
};

enum PACKET_PROTOCAL
{
    PACKET_PROTOCAL_MIN = 0,
    PACKET_PROTOCAL_ES  = 0x01, //ES
    PACKET_PROTOCAL_PS  = 0x02, //PS
    PACKET_PROTOCAL_TS  = 0x03, //TS
    PACKET_PROTOCAL_ES2PS = 0x04,
    PACKET_PROTOCAL_PS2ES = 0x05,
    PACKET_PROTOCAL_ES2TS = 0x06,
    PACKET_PROTOCAL_TS2ES = 0x07,
    PACKET_PROTOCAL_PS2TS = 0x08,
    PACKET_PROTOCAL_TS2PS = 0x09,
    PACKET_PROTOCAL_MAX
};

#define PACKET_PROTOCAL_NUMBER 3

/// MDU��Ϣ����ض���
enum _enMDU_MSG_BLOCK_DEF
{
    MDU_MSG_BLOCK_SIZE          = 2 * 1024,     /// ������Ϣ���С
    MDU_NSS_BLOCK_SIZE          = 1800,         /// ����һ��NSS����С
    MDU_BLOCK_NUM_PER_CHANNEL   = 200,          /// ÿ·��Ƶ�������Ϣ�����
    MDU_MAX_QUEUE_MSG_NUM       = 1 * 1024,     /// ������Ϣ���������Ϣ����
    MDU_MAX_QUEUE_BYTE_SIZE     = (MDU_MSG_BLOCK_SIZE * MDU_MAX_QUEUE_MSG_NUM)  /// ������Ϣ�������SIZE
};

/*
 * �Ƿ�֧��QOS SupportQoSFlag
 */
enum SUPPORT_QOS_FLAG
{
    SUPPORT_QOS_FLAG_TRUE    = 0x01, // ֧��
    SUPPORT_QOS_FLAG_FALSE   = 0x02, // ��֧��
    SUPPORT_QOS_FLAG_MAX
};

/*
 * ����ͷ����
 */
enum RECORD_CAMERA_TYPE
{
    RECORD_CAMERA_TYPE_TEYES        = 0,    // ǧ����Э�� 
    RECORD_CAMERA_TYPE_ONVIF        = 1,    //  OnvifЭ��
    RECORD_CAMERA_TYPE_HUAWEI       = 2,    // ��Ϊ����ͷ
    RECORD_CAMERA_TYPE_HKSDK        = 3,    // ����SDK
    RECORD_CAMERA_TYPE_MAX
};

/*
*ý����������
*/
enum MEDIA_DATA_TYPE
{
    VIDEO_RTP_PACKET             = 0, // ��ƵRTP��
    VIDEO_RTCP_PACKET            = 1, // ��ƵRTCP��
    AUDIO_RTP_PACKET             = 2, // ��ƵRTP��
    AUDIO_RTCP_PACKET            = 3  // ��ƵRTCP��
};

/*
 * ý������ MediaType
 */
enum NSS_MEDIA_TYPE
{
    MEDIA_TYPE_VIDEO_AUDIO = 0x01, // ����Ƶ����
    MEDIA_TYPE_VIDEO       = 0x02, // ��Ƶ����
    MEDIA_TYPE_AUDIO       = 0x03, // ��Ƶ����
    MEDIA_TYPE_ERROR       = 0x04  // ������ʾ����
};

/*
 * ��������
 * 0x01��
 * 0x02�����ˣ�PlayRateΪ�����ٶȣ�
 * 0x03�����ţ�PlayTimeΪ��ǰҪ���ŵĿ�ʼʱ��㣬���ͻ��˽�����������α����ڵ�ʱ��㣻
 * 0x04����ͣ��
 */
enum PLAYBACK_CTRL_CODE
{
    PLAYBACK_CTRL_CODE_MIN      = 0,
    PLAYBACK_CTRL_CODE_SPEED    = 0x01,     // �����PlayRateΪ����ٶȣ�
    PLAYBACK_CTRL_CODE_BACK     = 0x02,     // ���ˣ�PlayRateΪ�����ٶȣ�
    PLAYBACK_CTRL_CODE_DRAG     = 0x03,     // ���ţ�PlayTimeΪ��ǰҪ���ŵĿ�ʼʱ��㣻
    PLAYBACK_CTRL_CODE_PAUSE    = 0x04,     // ��ͣ��
    PLAYBACK_CTRL_GET_BASE_TIME = 0x20,     // ��ȡ���Ż�׼ʱ��
    PLAYBACK_CTRL_CODE_MAX
};

/************************************************************************/
/*                           ƽ̨���������Ͷ���                         */
/************************************************************************/
enum NSS_PLATFORM_SERVER_TYPE_CONST
{
    NSS_PLATFORM_SERVER_TYPE_NONE           =0x00,    /* ��Ч�ķ���������*/
    NSS_PLATFORM_SERVER_TYPE_SMU            =0x01,    /* SMU*/
    NSS_PLATFORM_SERVER_TYPE_SCU            =0x02,    /* SCU*/
    NSS_PLATFORM_SERVER_TYPE_CMU            =0x03,    /* CMU*/
    NSS_PLATFORM_SERVER_TYPE_OMU            =0x04,    /* OMU*/
    NSS_PLATFORM_SERVER_TYPE_MU             =0x05,    /* MU*/
    NSS_PLATFORM_SERVER_TYPE_DCG            =0x06,    /* DCG */
    NSS_PLATFORM_SERVER_TYPE_PCG            =0x07,    /* PCG*/
    NSS_PLATFORM_SERVER_TYPE_MAUS           =0x08,    /* MAUS*/
    NSS_PLATFORM_SERVER_TYPE_MTU            =0x09,    /* MTU*/
    
    //����MAXǰ���������ͣ���ֵ��֤����
    NSS_PLATFORM_SERVER_TYPE_MAX
};


#define NSS_NAME_MAP_TYPE \
{ \
    {NSS_PLATFORM_SERVER_TYPE_NONE,         "\0"        },   \
    {NSS_PLATFORM_SERVER_TYPE_SMU,          "NVS_SMU"   },   \
    {NSS_PLATFORM_SERVER_TYPE_SCU,          "IVS_SCU"   },   \
    {NSS_PLATFORM_SERVER_TYPE_CMU,          "NVS_CMU"   },   \
    {NSS_PLATFORM_SERVER_TYPE_OMU,          "IVS_OMU"   },   \
    {NSS_PLATFORM_SERVER_TYPE_MU,           "NVS_MU"    },   \
    {NSS_PLATFORM_SERVER_TYPE_DCG,          "NVS_DCG"   },   \
    {NSS_PLATFORM_SERVER_TYPE_PCG,          "NVS_PCG"   },   \
    {NSS_PLATFORM_SERVER_TYPE_MAUS,         "IVS_MAUS"  },   \
    {NSS_PLATFORM_SERVER_TYPE_MTU,          "NVS_MTU"   }    \
}


/// ������������״̬
typedef enum _enNETWORK_HANDLE_STATUS
{
    NETWORK_HANDLE_STATUS_INIT = 0,            /// ��ʼ״̬������մ���ʱ��״̬
    NETWORK_HANDLE_STATUS_OPEN,            /// �����Ѵ򿪣�ռ���˶˿ڣ�����ʱ�յ����ݻ����ӻ�ֱ�Ӷ���
    NETWORK_HANDLE_STATUS_START,           /// ��ʼ�������ݣ���������
    NETWORK_HANDLE_STATUS_DISCONNECT,      /// �������ݹ����������쳣��TCP���Ӳ��д�״̬
    NETWORK_HANDLE_STATUS_ABNORMAL         /// �����쳣
}NETWORK_HANDLE_STATUS;

/*
 * ¼������
 */
typedef enum _E_RECORD_TYPE
{
    RECORD_TYPE_NONE            = -1,    

    MANUAL_RECORD               = 0x00,     // �ֶ�¼��
    ALARM_RECORD               	= 0x01,     // �澯¼��
    PLAN_RECORD                 = 0x02,     // �ƻ�¼��
    PRE_RECORD                  = 0x03,     // ƽ̨Ԥ¼
    MEND_RECORD                 = 0x04,     // ��¼
    LOCK_RECORD                 = 0x05,     // ����¼��
    PRE_TO_RECORD               = 0x06,     // Ԥ¼ת¼��
    
    RECORD_TYPE_MAX
}E_RECORD_TYPE;

/*
 * ¼��״̬flags 
 */
enum RECORD_FLAGS
{
    RECORD_FLAG_NONE            = 0,    

    RECORD_FLAG_MANUAL          = 1,     // �ֶ�¼��
    RECORD_FLAG_ALARM          	= 2,     // �澯¼��
    RECORD_FLAG_PLAN            = 4,     // �ƻ�¼��
    RECORD_FLAG_PRE             = 8,     // ƽ̨Ԥ¼

    RECORD_FLAG_MASK            = 15
};

/*
 * ��������
 */
enum STREAM_ID_
{
    STREAM_ID_INVALID           = -1,  
    STREAM_ID_NOT_ASSIGNED      = 0,            // ��ָ������
    STREAM_ID_MAIN,                             // ������ 
    STREAM_ID_SUB1,                             // ������1
    STREAM_ID_SUB2,                             // ������2

    STREAM_ID_MAX               = 17
};

/*
 * ý�嵥�鲥����
 */
enum CAST_MODE_
{
    CAST_MODE_INVALID           = -1,
    CAST_MODE_UNICAST           = 0,        // ����
    CAST_MODE_MULTICAST         = 1,        // �鲥

    CAST_MODE_MAX
};

/*
 * ¼��ʽ
 */
typedef enum MU_RECORD_METHOD_
{
    MU_RECORD_METHOD_INVALID       = -1,
    MU_RECORD_METHOD_PLAT          = 0,        // ¼�������
    MU_RECORD_METHOD_PU            = 1,        // ǰ��
	MU_RECORD_METHOD_MBU           = 2,        // ���ݷ�����
	MU_RECORD_METHOD_RECOVERY	   = 3,		   // ���ַ�����
    MU_RECORD_METHOD_MAX
}MU_RECORD_METHOD;

// �ֶ�¼��״̬
typedef enum _MANUAL_REC_STATE
{
    MANUAL_REC_STATE_INVALID    = -1,        
    MANUAL_REC_STATE_END        = 0,        // ֹͣ
    MANUAL_REC_STATE_START      = 1,        // ����

    MANUAL_REC_STATE_MAX
}MANUAL_REC_STATE;

// �ֶ�¼��ֹͣԭ��
typedef enum _MANUAL_REC_STOP_REASON
{
    STOP_REASON_INVALID         = -1,
    STOP_REASON_MANUAL          = 0,        // �ֶ�ֹͣ
    STOP_REASON_TIMEOUT         = 1,        // ��ʱֹͣ
    STOP_REASON_UNEXCPETED      = 2,        // �쳣ֹͣ

    STOP_REASON_MAX
}MANUAL_REC_STOP_REASON;

// ��ѯ�ֶ�¼��������Ӧ�ľ�ͷ��Ϣ�ṹ
typedef struct _REC_TASK_CAMERAINFO
{    
    // char            DomainCode[DOMAIN_CODE_LEN];                // ��ϢĿ����
    char            CameraCode[DEVICE_ID_LEN];                  // ���������
    unsigned int    RecordMethod;                               // ¼��ʽ��0-ƽ̨¼��1-ǰ��¼��
}REC_TASK_CAMERAINFO;
typedef std::vector<REC_TASK_CAMERAINFO> CAMERA_LIST;

// ��ǵ�ǰ¼��״̬���
typedef struct _ST_RECORD_STATUS
{
	volatile E_RECORD_TYPE	record_type;					// ¼������
	volatile unsigned char	record_flag;					// ¼���ʶ
	volatile bool			recording_flag;					// д¼���ʶ
	volatile unsigned int	total_manual;					// �ܵ��ֶ�¼����
	volatile unsigned int	total_event;					// �ܸ澯¼����
	volatile unsigned int	total_plan;						// �ƻ���ֹʱ���ص�ʱʹ��
	char					alarm_type[ID_LEN];				// �澯���ͣ��澯¼��ʱ��Ч
	char					user_id[USER_ID_LEN];			// �û�ID���ֶ�¼��ʱ��Ч
	char					user_domain[DOMAIN_CODE_LEN];	// �û���ID, �ֶ�¼��ʱ��Ч
	int						tbl_event_id;					// �¼�¼����еĲ���ID
	bool					restart_flag;					// ����¼����	
	bool					prealarm_sync_flag;				// �澯Ԥ¼ͬ����ʶ
}ST_RECORD_STATUS;

// ��������  ��ʱ���壬SDK�ﶨ��
typedef enum
{
    HOME_DOMAIN           = 0x0000,
    EX_DOMAIN             = 0x0001
}DOMAINTYPE;

enum NVR_TYPE
{
	NSS_NVR_TYPE_INVALID	= -1,
    NSS_NORMAL_NVR_TYPE 	= 0,	//��ͨNVR
    NSS_AGENT_NVR_TYPE 		= 1,  	//����NVR
    NSS_MBU_NVR_TYPE 		= 2    	//���ݷ�����
};

typedef enum _enNET_HANDLE_TYPE
{
    NET_HANDLE_RTP,            /// RTP
    NET_HANDLE_RTCP,            /// RTCP

    HANDLE_RELEASE_SESSION 
}NET_HANDLE_TYPE;

// ���͡�������Ϣǰ��װ��ͷ��Ϣ
typedef struct _NSS_SOCKET_BIRTH
{
	long seconds_;
	long microseconds_;
}NSS_SOCKET_BIRTH;


#endif // _NSS_DF_C02_HEADER_

