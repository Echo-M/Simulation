
// SimulationDlg.cpp : ʵ���ļ�
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

// ״̬�����
#define STATUSBAR_CONNECT_STATE		1
#define STATUSBAR_DEVICE_INFO		2

// CSimulationDlg �Ի���



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
	//ON_WM_ERASEBKGND() // ���� ��ȥ��������Ϣ����
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
	m_statusBar.AddPanel(STATUSBAR_CONNECT_STATE, str, IDB_RED_CIRCLE, StatusBar::PANEL_ALIGN_RIGHT);
	str = "�汾: ";
	str += ConfigBlock::GetInstance()->GetStringParameter(L"DeviceInfo", L"firmwareVersion", L"");
	m_statusBar.AddPanel(STATUSBAR_DEVICE_INFO, str, NULL, StatusBar::PANEL_ALIGN_RIGHT);

	// �㳮��״̬��ť
	m_connectStateChkBtn.SetCheck(0);
	m_connectStateChkBtn.SetWindowText(_T("�����㳮��"));
	
	// ����
	m_layout.Init(m_hWnd);
	m_layout.AddAnchor(m_statusBar.m_hWnd, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_RIGHT);
	m_layout.AddDlgItem(IDC_EDIT_TIPS, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_RIGHT);
	m_layout.AddDlgItem(IDC_STATIC_TIPS, AnchorLayout::BOTTOM_LEFT, AnchorLayout::BOTTOM_LEFT);

	// ��ʾ
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

LRESULT MainDlg::OnConnectStateChanged(WPARAM wParam, LPARAM lParam)
{
	ConnectState state = DeviceProxy::GetInstance()->GetConnectState();
	if (state == STATE_DEVICE_CLOSED)
	{
		// ��ť״̬
		m_connectStateChkBtn.SetCheck(0);
		m_connectStateChkBtn.SetWindowText(_T("�����鳮��"));

		// ״̬����ʾ
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
		// ״̬����ʾ
		CString str;
		str.LoadStringW(IDS_DEVICE_CONNECTED);
		m_statusBar.SetPanel(STATUSBAR_CONNECT_STATE, str, IDB_GREEN_CIRCLE);
	}
	else if (state == STATE_DEVICE_LISTENING)
	{
		// ״̬����ʾ
		CString str;
		str.LoadStringW(IDS_DEVICE_LISTENING);
		m_statusBar.SetPanel(STATUSBAR_CONNECT_STATE, str, IDB_GREEN_CIRCLE);
	}
	return TRUE;
}

LRESULT MainDlg::OnReceivedCommandUpgrade(WPARAM wParam, LPARAM lParam)
{
	if (MessageBox(L"ȷ��Ҫ����������", L"�㳮����������", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		ConfigBlock::GetInstance()->SetIntParameter(L"UpgradePara", L"flag", 0);
	}
	else
	{
		ConfigBlock::GetInstance()->SetIntParameter(L"UpgradePara", L"flag", 1);
	}
	CString str = L"�汾: ";
	str += ConfigBlock::GetInstance()->GetStringParameter(L"DeviceInfo", L"firmwareVersion", L"");
	m_statusBar.SetPanel(STATUSBAR_DEVICE_INFO, str, NULL);
	return true;
}

LRESULT MainDlg::OnReceivedCommandSetIRPara(WPARAM wParam, LPARAM lParam)
{
	if (ConfigBlock::GetInstance()->GetIntParameter(L"IRCalibrationPara", L"flag", 0) == 1)//��һ�Σ���ʼIRУ׼
	{
		if (MessageBox(L"�Ƿ����У׼ֽ��", L"�㳮������У׼����", MB_OK | MB_ICONQUESTION) == IDOK)
		{
			ConfigBlock::GetInstance()->SetIntParameter(L"IRCalibrationPara", L"paper", 0);
			ConfigBlock::GetInstance()->SetIntParameter(L"IRCalibrationPara", L"flag", 0);//�������״̬
		}
	}
	return true;
}

LRESULT MainDlg::OnReceivedCommand(WPARAM wParam, LPARAM lParam)
{
	DeviceProxy::CurCommand curCommand = DeviceProxy::GetInstance()->GetCurCommand();
	CString str;
	if (curCommand.status == 0)
		str = "�ɹ�: ";
	else
		str = "ʧ��: ";
	int index = m_tipsEdit.GetWindowTextLength();
	m_tipsEdit.SetSel(index, index, TRUE);
	switch (curCommand.id)
	{
	case RESULT_GET_DEVICE_INFO:
		str += "�����豸��Ϣ\r\n";
		break;
	case RESULT_GET_CIS_CORRECTION_TABLE:
		str += "����CISͼ��У׼��\r\n";
		break;
	case RESULT_SET_TIME:
		str += "���ñ���ʱ��\r\n";
		break;
	/*case RESULT_ECHO:
		str = "";
		m_tipsEdit.ReplaceSel(str);
		break;*/
	case RESULT_UPGRADE:
		str += "��ʼ����\r\n";
	case RESULT_UPGRADE_DATA:
		str += "�����������ݰ�\r\n";
		break;
	case RESULT_UPDATE_DEBUG_STATE:
		str += "���µ���״̬\r\n";
		break;
	case RESULT_RESTART:
		str += "�豸����\r\n";
		/*DeviceProxy::GetInstance()->Stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		DeviceProxy::GetInstance()->Start();*/
		break;
	case RESULT_SET_IR_PARAMETERS:
		str += "���ú���У׼����\r\n";
		break;
	case RESULT_GET_IR_VALUES:
		str += "��ȡ����Թ���ֵ\r\n";
		break;
	case RESULT_UPDATE_IR_PARAMETERS:
		str += "���º���У׼����\r\n";
		break;
	case RESULT_START_MOTOR:
		str += "ת�����\r\n";
		break;
	case RESULT_START_RUN_CASH_DETECT:
		str += "�߳��ź�\r\n";;
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
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (!m_connectStateChkBtn.GetCheck())
	{
		DeviceProxy::GetInstance()->Stop();
		m_connectStateChkBtn.SetWindowText(_T("�����鳮��"));
	}
	else if (m_connectStateChkBtn.GetCheck())
	{
		DeviceProxy::GetInstance()->Start();
		m_connectStateChkBtn.SetWindowText(_T("�ر��鳮��"));
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
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	
	if (m_connectStateChkBtn.GetCheck())
	{
		MessageBox(L"���ȹرյ㳮����", L"����", MB_OK | MB_ICONERROR);
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
