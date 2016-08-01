
// SimulationDlg.h : 头文件
// 

#pragma once
#include "afxwin.h"
#include "status_bar.h"
#include "button.h"
#include "label.h"
#include "anchor_layout.h"

// CSimulationDlg 对话框
class MainDlg : public CDialog
{
// 构造
public:
	MainDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SIMULATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON			m_hIcon;
	AnchorLayout	m_layout;
	StatusBar		m_statusBar;
	Button			m_deviceStateChkBtn;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCheckDeviceState();
	// 添加的消息映射函数
	afx_msg LRESULT OnDeviceStateChanged(WPARAM wParam,LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	
};
