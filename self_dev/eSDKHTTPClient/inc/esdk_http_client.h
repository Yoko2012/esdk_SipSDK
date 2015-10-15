/********************************************************************
filename		: 	esdk_http_client.h
author			:	z00233055
created		:	2014/12/22	
description	:	定义 DLL 应用程序的导出函数
copyright		:	Copyright (C) 2014-2016
history			:	2014/12/22 初始版本
*********************************************************************/

#ifndef _ESDK_HTTP_CLIENT_H_
#define _ESDK_HTTP_CLIENT_H_

#include <list>
#include <string>

#ifdef WIN32
#ifdef  ESDKHTTPCLIENT_EXPORTS
#define eSDK_HTTP_API __declspec(dllexport)
#else
#define eSDK_HTTP_API __declspec(dllimport)
#endif
#else
#define eSDK_HTTP_API __attribute__((visibility("default")))
#endif

/*错误码*/
typedef enum 
{
	RET_OK = 0,
	RET_UNSUPPORTED_PROTOCOL,    /* 1 */
	RET_FAILED_INIT,             /* 2 */
	RET_URL_MALFORMAT,           /* 3 */
	RET_NOT_BUILT_IN,            /* 4 */
	RET_COULDNT_RESOLVE_PROXY,   /* 5 */
	RET_COULDNT_RESOLVE_HOST,    /* 6 */
	RET_COULDNT_CONNECT,         /* 7 */
	RET_FTP_WEIRD_SERVER_REPLY,  /* 8 */
	RET_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
									due to lack of access - when login fails
									this is not returned */
	RET_FTP_ACCEPT_FAILED,       /* 10 */
	RET_FTP_WEIRD_PASS_REPLY,    /* 11 */
	RET_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server */
	RET_FTP_WEIRD_PASV_REPLY,    /* 13 */
	RET_FTP_WEIRD_227_FORMAT,    /* 14 */
	RET_FTP_CANT_GET_HOST,       /* 15 */
	RET_OBSOLETE16,              /* 16 - NOT USED */
	RET_FTP_COULDNT_SET_TYPE,    /* 17 */
	RET_PARTIAL_FILE,            /* 18 */
	RET_FTP_COULDNT_RETR_FILE,   /* 19 */
	RET_OBSOLETE20,              /* 20 - NOT USED */
	RET_QUOTE_ERROR,             /* 21 - quote command failure */
	RET_HTTP_RETURNED_ERROR,     /* 22 */
	RET_WRITE_ERROR,             /* 23 */
	RET_OBSOLETE24,              /* 24 - NOT USED */
	RET_UPLOAD_FAILED,           /* 25 - failed upload "command" */
	RET_READ_ERROR,              /* 26 - couldn't open/read from file */
	RET_OUT_OF_MEMORY,           /* 27 */
	RET_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
	RET_OBSOLETE29,              /* 29 - NOT USED */
	RET_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
	RET_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
	RET_OBSOLETE32,              /* 32 - NOT USED */
	RET_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
	RET_HTTP_POST_ERROR,         /* 34 */
	RET_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
	RET_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
	RET_FILE_COULDNT_READ_FILE,  /* 37 */
	RET_LDAP_CANNOT_BIND,        /* 38 */
	RET_LDAP_SEARCH_FAILED,      /* 39 */
	RET_OBSOLETE40,              /* 40 - NOT USED */
	RET_FUNCTION_NOT_FOUND,      /* 41 */
	RET_ABORTED_BY_CALLBACK,     /* 42 */
	RET_BAD_FUNCTION_ARGUMENT,   /* 43 */
	RET_OBSOLETE44,              /* 44 - NOT USED */
	RET_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
	RET_OBSOLETE46,              /* 46 - NOT USED */
	RET_TOO_MANY_REDIRECTS ,     /* 47 - catch endless re-direct loops */
	RET_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
	RET_TELNET_OPTION_SYNTAX ,   /* 49 - Malformed telnet option */
	RET_OBSOLETE50,              /* 50 - NOT USED */
	RET_PEER_FAILED_VERIFICATION, /* 51 - peer's certificate or fingerprint
										wasn't verified fine */
	RET_GOT_NOTHING,             /* 52 - when this is a specific error */
	RET_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
	RET_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
									default */
	RET_SEND_ERROR,              /* 55 - failed sending network data */
	RET_RECV_ERROR,              /* 56 - failure in receiving network data */
	RET_OBSOLETE57,              /* 57 - NOT IN USE */
	RET_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
	RET_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
	RET_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
	RET_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
	RET_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
	RET_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
	RET_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
	RET_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
									that failed */
	RET_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
	RET_LOGIN_DENIED,            /* 67 - user, password or similar was not
									accepted and we failed to login */
	RET_TFTP_NOTFOUND,           /* 68 - file not found on server */
	RET_TFTP_PERM,               /* 69 - permission problem on server */
	RET_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
	RET_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
	RET_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
	RET_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
	RET_TFTP_NOSUCHUSER,         /* 74 - No such user */
	RET_CONV_FAILED,             /* 75 - conversion failed */
	RET_CONV_REQD,               /* 76 - caller must register conversion
									callbacks using curl_easy_setopt options
									CURLOPT_CONV_FROM_NETWORK_FUNCTION,
									CURLOPT_CONV_TO_NETWORK_FUNCTION, and
									CURLOPT_CONV_FROM_UTF8_FUNCTION */
	RET_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
									or wrong format */
	RET_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
	RET_SSH,                     /* 79 - error from the SSH layer, somewhat
									generic so the error message will be of
									interest when this has happened */

	RET_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
									connection */
	RET_AGAIN,                   /* 81 - socket is not ready for send/recv,
									wait till it's ready and try again (Added
									in 7.18.2) */
	RET_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
									wrong format (Added in 7.19.0) */
	RET_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
									7.19.0) */
	RET_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
	RET_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
	RET_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
	RET_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
	RET_CHUNK_FAILED,            /* 88 - chunk callback reported error */
	RET_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
									session will be queued */
	RET_LAST /* never use! */
}ERROR_CODE;


/*http版本*/
typedef enum 
{
	VERSION_NONE,
	VERSION_1_0,  // HTTP 1.0
	VERSION_1_1,  // HTTP 1.1
	VERSION_2_0   // HTTP 2.0
}HTTP_VERSION;


/*请求方式*/
typedef enum 
{
	MODE_GET,
	MODE_POST,
	MODE_PUT,
	MODE_DELETE
}REQUEST_MODE;


/*请求header*/
typedef std::list<std::string> REQUEST_HEADER;


/*请求参数*/
typedef struct 
{
	std::string strUrl;						//请求地址
	unsigned int uiTimeOut;			//超时时间 (50-5000ms)
	HTTP_VERSION uiVersion;		//HTTP版本
	REQUEST_MODE uiReqMode;	//请求方式
	std::string strReqData;			//请求消息体
	REQUEST_HEADER ReqHeader;//请求消息头
}REQUEST_OPTIONS;


/**************************************************************************
* description: 发送HTTP请求
* input			: ReqOptions 请求配置
* output		: strRspBody 响应消息体
					: strRspHeader 响应消息头	
* return		: 成功返回0，失败返回错误码
**************************************************************************/
eSDK_HTTP_API int __stdcall eSDK_HTTP_SendRequest(const REQUEST_OPTIONS &ReqOptions, std::string &strRspBody, std::string &strRspHeader);

#endif //_ESDK_HTTP_CLIENT_H_
