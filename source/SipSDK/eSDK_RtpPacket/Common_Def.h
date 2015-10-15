/*******************************************************************
Copyright (C), 2008-2009, Huawei Symantec Tech. Co., Ltd.
File name:    Store_Signal.cpp
Author: ʯ���� ID 00003596     Version:  1.0        Date: 2009-09-15
Description:   ����ͨ�õĺ�: ����ֵ��.

Others:        
Function List:  
1. ....
History:
1. Date:      2009-09-19
Author:       ʯ����
ID:           00003596
Modification: create
2. ...
 ********************************************************************/
#ifndef _Common_Def_h
#define _Common_Def_h

enum NRU_CONST
{
    NRU_SUCCESS                = 0,   //���سɹ�
    NRU_FAIL                   = -1  //����ʧ��

    //NRU_CALLER_CAN_DELETE_MEM  = 0,   //�������ܹ��ͷŴ�����ڴ�
   // NRU_CALLER_CANT_DELETE_MEM = 1,   //�����߲��ܹ��ͷŴ�����ڴ�
    //NRU_TASK_ERROR_RETURN    = 0,   //Task�Ĵ��󷵻�ֵ

    //NRU_CANDELETE              = 1,   //�������ɾ��,���ü���ʱʹ��
    //NRU_CANNOTDELETE           = 0,   //���󲻿���ɾ�������ü���ʱʹ��

    //NRU_NOUPLOADFILE           = 1,   //û�ж���¼����Ҫ����

    //NRU_INVALID_INDEX_VALUE    = -1,  //��Ч������ֵ

    //NRU_LOGIN_AGAIN            = 1,   //��Ҫ�ٴ����ӵ�¼������

    //NRU_INVALID_BSM_SESSION_ID = -1,//BSM��Ч�ĻỰID

    //NRU_MAXUINT32              = 4294967295, //unsinged int �����ֵ

    //NRU_INVALID_FILE           = -1,  //��Ч���ļ�������

    //NRU_INFINITE_TIME          = -1  //���޳�ʱ��
};

//NRU��set_timer()�ຯ��: ���ö�ʱ��ʱ�ķ���ֵ
enum NRU_TIMER_RET
{
    NRU_TIMER_SET_SUCCESS      = 1,   //���ö�ʱ���ɹ�
    NRU_TIMER_SET_LOCK         = 0,   //��һ���߳���������, �Ѿ���ס�ò���
    NRU_TIMER_SET_FAIL         = -1  //���ö�ʱ��ʧ��
};


enum _enRTP_HANDLE_TYPE
{
    VIDEO_RTP_HANDLE,
    VIDEO_RTCP_HANDLE,
    AUDIO_RTP_HANDLE,
    AUDIO_RTCP_HANDLE,

    HANDLE_TYPE_MAX
};

enum _enRTP_TRACK_HANDLE_TYPE
{
    TRACK_RTP_HANDLE,
    TRACK_RTCP_HANDLE,

    TRACK_HANDLE_TYPE_MAX
};

typedef struct _REAL_RECORD_TIME
{
    unsigned int uiSecond;
    unsigned int uiMSecond;
}REAL_RECORD_TIME;

#define NRU_NULL_CHAR     '\0' //�ַ���������
#define NRU_ZERO          0    //��ֵ0
#define NRU_ONE           1    //��ֵ1
#define NRU_TWO           2    //��ֵ2
#define NRU_INVALID_PORT  0    //��Ч�˿�
#define NRU_KILO         (1024)

#define MAX_SQL_LEN 512

#define HIGH_WATER_M   ((500) * (1024) * (1024))     //¼��������е���󻺳���Ϊ500M
#define LOW_WATER_M    ((500) * (1024) * (1024))     //¼��������е���󻺳���Ϊ500M
#define DEFAULT_STACK_SIZE   ((128) * (1024))        //���߳�ջ��С128k

#define MEDIA_DATA_TIMEOUT       (120)                //��ȡý�����ݳ�ʱ 
#define MBU_MEDIA_DATA_TIMEOUT   (5)                  //��ȡý�����ݳ�ʱ 
#define CONTROL_DATA_TIMEOUT     (15)                 //������Ϣ����г�ʱ


#define DEFAULT_GOP_SIZE    25
#define MAX_GOP_SIZE    250

#define LOG_ERR_DISKFULL       500   /* ������         */
#define LOG_ERR_OPENFAIL       502   /* ���ļ�ʧ��    */
#define LOG_ERR_DBIMPORT       503   /* �������ݿ����   */
#define LOG_ERR_RENAMEFAIL     504   /* �������ļ�ʧ��  */
#define LOG_ERR_ZIPFAIL        505   /* ѹ���ļ�ʧ��    */
#define LOG_ERR_SFTPFAIL       506   /* ��־�ϴ�ʧ��    */

/* �쳣�ص�����---�ָ� */
#define LOG_RESUME_DISKFULL       550   /* �������ָ�         */
#define LOG_RESUME_OPENFAIL       552   /* ���ļ�ʧ�ָܻ�   */
#define LOG_RESUME_DBIMPORT       553   /* �������ݿ����ָ� */
#define LOG_RESUME_RENAMEFAIL     554   /* �������ļ�ʧ�ָܻ� */
#define LOG_RESUME_ZIPFAIL        555   /* ѹ���ļ�ʧ�ָܻ�   */
#define LOG_RESUME_SFTPFAIL       556   /* ��־�ϴ�ʧ�ָܻ�   */

const int DAYS_OF_WEEK = 7;   /* һ�ܵ�����   */

//socket��������С
#define SOCKET_BUFFER_2M       (2 * 1024 * 1024)
#define SOCKET_BUFFER_1M       (1024 * 1024)
#define SOCKET_BUFFER_512K     (512 * 1024)
#define SOCKET_BUFFER_256K     (256 * 1024)
#define SOCKET_BUFFER_128K     (128 * 1024)
#define SOCKET_BUFFER_32K      (32  * 1024)
#define SOCKET_BUFFER_8K       ( 8  * 1024)

#ifndef WIN32 
#define UNUSED_PARA(P) (void)P
#else
#define UNUSED_PARA(P) UNREFERENCED_PARAMETER(P)
#endif

//log����,��ֹƵ����ӡ
#define LOG_COUNT_THRESHOLD_1000    (1000)

#endif //_Common_Def_h

