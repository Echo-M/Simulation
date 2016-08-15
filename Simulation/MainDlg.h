
// SimulationDlg.h : ͷ�ļ�
// 

#pragma once
#include "afxwin.h"
#include "status_bar.h"
#include "anchor_layout.h"
#include "SettingDlg.h"

// CSimulationDlg �Ի���
class MainDlg : public CDialog
{
// ����
public:
	MainDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MAINDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
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

	// ��ӵ���Ϣӳ�亯��
	afx_msg LRESULT OnConnectStateChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceivedCommandUpgrade(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceivedCommand(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceivedCommandSetIRPara(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
