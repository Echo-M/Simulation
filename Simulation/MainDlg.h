
// SimulationDlg.h : ͷ�ļ�
// 

#pragma once
#include "afxwin.h"
#include "status_bar.h"
#include "button.h"
#include "label.h"
#include "anchor_layout.h"

// CSimulationDlg �Ի���
class MainDlg : public CDialog
{
// ����
public:
	MainDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SIMULATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON			m_hIcon;
	AnchorLayout	m_layout;
	StatusBar		m_statusBar;
	Button			m_deviceStateChkBtn;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedCheckDeviceState();
	// ��ӵ���Ϣӳ�亯��
	afx_msg LRESULT OnDeviceStateChanged(WPARAM wParam,LPARAM lParam);
	
	DECLARE_MESSAGE_MAP()
	
};
