
// SipSDKDemoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "IVS_SDK.h"


// CSipSDKDemoDlg dialog
class CSipSDKDemoDlg : public CDialogEx
{
// Construction
public:
	CSipSDKDemoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SIPSDKDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	int m_iResponseID;
	bool mbNetSourceLoad;
	unsigned long mFreeChannel;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	static int __stdcall SIP_CallBack_FUNC(int msgType,int eventType,const char* pPara);
	afx_msg void OnBnClickedInit();
	afx_msg void OnBnClickedSub();
	afx_msg void OnBnClickedReg();
	afx_msg void OnBnClickedInvite();
	afx_msg void OnBnClickedMessage();
	afx_msg void OnBnClickedUninit();
	afx_msg void OnClose();
	afx_msg void OnBnClickedUnreg();
	afx_msg void OnBnClickedBye();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnDevRefresh(WPARAM,LPARAM);
	CString m_LocalIP;
	CString m_RemoteIP;
	CString m_LocalPort;
	CString m_RemotePort;
	CString m_SIPUserName;
	CString m_SIPPwd;
	CString m_DevID;
	IVS_INT32 m_iSessionID;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	CEdit m_DevList;
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	CString m_ServerID;
	CString m_QueryID;
	CString m_LocalID;
	CString m_RemoteSipName;
	CString m_RemoteSipPwd;
};
