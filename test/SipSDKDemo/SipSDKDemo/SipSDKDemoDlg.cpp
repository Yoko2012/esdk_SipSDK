
// SipSDKDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SipSDKDemo.h"
#include "SipSDKDemoDlg.h"
#include "afxdialogex.h"
#include "sipsdk.h"
#include "eSDK_RtpPacket.h"
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <stdlib.h>
#include "digcalc.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_REFREH_DEV (10000)

// 媒体流回调
typedef  void (__stdcall *NET_DATA_CALLBACK)(char    *pBuf,      // 回调数据;
	unsigned int  uiSize,     // 数据长度;
	void    *pUser);    // 用户数据;

typedef int (_stdcall * NETSOURCE_Init)(const char*);
static NETSOURCE_Init g_Init_Func = NULL;
typedef int (_stdcall * NETSOURCE_Getchannel)(unsigned long* );
static NETSOURCE_Getchannel g_Getchannel_Func = NULL;
typedef void (_stdcall *NETSOURCE_SetDataCallBack)(unsigned long,NET_DATA_CALLBACK,void*);
static NETSOURCE_SetDataCallBack g_SetDataCallBack_Func = NULL;
typedef int (__stdcall *NETSOURCE_SetProtocolType)(unsigned long , unsigned int);
static NETSOURCE_SetProtocolType g_SetProtocolType_Func = NULL;
typedef int (__stdcall *NETSOURCE_SetLocalRecvStreamIP)(unsigned long,char*);
static NETSOURCE_SetLocalRecvStreamIP g_SetLocalRecvStreamIP_Func = NULL;
typedef int (__stdcall *NETSOURCE_GetLocalRecvStreamPort)(unsigned long, unsigned int* ,unsigned int*);
static NETSOURCE_GetLocalRecvStreamPort g_GetLocalRecvStreamPort_Func = NULL;

typedef int (__stdcall *NETSOURCE_OpenNetStream)(unsigned long , char* ,unsigned int ,unsigned int);
static NETSOURCE_OpenNetStream g_OpenNetStream_Func = NULL;


typedef int (__stdcall *NETSOURCE_StartRecvStream)(unsigned long);
static NETSOURCE_StartRecvStream g_StartRecvStream_Func = NULL;

typedef int (__stdcall *NETSOURCE_CloseNetStream)(unsigned long ulChannel);
static NETSOURCE_CloseNetStream g_CloseNetStream_Func = NULL;

static 	std::vector<unsigned long> m_listChannel;
static std::vector<int> m_listResponseID;
static std::map<std::string,unsigned long> m_MapUserData;
static std::map<int,char*> m_userDataList;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSipSDKDemoDlg dialog

void __SDK_CALL FrameDataCB( void* pBuf, unsigned int uiBufSize,ST_FRAME_DATA* pFrameData,unsigned int uiChannel)
{
	//return;
	FILE* pFile = fopen("media.264","ab+");
	if(NULL == pFile)
	{
		return;
	}	
	if(pFrameData->iFrameDataType != 0)
	{
		char buf[4]={0,0,0,1};
		fwrite(buf,1,4,pFile);
	}
	fwrite(pBuf,1,uiBufSize,pFile);
	fclose(pFile);
	return;
}




CSipSDKDemoDlg::CSipSDKDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSipSDKDemoDlg::IDD, pParent),m_iResponseID(0),mbNetSourceLoad(false),mFreeChannel(0)
	, m_LocalIP(_T("172.25.0.175"))
	, m_RemoteIP(_T("172.22.9.51"))
	, m_LocalPort(_T("5062"))
	, m_RemotePort(_T("5061"))
	, m_SIPUserName(_T("31000000001180000012"))
	, m_SIPPwd(_T("1qw2!QW@"))
	, m_DevID(_T(""))
	, m_iSessionID(-1)
	, m_ServerID(_T("T28181"))
	, m_QueryID(_T(""))
	, m_LocalID(_T("31000000001180000012"))
	, m_RemoteSipName(_T("T89"))
	, m_RemoteSipPwd(_T("1qw2!QW@"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSipSDKDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_LocalIP);
	DDX_Text(pDX, IDC_EDIT2, m_RemoteIP);
	DDX_Text(pDX, IDC_EDIT3, m_LocalPort);
	DDX_Text(pDX, IDC_EDIT4, m_RemotePort);
	DDX_Text(pDX, IDC_EDIT5, m_SIPUserName);
	DDX_Text(pDX, IDC_EDIT6, m_SIPPwd);
	DDX_Text(pDX, IDC_EDIT7, m_DevID);
	DDX_Control(pDX, IDC_EDIT8, m_DevList);
	DDX_Text(pDX, IDC_EDIT_SERVERID, m_ServerID);
	DDX_Text(pDX, IDC_EDIT_QUERYID, m_QueryID);
	DDX_Text(pDX, IDC_EDIT_LOCALID, m_LocalID);
	DDX_Text(pDX, IDC_EDIT9, m_RemoteSipName);
	DDX_Text(pDX, IDC_EDIT11, m_RemoteSipPwd);
}

BEGIN_MESSAGE_MAP(CSipSDKDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSipSDKDemoDlg::OnBnClickedInit)
	ON_BN_CLICKED(IDC_BUTTON2, &CSipSDKDemoDlg::OnBnClickedSub)
	ON_BN_CLICKED(IDC_BUTTON3, &CSipSDKDemoDlg::OnBnClickedReg)
	ON_BN_CLICKED(IDC_BUTTON4, &CSipSDKDemoDlg::OnBnClickedInvite)
	ON_BN_CLICKED(IDC_BUTTON5, &CSipSDKDemoDlg::OnBnClickedMessage)
	ON_BN_CLICKED(IDC_BUTTON6, &CSipSDKDemoDlg::OnBnClickedUninit)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON7, &CSipSDKDemoDlg::OnBnClickedUnreg)
	ON_BN_CLICKED(IDC_BUTTON8, &CSipSDKDemoDlg::OnBnClickedBye)
	ON_WM_TIMER()
	ON_MESSAGE(WM_REFREH_DEV,&CSipSDKDemoDlg::OnDevRefresh)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON9, &CSipSDKDemoDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CSipSDKDemoDlg::OnBnClickedButton10)
END_MESSAGE_MAP()


// CSipSDKDemoDlg message handlers

BOOL CSipSDKDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here	

#ifdef _DEBUG
	//初始化netsource
	HMODULE pM = ::LoadLibrary("IVS_NetSource.dll");
#else
	HMODULE pM = ::LoadLibrary("IVS_NetSource.dll");
#endif

	
	if(NULL != pM)
	{
		g_Init_Func=(NETSOURCE_Init)GetProcAddress(pM,"_IVS_NETSOURCE_Init@4");//获取Dll的导出函数

		g_Getchannel_Func = (NETSOURCE_Getchannel)GetProcAddress(pM,"_IVS_NETSOURCE_GetChannel@4");
		g_SetDataCallBack_Func = (NETSOURCE_SetDataCallBack)GetProcAddress(pM,"_IVS_NETSOURCE_SetDataCallBack@12");
		g_SetProtocolType_Func = (NETSOURCE_SetProtocolType)GetProcAddress(pM,"_IVS_NETSOURCE_SetProtocolType@8");
		g_SetLocalRecvStreamIP_Func = (NETSOURCE_SetLocalRecvStreamIP)GetProcAddress(pM,"_IVS_NETSOURCE_SetLocalRecvStreamIP@8");
		g_GetLocalRecvStreamPort_Func = (NETSOURCE_GetLocalRecvStreamPort)GetProcAddress(pM,"_IVS_NETSOURCE_GetLocalRecvStreamPort@12");
		g_OpenNetStream_Func = (NETSOURCE_OpenNetStream)GetProcAddress(pM,"_IVS_NETSOURCE_OpenNetStream@16");
		g_StartRecvStream_Func = (NETSOURCE_StartRecvStream)GetProcAddress(pM,"_IVS_NETSOURCE_StartRecvStream@4");		
		g_CloseNetStream_Func = (NETSOURCE_CloseNetStream)GetProcAddress(pM,"_IVS_NETSOURCE_CloseNetStream@4");	
		mbNetSourceLoad = true;
	}

	/*
	char * pszNonce = "4aN98u2nkFzpVxJ9Slhaye96vKJKbjir";
	char * pszCNonce = "cf1e2ecb";
	char * pszUser = "T249";
	char * pszRealm = "huawei.com";
	char * pszPass = "1qw2!QW@";
	char * pszAlg = "MD5";
	char szNonceCount[9] = "00000002";
	char * pszMethod = "REGISTER";
	char * pszQop = "auth";
	char * pszURI = "sip:T249@10.170.103.60:5061";
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX Response;

	DigestCalcHA1(pszAlg, pszUser, pszRealm, pszPass, pszNonce,pszCNonce, HA1);
	DigestCalcResponse(HA1, pszNonce, szNonceCount, pszCNonce, pszQop,pszMethod, pszURI, HA2, Response);
	printf("Response = %s\n", Response);//dbd8717f896df56ff17af0fe16239b03
	*/
	char * pszAlg = "MD5";
	char * pszUser = "cgw";
	char * pszPass = "huawei123";
	char * pszNonce = "Sb1mA0GPoCyZOhLrBjDFhlkezSw8TIK7";
	char * pszCNonce = "";	
	char * pszRealm = "huawei.com";	
	char szNonceCount[9] = "";
	char * pszMethod = "REGISTER";
	char * pszURI = "sip:117.115.118.205:5062";
	char * pszQop = "";
	
	HASHHEX HA1;
	HASHHEX HA2 = "";
	HASHHEX Response;

	DigestCalcHA1(pszAlg, pszUser, pszRealm, pszPass, pszNonce,pszCNonce, HA1);
	DigestCalcResponse(HA1, pszNonce, szNonceCount, pszCNonce, pszQop,pszMethod, pszURI, HA2, Response);

	printf("Response = %s\n", Response);//4380db148d152a9861ab6f4fa09e2d23


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSipSDKDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSipSDKDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSipSDKDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
static HWND m_DlgHwnd = NULL;
static std::string strDevList = "";
int __stdcall CSipSDKDemoDlg::SIP_CallBack_FUNC(int msgType,int eventType,const char* pPara)
{
	std::cout << pPara <<endl;
	if(SIP_REQUEST_MSG == msgType)
	{
		if(SIP_REQ_MESSAGE == eventType)
		{
			if(NULL != m_DlgHwnd)
			{
				strDevList.append("\n");
				strDevList.append(pPara);
				::PostMessage(m_DlgHwnd,WM_REFREH_DEV,NULL,NULL);
			}
			
		}
	}
	return 0;

}
// 媒体流回调
void __stdcall netdatacb(char    *pBuf,      // 回调数据;
	unsigned int  uiSize,     // 数据长度;
	void    *pUser)    // 用户数据;
{
	char* usedata = (char*) pUser;
	std::string strGuid(usedata);
	if(m_MapUserData.find(strGuid) != m_MapUserData.end())
	{
		unsigned long ulChanelID = m_MapUserData.find(strGuid)->second;
		ESDK_RTP_ProcessPacket(pBuf,(int)uiSize,ulChanelID);
	}
	
	return;
}

void CSipSDKDemoDlg::OnBnClickedInit()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	//初始化netsource
	if(NULL != g_Init_Func)
	{
		g_Init_Func("D:\\log");
	}
	//初始化rtpconvert
	ESDK_RTP_Init();

	//init sipsdk
	std::string strName(m_SIPUserName.GetBuffer(m_SIPUserName.GetLength()));
	m_SIPUserName.ReleaseBuffer();
	std::string strPWD(m_SIPPwd.GetBuffer(m_SIPPwd.GetLength()));
	m_SIPPwd.ReleaseBuffer();
	std::string strLocalID(m_LocalID.GetBuffer(m_LocalID.GetLength()));
	m_LocalID.ReleaseBuffer();
	std::string strLocalIP(m_LocalIP.GetBuffer(m_LocalIP.GetLength()));
	m_LocalIP.ReleaseBuffer();
	std::string strRemoteIP(m_RemoteIP.GetBuffer(m_RemoteIP.GetLength()));
	m_RemoteIP.ReleaseBuffer();
	std::string strLocalPort(m_LocalPort.GetBuffer(m_LocalPort.GetLength()));
	m_LocalPort.ReleaseBuffer();
	int iLocalPort = atoi(strLocalPort.c_str());
	std::string strRemotePort(m_RemotePort.GetBuffer(m_RemotePort.GetLength()));
	m_RemotePort.ReleaseBuffer();
	int iRemotePort = atoi(strRemotePort.c_str());
	std::string strRemoteID(m_ServerID.GetBuffer(m_ServerID.GetLength()));m_ServerID.ReleaseBuffer();
	std::string strRemoteUser(m_RemoteSipName.GetBuffer(m_RemoteSipName.GetLength()));m_RemoteSipName.ReleaseBuffer();
	std::string strRemotePWD(m_RemoteSipPwd.GetBuffer(m_RemoteSipPwd.GetLength()));m_RemoteSipPwd.ReleaseBuffer();
	
	int iRet = SIP_SDK_Init(strName.c_str(),strPWD.c_str(),strLocalID.c_str(),strLocalIP.c_str(),iLocalPort,strRemoteUser.c_str(),strRemotePWD.c_str(),strRemoteID.c_str(),strRemoteIP.c_str(),iRemotePort,SIP_CallBack_FUNC);
}


void CSipSDKDemoDlg::OnBnClickedSub()
{
	// TODO: Add your control notification handler code here
	std::string xmlstr = "<?xml version='1.0' encoding='UTF-8'?>" ;
	xmlstr.append("<Query><CmdType>Catalog</CmdType><SN>2</SN><DeviceID>a87e6371cce9435baca4</DeviceID><StartTime>2014-03-01T00:00:00</StartTime><EndTime>2014-03-01T00:00:00</EndTime></Query>");
	int iRet = SIP_SDK_Subscribe("fjj@10.170.103.60:5061",xmlstr.c_str());
}


void CSipSDKDemoDlg::OnBnClickedReg()
{
	// TODO: Add your control notification handler code here
	int iRet = SIP_SDK_REGISTER();
	SetTimer(1000,30000,NULL);
}
void CSipSDKDemoDlg::OnBnClickedInvite()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	std::string strDev(m_DevID.GetBuffer(m_DevID.GetLength()));m_DevID.ReleaseBuffer();
	if(strDev.empty())
	{
		return;
	}
	unsigned int uiAudioPort = 0;
	unsigned int uiVideoPort = 0;
	int iRet;
	static int iuserdata = 1;
	char* strUserdata  =new char[10];
	memset(strUserdata,0,10);
	_itoa_s(iuserdata,strUserdata,10,10);
	std::string guid(strUserdata);
	if(NULL != g_Getchannel_Func)
	{
		g_Getchannel_Func(&mFreeChannel);
		if(NULL != g_SetDataCallBack_Func)
		{			
			g_SetDataCallBack_Func(mFreeChannel,netdatacb,strUserdata);
		}
		if(NULL != g_SetProtocolType_Func)
		{
			iRet = g_SetProtocolType_Func(mFreeChannel,1);//1-RTP over UDP , 3-UDP
		}
		if(NULL != g_SetLocalRecvStreamIP_Func)
		{
			iRet = g_SetLocalRecvStreamIP_Func(mFreeChannel,"10.170.54.134");
		}
		if(NULL != g_GetLocalRecvStreamPort_Func)
		{
			iRet = g_GetLocalRecvStreamPort_Func(mFreeChannel,&uiVideoPort,&uiAudioPort);
		}
	}
	
	std::string strBody = "v=0\r\n";
	strBody.append("o=");
	strBody.append(strDev);
	strBody.append(" 0 0 IN IP4 10.170.54.134\r\n");
	strBody.append("s=Play\r\n");
	strBody.append("c=IN IP4 10.170.54.134\r\n");
	strBody.append("t=0 0\r\n");
	strBody.append("m=video ");
	char buffer[10] = {0};
	_itoa_s(uiVideoPort,buffer,10,10);
	strBody.append(buffer);
	strBody.append(" RTP/AVP 96 98 97\r\n");
	strBody.append("a=recvonly\r\n");
	strBody.append("a=rtpmap:96 PS/90000\r\n");
	strBody.append("a=rtpmap:98 H264/90000\r\n");
	strBody.append("a=rtpmap:97 MPEG4/90000\r\n");
	char strRemoteBody[1024];
	iRet = SIP_SDK_INVITE(strDev.c_str(),strBody.c_str(),m_iResponseID,strRemoteBody);
	if(SIPSDK_RET_SUCCESS == iRet)
	{
		SIP_SDK_ACK(m_iResponseID);
		ESDK_RTP_OpenChannel(FrameDataCB,mFreeChannel);

		m_listChannel.push_back(mFreeChannel);
		m_listResponseID.push_back(m_iResponseID);
		m_MapUserData[guid] = mFreeChannel;
		m_userDataList[m_iResponseID] = strUserdata;
		
		std::string sstrRemoteSdp(strRemoteBody);

		//解析对方的ip
		char ip[20] = {0};
		unsigned int  start = sstrRemoteSdp.find("c=IN IP4 ");
		if(0 != start)
		{
			unsigned int end = sstrRemoteSdp.find("\r\n",start);
			std::string strIP = sstrRemoteSdp.substr(start+9,end-start-9);
			strcpy_s(ip,20,strIP.c_str());
		}

		//解析对方的端口
		char port[10] = {0};
		unsigned int  portstart = sstrRemoteSdp.find("m=video ");
		if(0 != portstart)
		{
			std::string strPort = sstrRemoteSdp.substr(portstart+8,5);
			strcpy_s(port,10,strPort.c_str());
		}


		unsigned long ulPort = atol(port);



		int iRet = -1;
		
		if(NULL != g_OpenNetStream_Func)
		{
			iRet = g_OpenNetStream_Func(mFreeChannel,ip,ulPort,ulPort+2);
		}
		if(NULL != g_StartRecvStream_Func)
		{
			if(0==iRet)
			{
				g_StartRecvStream_Func(mFreeChannel);
			}
		}
	}
}


void CSipSDKDemoDlg::OnBnClickedMessage()
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	strDevList = "";
	std::string strQueryID(m_QueryID.GetBuffer(m_QueryID.GetLength()));
	m_QueryID.ReleaseBuffer();

	std::string strDev(m_DevID.GetBuffer(m_DevID.GetLength()));m_DevID.ReleaseBuffer();
	if(strDev.empty())
	{
		return;
	}

	std::string strBody = "<?xml version='1.0' encoding='UTF-8'?><Query><CmdType>Catalog</CmdType><SN>2</SN><DeviceID>";
	strBody.append(strQueryID);
	strBody.append("</DeviceID></Query>");
	int iRet = SIP_SDK_MESSAGE(strDev.c_str(),strBody.c_str());

}


void CSipSDKDemoDlg::OnBnClickedUninit()
{
	// TODO: Add your control notification handler code here
	int iRet = SIP_SDK_UnInit();
	iRet = ESDK_RTP_UnInit();
	
}


void CSipSDKDemoDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CDialogEx::OnClose();
}


void CSipSDKDemoDlg::OnBnClickedUnreg()
{
	// TODO: Add your control notification handler code here
	int iRet = SIP_SDK_UNREGISTER();
	KillTimer(1000);
}


void CSipSDKDemoDlg::OnBnClickedBye()
{
	// TODO: Add your control notification handler code here
	for(unsigned int i=0;i<m_listResponseID.size();++i)
	{
		int resid = m_listResponseID.at(i);
		int iRet = SIP_SDK_BYE(resid);
		char* pUserData = m_userDataList[resid];
		delete[] pUserData;
	}
	m_listResponseID.clear();
	m_userDataList.clear();

	for(unsigned int i=0;i<m_listChannel.size();++i)
	{
		unsigned long channelid = m_listChannel.at(i);
		int iRet = ESDK_RTP_CloseChannel(channelid);
		if(NULL != g_CloseNetStream_Func)
		{
			g_CloseNetStream_Func(channelid);
		}
	}
	m_listChannel.clear();

}


void CSipSDKDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	UpdateData(TRUE);
	if(1000 == nIDEvent)
	{
		std::string strRemoteID(m_ServerID.GetBuffer(m_ServerID.GetLength()));
		m_ServerID.ReleaseBuffer();
		static int i = 1;
		std::string strBody = "<?xml version='1.0' encoding='UTF-8'?>\r";
		strBody.append("<Notify>\r\n<CmdType>Keepalive</CmdType>\r\n<SN>");
		char seq[10]={0};
		itoa(i,seq,10);
		strBody.append(seq);
		strBody.append("</SN>\r\n<DeviceID>");
		strBody.append(strRemoteID);
		strBody.append("</DeviceID>\r\n<Status>OK</Status>\r\n</Notify>\r\n");
		//std::string strBody = "<?xml version='1.0' encoding='UTF-8'?><Query><CmdType>Catalog</CmdType><SN>2</SN><DeviceID>a87e6371cce9435baca4</DeviceID></Query>";
		int iRet = SIP_SDK_MESSAGE(strRemoteID.c_str(),strBody.c_str());
		i++;
	}

	CDialogEx::OnTimer(nIDEvent);
}
LRESULT CSipSDKDemoDlg::OnDevRefresh(WPARAM wParam,LPARAM lParam)
{
	m_DevList.SetWindowText(strDevList.c_str());	
	return 0L;
}


int CSipSDKDemoDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	m_DlgHwnd = this->GetSafeHwnd();

	return 0;
}

IVS_VOID __SDK_CALL EventCB(IVS_INT32 iEventType, IVS_VOID* pEventBuf, IVS_UINT32 uiBufSize, IVS_VOID* pUserData)
{
	return;
}

void CSipSDKDemoDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here
	//初始化netsource
	if(NULL != g_Init_Func)
	{
		g_Init_Func("D:\\log");
	}
	//初始化rtpconvert
	ESDK_RTP_Init();

	IVS_INT32 iRet = 1;
	iRet = IVS_SDK_Init();
	if (IVS_SUCCEED == iRet)
	{
		AfxMessageBox(_T("Init SDK OK"));

		IVS_LOGIN_INFO LoginReqInfo = {0};
		LoginReqInfo.stIP.uiIPType = IP_V4;
		strncpy_s(LoginReqInfo.stIP.cIP, "10.170.103.60", IVS_IP_LEN);
		LoginReqInfo.uiPort = 9900;
		strncpy_s(LoginReqInfo.cUserName, "admin", IVS_NAME_LEN);
		strncpy_s(LoginReqInfo.pPWD, "1qw2!QW@", IVS_PWD_LEN);

		iRet = IVS_SDK_Login(&LoginReqInfo, &m_iSessionID);
		if (IVS_SUCCEED == iRet)
		{
			AfxMessageBox(("Login OK"));
			iRet = IVS_SDK_SetEventCallBack(m_iSessionID, (EventCallBack)EventCB, this);
		} 
		else
		{
			AfxMessageBox(("Login Fail"));
		}
	} 
	else
	{
		AfxMessageBox(("Init SDK Fail"));
	}

}

static int idata = 1;
void CSipSDKDemoDlg::OnBnClickedButton10()
{
	// TODO: Add your control notification handler code here
	IVS_INT32 iRet = IVS_FAIL;
	unsigned int uiAudioPort = 0;
	unsigned int uiVideoPort = 0;
	if(NULL != g_Getchannel_Func)
	{
		
		char* strUserdata  =new char[10];
		memset(strUserdata,0,10);
		_itoa_s(idata,strUserdata,10,10);
		idata++;
		std::string guid(strUserdata);
		
		g_Getchannel_Func(&mFreeChannel);
		m_MapUserData[guid] = mFreeChannel;
		if(NULL != g_SetDataCallBack_Func)
		{			
			g_SetDataCallBack_Func(mFreeChannel,netdatacb,strUserdata);
		}
		if(NULL != g_SetProtocolType_Func)
		{
			iRet = g_SetProtocolType_Func(mFreeChannel,1);//1-RTP over UDP , 3-UDP
		}
		if(NULL != g_SetLocalRecvStreamIP_Func)
		{
			iRet = g_SetLocalRecvStreamIP_Func(mFreeChannel,"10.170.54.134");
		}
		if(NULL != g_GetLocalRecvStreamPort_Func)
		{
			iRet = g_GetLocalRecvStreamPort_Func(mFreeChannel,&uiVideoPort,&uiAudioPort);
		}
	}

	
	IVS_REALPLAY_PARAM  RealplayParam = {0};//播放参数
	RealplayParam.bDirectFirst = MEDIA_TRANS;
	RealplayParam.bMultiCast = BROADCAST_UNICAST;
	RealplayParam.uiProtocolType = PROTOCOL_RTP_OVER_UDP;
	RealplayParam.uiStreamType = STREAM_TYPE_MAIN;//主码流
	IVS_MEDIA_ADDR MediaAddrDst = {0};
	IVS_MEDIA_ADDR MediaAddrSrc = {0};
	MediaAddrDst.stIP.uiIPType = IP_V4;
	strncpy(MediaAddrDst.stIP.cIP, "10.170.54.134", IVS_IP_LEN);
	MediaAddrDst.uiAudioPort = uiAudioPort;
	MediaAddrDst.uiVideoPort = uiVideoPort;
	std::string strCameracode = "46638559001317772086#089d6ae965674667bf7f9e5339f8e76b";
	IVS_ULONG ulHandle = 0;
	iRet = IVS_SDK_StartRealPlayByIPEx(m_iSessionID, &RealplayParam, strCameracode.c_str(), &MediaAddrDst, &MediaAddrSrc, &ulHandle);

	if(IVS_SUCCEED == iRet)
	{
		ESDK_RTP_OpenESChannel(FrameDataCB,mFreeChannel);

		if(NULL != g_OpenNetStream_Func)
		{
			iRet = g_OpenNetStream_Func(mFreeChannel,MediaAddrSrc.stIP.cIP,MediaAddrSrc.uiVideoPort,MediaAddrSrc.uiAudioPort);
		}
		if(NULL != g_StartRecvStream_Func)
		{
			if(0==iRet)
			{
				g_StartRecvStream_Func(mFreeChannel);
			}
		}
	}

	
}
