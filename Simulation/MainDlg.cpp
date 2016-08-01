
// SimulationDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Simulation.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "DeviceProxy.h"
#include "gui_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define STATUS_DEVICE_STATE 1

// CSimulationDlg �Ի���



MainDlg::MainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void MainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_DEVICE_STATE, m_deviceStateChkBtn);
}

BEGIN_MESSAGE_MAP(MainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	//ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_STATE_DEVICE_CHANGED, &MainDlg::OnDeviceStateChanged)
	ON_BN_CLICKED(IDC_CHECK_DEVICE_STATE, &MainDlg::OnBnClickedCheckDeviceState)
END_MESSAGE_MAP()


// CSimulationDlg ��Ϣ�������

BOOL MainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	// ״̬��
	m_statusBar.Create(this, IDC_PLACE_STATUS);
	CString str;
	str.LoadStringW(IDS_DEVICE_CLOSED);//��ʼ״̬��ʾ�ر�
	m_statusBar.AddPanel(STATUS_DEVICE_STATE, str, IDB_RED_CIRCLE, StatusBar::PANEL_ALIGN_LEFT);

	// �㳮��״̬��ť
	m_deviceStateChkBtn.SetCheck(0);
	m_deviceStateChkBtn.SetWindowText(_T("�����㳮��"));
	

	m_layout.Init(m_hWnd);
	m_layout.AddDlgItem(IDC_CHECK_DEVICE_STATE, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_LEFT);
	m_layout.AddAnchor(m_statusBar.m_hWnd, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_RIGHT);

	ShowWindow(SW_SHOW);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void MainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
		/*CPaintDC dc(this);
		CRect rectTitle;
		GetDlgItem(IDC_PLACE_CAPTION)->GetWindowRect(&rectTitle);
		ScreenToClient(&rectTitle);
		dc.FillSolidRect(rectTitle, RGB(54, 133, 214));*/
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR MainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT MainDlg::OnDeviceStateChanged(WPARAM wParam, LPARAM lParam)
{
	DeviceProxy::ConnectState state = DeviceProxy::GetInstance()->GetDeviceState();
	if (state == DeviceProxy::STATE_DEVICE_CLOSED)
	{
		// ��ť״̬
		m_deviceStateChkBtn.SetCheck(0);
		m_deviceStateChkBtn.SetWindowText(_T("�����鳮��"));

		// ״̬����ʾ
		CString str;
		str.LoadStringW(IDS_DEVICE_CLOSED);
		m_statusBar.SetPanel(1, str, IDB_RED_CIRCLE);
	}
	else if (state == DeviceProxy::STATE_DEVICE_UNCONNECTED)
	{
		DeviceProxy::GetInstance()->Close();
	}
	else if (state == DeviceProxy::STATE_DEVICE_CONNECTED)
	{
		// ״̬����ʾ
		CString str;
		str.LoadStringW(IDS_DEVICE_CONNECTED);
		m_statusBar.SetPanel(1, str, IDB_GREEN_CIRCLE);
	}
	else if (state == DeviceProxy::STATE_DEVICE_LISTENING)
	{
		// ״̬����ʾ
		CString str;
		str.LoadStringW(IDS_DEVICE_LISTENING);
		m_statusBar.SetPanel(1, str, IDB_GREEN_CIRCLE);
	}
	return TRUE;
}


void MainDlg::OnBnClickedCheckDeviceState()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (!m_deviceStateChkBtn.GetCheck())
	{
		m_deviceStateChkBtn.SetWindowText(_T("�����鳮��"));
		DeviceProxy::GetInstance()->Close();
	}
	else if (m_deviceStateChkBtn.GetCheck())
	{
		m_deviceStateChkBtn.SetWindowText(_T("�ر��鳮��"));
		DeviceProxy::GetInstance()->TryConnect();
	}
	UpdateData(FALSE);
}

void MainDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (::IsWindow(m_hWnd)) {
		m_layout.RecalcLayout();
	}
}

BOOL MainDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void MainDlg::OnDestroy()
{
	CDialog::OnDestroy();
	DeviceProxy::GetInstance()->Close();
}