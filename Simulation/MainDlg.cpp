
// SimulationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Simulation.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "DeviceProxy.h"
#include "gui_resources.h"
#include "ConfigBlock.h"
#include "SettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 状态栏编号
#define STATUSBAR_CONNECT_STATE		1
#define STATUSBAR_DEVICE_INFO		2

// CSimulationDlg 对话框



MainDlg::MainDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MainDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void MainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_CONNECT_STATE, m_connectStateChkBtn);
	DDX_Control(pDX, IDC_EDIT_TIPS, m_tipsEdit);
	DDX_Control(pDX, IDC_BUTTON_SETTING, m_settingBtn);
}

BEGIN_MESSAGE_MAP(MainDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	//ON_WM_ERASEBKGND() // 屏蔽 擦去背景的消息函数
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_STATE_CONNECT_CHANGED, &MainDlg::OnConnectStateChanged)
	ON_MESSAGE(WM_RECEIVED_COMMAND_UPGRADE, &MainDlg::OnReceivedCommandUpgrade)
	ON_MESSAGE(WM_RECEIVED_COMMAND,&MainDlg::OnReceivedCommand)
	ON_MESSAGE(WM_RECEIVED_COMMAND_SET_IR_PARA, &MainDlg::OnReceivedCommandSetIRPara)
	ON_MESSAGE(WM_RUN_CASH_STOPPED,&MainDlg::OnRunCashStopped)
	ON_BN_CLICKED(IDC_CHECK_CONNECT_STATE, &MainDlg::OnBnClickedCheckConnectState)
	ON_BN_CLICKED(IDC_BUTTON_SETTING, &MainDlg::OnBnClickedButtonSetting)
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
	m_statusBar.AddPanel(STATUSBAR_CONNECT_STATE, str, IDB_RED_CIRCLE, StatusBar::PANEL_ALIGN_RIGHT);
	str = "版本: ";
	str += ConfigBlock::GetInstance()->GetStringParameter(L"DeviceInfo", L"firmwareVersion", L"");
	m_statusBar.AddPanel(STATUSBAR_DEVICE_INFO, str, NULL, StatusBar::PANEL_ALIGN_RIGHT);

	// 点钞机状态按钮
	m_connectStateChkBtn.SetCheck(0);
	m_connectStateChkBtn.SetWindowText(_T("启动点钞机"));
	
	// 布局
	m_layout.Init(m_hWnd);
	m_layout.AddAnchor(m_statusBar.m_hWnd, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_RIGHT);
	m_layout.AddDlgItem(IDC_EDIT_TIPS, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_RIGHT);
	m_layout.AddDlgItem(IDC_STATIC_TIPS, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_LEFT);

	// 显示
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

LRESULT MainDlg::OnConnectStateChanged(WPARAM wParam, LPARAM lParam)
{
	ConnectState state = DeviceProxy::GetInstance()->GetConnectState();
	if (state == STATE_DEVICE_CLOSED)
	{
		// 按钮状态
		m_connectStateChkBtn.SetCheck(0);
		m_connectStateChkBtn.SetWindowText(_T("启动验钞机"));

		// 状态栏提示
		CString str;
		str.LoadStringW(IDS_DEVICE_CLOSED);
		m_statusBar.SetPanel(STATUSBAR_CONNECT_STATE, str, IDB_RED_CIRCLE);
	}
	else if (state == STATE_DEVICE_UNCONNECTED)
	{
		DeviceProxy::GetInstance()->Stop();
	}
	else if (state == STATE_DEVICE_CONNECTED)
	{
		// 状态栏提示
		CString str;
		str.LoadStringW(IDS_DEVICE_CONNECTED);
		m_statusBar.SetPanel(STATUSBAR_CONNECT_STATE, str, IDB_GREEN_CIRCLE);
	}
	else if (state == STATE_DEVICE_LISTENING)
	{
		// 状态栏提示
		CString str;
		str.LoadStringW(IDS_DEVICE_LISTENING);
		m_statusBar.SetPanel(STATUSBAR_CONNECT_STATE, str, IDB_GREEN_CIRCLE);
	}
	return TRUE;
}

LRESULT MainDlg::OnReceivedCommandUpgrade(WPARAM wParam, LPARAM lParam)
{
	if (MessageBox(L"确定要进行升级吗？", L"点钞机升级配置", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		ConfigBlock::GetInstance()->SetIntParameter(L"UpgradePara", L"flag", 0);
	}
	else
	{
		ConfigBlock::GetInstance()->SetIntParameter(L"UpgradePara", L"flag", 1);
	}
	CString str = L"版本: ";
	str += ConfigBlock::GetInstance()->GetStringParameter(L"DeviceInfo", L"firmwareVersion", L"");
	m_statusBar.SetPanel(STATUSBAR_DEVICE_INFO, str, NULL);
	return true;
}

LRESULT MainDlg::OnReceivedCommandSetIRPara(WPARAM wParam, LPARAM lParam)
{
	if (ConfigBlock::GetInstance()->GetIntParameter(L"IRCalibrationPara", L"flag", 0) == 1)//第一次，开始IR校准
	{
		if (MessageBox(L"是否放入校准纸？", L"点钞机红外校准配置", MB_OK | MB_ICONQUESTION) == IDOK)
		{
			ConfigBlock::GetInstance()->SetIntParameter(L"IRCalibrationPara", L"paper", 0);
			ConfigBlock::GetInstance()->SetIntParameter(L"IRCalibrationPara", L"flag", 0);//进入调试状态
		}
	}
	return true;
}

LRESULT MainDlg::OnReceivedCommand(WPARAM wParam, LPARAM lParam)
{
	DeviceProxy::CurCommand curCommand = DeviceProxy::GetInstance()->GetCurCommand();
	CString str;
	if (curCommand.status == 0)
		str = "成功: ";
	else
		str = "失败: ";
	int index = m_tipsEdit.GetWindowTextLength();
	m_tipsEdit.SetSel(index, index, TRUE);
	switch (curCommand.id)
	{
	case RESULT_GET_DEVICE_INFO:
		str += "发送设备信息\r\n";
		break;
	case RESULT_GET_CIS_CORRECTION_TABLE:
		str += "发送CIS图像校准表\r\n";
		break;
	case RESULT_SET_TIME:
		str += "设置本地时间\r\n";
		break;
	/*case RESULT_ECHO:
		str = "";
		m_tipsEdit.ReplaceSel(str);
		break;*/
	case RESULT_UPGRADE:
		str += "开始升级\r\n";
	case RESULT_UPGRADE_DATA:
		str += "接收升级数据包\r\n";
		break;
	case RESULT_UPDATE_DEBUG_STATE:
		str += "更新调试状态\r\n";
		break;
	case RESULT_RESTART:
		str += "设备重启\r\n";
		/*DeviceProxy::GetInstance()->Stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		DeviceProxy::GetInstance()->Start();*/
		break;
	case RESULT_SET_IR_PARAMETERS:
		str += "设置红外校准参数\r\n";
		break;
	case RESULT_GET_IR_VALUES:
		str += "获取红外对管数值\r\n";
		break;
	case RESULT_UPDATE_IR_PARAMETERS:
		str += "更新红外校准参数\r\n";
		break;
	case RESULT_START_MOTOR:
		str += "转动电机\r\n";
		break;
	case RESULT_START_RUN_CASH_DETECT:
		str += "走钞信号\r\n";;
	default:
		return false;
		break;
	}

	m_tipsEdit.ReplaceSel(str);
	m_tipsEdit.LineScroll(m_tipsEdit.GetLineCount());
	return true;

}

void MainDlg::OnBnClickedCheckConnectState()
{
	// TODO:  在此添加控件通知处理程序代码
	if (!m_connectStateChkBtn.GetCheck())
	{
		DeviceProxy::GetInstance()->Stop();
		m_connectStateChkBtn.SetWindowText(_T("启动验钞机"));
	}
	else if (m_connectStateChkBtn.GetCheck())
	{
		DeviceProxy::GetInstance()->Start();
		m_connectStateChkBtn.SetWindowText(_T("关闭验钞机"));
	}
}

void MainDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (::IsWindow(m_hWnd)) 
	{
		m_layout.RecalcLayout();
	}
}

BOOL MainDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void MainDlg::OnDestroy()
{
	DeviceProxy::GetInstance()->Stop();
	CDialog::OnDestroy();
}


void MainDlg::OnBnClickedButtonSetting()
{
	// TODO:  在此添加控件通知处理程序代码
	
	if (m_connectStateChkBtn.GetCheck())
	{
		MessageBox(L"请先关闭点钞机！", L"错误", MB_OK | MB_ICONERROR);
	}
	else
	{
		dlg.DoModal();
	}
}

LRESULT MainDlg::OnRunCashStopped(WPARAM wParam, LPARAM lParam)
{
	DeviceProxy::GetInstance()->StopSendingCash();
	return TRUE;
}
