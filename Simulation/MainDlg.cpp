
// SimulationDlg.cpp : 实现文件
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

// CSimulationDlg 对话框



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


// CSimulationDlg 消息处理程序

BOOL MainDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	// 状态栏
	m_statusBar.Create(this, IDC_PLACE_STATUS);
	CString str;
	str.LoadStringW(IDS_DEVICE_CLOSED);//初始状态显示关闭
	m_statusBar.AddPanel(STATUS_DEVICE_STATE, str, IDB_RED_CIRCLE, StatusBar::PANEL_ALIGN_LEFT);

	// 点钞机状态按钮
	m_deviceStateChkBtn.SetCheck(0);
	m_deviceStateChkBtn.SetWindowText(_T("启动点钞机"));
	

	m_layout.Init(m_hWnd);
	m_layout.AddDlgItem(IDC_CHECK_DEVICE_STATE, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_LEFT);
	m_layout.AddAnchor(m_statusBar.m_hWnd, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_RIGHT);

	ShowWindow(SW_SHOW);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void MainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
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

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR MainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT MainDlg::OnDeviceStateChanged(WPARAM wParam, LPARAM lParam)
{
	DeviceProxy::ConnectState state = DeviceProxy::GetInstance()->GetDeviceState();
	if (state == DeviceProxy::STATE_DEVICE_CLOSED)
	{
		// 按钮状态
		m_deviceStateChkBtn.SetCheck(0);
		m_deviceStateChkBtn.SetWindowText(_T("启动验钞机"));

		// 状态栏提示
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
		// 状态栏提示
		CString str;
		str.LoadStringW(IDS_DEVICE_CONNECTED);
		m_statusBar.SetPanel(1, str, IDB_GREEN_CIRCLE);
	}
	else if (state == DeviceProxy::STATE_DEVICE_LISTENING)
	{
		// 状态栏提示
		CString str;
		str.LoadStringW(IDS_DEVICE_LISTENING);
		m_statusBar.SetPanel(1, str, IDB_GREEN_CIRCLE);
	}
	return TRUE;
}


void MainDlg::OnBnClickedCheckDeviceState()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!m_deviceStateChkBtn.GetCheck())
	{
		m_deviceStateChkBtn.SetWindowText(_T("启动验钞机"));
		DeviceProxy::GetInstance()->Close();
	}
	else if (m_deviceStateChkBtn.GetCheck())
	{
		m_deviceStateChkBtn.SetWindowText(_T("关闭验钞机"));
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