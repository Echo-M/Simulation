
// SimulationDlg.h : 头文件
// 

#pragma once
#include "afxwin.h"
#include "status_bar.h"
#include "anchor_layout.h"
#include "SettingDlg.h"

// CSimulationDlg 对话框
class MainDlg : public CDialog
{
// 构造
public:
	MainDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_MAINDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON			m_hIcon;
	AnchorLayout	m_layout;
	StatusBar		m_statusBar;
	CButton			m_connectStateChkBtn;
	CEdit			m_tipsEdit;
	CButton			m_settingBtn;
	SettingDlg		dlg;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();

	afx_msg void OnBnClickedButtonSetting();
	afx_msg void OnBnClickedCheckConnectState();

	// 添加的消息映射函数
	afx_msg LRESULT OnConnectStateChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceivedCommandUpgrade(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceivedCommand(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceivedCommandSetIRPara(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
