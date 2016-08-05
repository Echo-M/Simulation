#include "stdafx.h"
#include "DeviceProxy.h"
#include "ConfigBlock.h"

// 指令ID USHORT
//#define RESULT_GET_SN						0x0001
#define RESULT_GET_DEVICE_INFO				0x0002
#define RESULT_UPDATE_DEBUG_STATE			0x0003
//#define RESULT_GET_IR_PARAMETERS			0x0003
#define RESULT_UPGRADE						0x0004
#define RESULT_UPGRADE_DATA					0x0005
#define RESULT_RESTART						0x0006
#define RESULT_ECHO							0x8181
#define RESULT_GET_IR_VALUES				0x0007
#define RESULT_SET_IR_PARAMETERS			0x0008
#define RESULT_UPDATE_IR_PARAMETERS			0x0009
#define RESULT_START_MASTER_SIGNAL_DETECT	0x0006
#define RESULT_GET_CIS_PARAMETER			0x0009
#define RESULT_TAKE_CIS_IMAGE				0x000a
#define RESULT_SET_CIS_PARAMETER			0x000b
#define RESULT_UPDATE_CIS_PARAMETER			0x000c
#define RESULT_GET_CIS_CORRECTION_TABLE		0x000d
#define RESULT_UPDATE_CIS_CORRECTION_TABLE	0x000e
#define RESULT_GET_MAC						0x0011
#define RESULT_GET_STUDY_COMPLETED_STATE	0x0012
//#define RESULT_START_OVI_STUDY             0x0013
#define RESULT_SET_AGING_TIME				0x0013
#define RESULT_START_TAPE_STUDY				0x0014
#define RESULT_START_MOTOR					0x0015
#define RESULT_START_RUN_CASH_DETECT		0x8004
#define RESULT_START_SIGNAL_COLLECT			0x0016
#define RESULT_DISABLE_DEBUG				0x0017
#define RESULT_SET_TIME						0x0018
#define RESULT_GET_TIME						0x0019
#define RESULT_LIGHT_CIS					0x0020
#define RESULT_SET_SN						0x0021
#define RESULT_TAPE_LEARNING				0x0022

// ReceiveResult
ReceiveResult::ReceiveResult()
: m_dataLength(0)
, m_id(0) 
, m_count(0)
{
	m_dataBuffer = m_staticBuffer;
}

ReceiveResult::~ReceiveResult() {
	if (m_dataBuffer != m_staticBuffer) {
		delete[] m_dataBuffer;
	}
}

int ReceiveResult::GetID() const 
{
	return m_id;
}

int ReceiveResult::GetCount() const
{
	return m_count;
}

int ReceiveResult::GetDataLength() const 
{
	return m_dataLength;
}

const void *ReceiveResult::GetData() const {
	return m_dataBuffer;
}

int ReceiveResult::GetData(void *buffer, int size) 
{
	if (size > m_dataLength) 
	{
		size = m_dataLength;
	}
	if (size > 0) 
	{
		memcpy(buffer, m_dataBuffer, size);
	}
	return size;
}

void *ReceiveResult::GetDataBuffer(int length) 
{
	if (m_dataBuffer != m_staticBuffer) 
	{
		delete[] m_dataBuffer;
	}
	if (length > STATIC_BUFFER_SIZE) 
	{
		m_dataBuffer = new char[length];
	}
	else 
	{
		m_dataBuffer = m_staticBuffer;
	}
	m_dataLength = length;

	return m_dataBuffer;
}

void ReceiveResult::SetID(int code) 
{
	m_id = code;
}

void ReceiveResult::SetCount(int cnt)
{
	m_count = cnt;
}

// DeviceProxy
DeviceProxy* DeviceProxy::m_device = new DeviceProxy();

static bool WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int value, LPCTSTR lpFileName)
{
	TCHAR valBuf[16];
	_itot(value, valBuf, 10);
	return WritePrivateProfileString(lpAppName, lpKeyName, valBuf, lpFileName);
}

DeviceProxy::DeviceProxy()
{
}


DeviceProxy::~DeviceProxy()
{
}

bool DeviceProxy::Process()
{
	// 只有处于关闭状态才继续执行
	if (m_connectState != STATE_DEVICE_CLOSED)
		return false;

	if (!m_connect.Listen(_T("192.168.8.131"), 1234))
		return false;
								
	m_connectState = STATE_DEVICE_LISTENING;
	PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_CONNECT_CHANGED, 0, 1);

	// 监听请求
	while (m_connecting)
	{
		SOCKET s = m_connect.Accept(100);
		if (s != INVALID_SOCKET)// 收到连接请求且接收成功
		{
			m_connect.Attach(s);
			m_connectState = STATE_DEVICE_CONNECTED;
			PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_CONNECT_CHANGED, 0, 1);
			break;
		}	
	}

	m_connect.SetTimeout(18000);

	// 已连接
	while (m_connecting)
	{
		ReceiveResult result;
		if (!ReceiveCommand(&result))
		{
			TRACE("接收指令与回应失败！\n");
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return true;
}

bool DeviceProxy::Start()
{
	if (m_connecting) // 若线程connect已在运行
	{
		return false;
	}
	m_connecting = true;
	m_thrConnect = std::thread([this](){this->Process(); });

	return true;
}

bool DeviceProxy::Stop()
{
	m_connecting = false;
	m_connectState = STATE_DEVICE_CLOSED;
	PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_CONNECT_CHANGED, 0, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	if (m_thrConnect.joinable())
		m_thrConnect.join();
	m_connect.Close();
	return true;
}

bool DeviceProxy::ReceiveCommand(ReceiveResult* result)
{
	RequestHeader hd;
	
	if (!m_connect.ReceiveFully(&hd, sizeof(hd)))
	{
		m_connectState = STATE_DEVICE_UNCONNECTED;
		PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_CONNECT_CHANGED, 0, 1);
		return false;
	}
	
	if (hd.signatures[0] != 'R' || hd.signatures[1] != 'Q')
	{
		TRACE("包头的signature不是RQ\n");
		return false;
	}

	result->SetID(hd.shortid);
	result->SetCount(hd.count);

	if (hd.length > 0)
	{
		if (!m_connect.ReceiveFully(result->GetDataBuffer(hd.length), hd.length))
		{
			TRACE("接收数据部分时出错\n");
			return false;
		}
	}

	return SendResponse(result);
}

bool DeviceProxy::SendResponse(ReceiveResult* result)
{
	return SendResponse((unsigned short)(result->GetID()), result->GetCount(), result->GetData(), result->GetDataLength());
}

bool DeviceProxy::SendResponse(unsigned short id, unsigned long cnt, const void* recvData, int recvDataLength)
{
	switch(id)
	{
	case RESULT_GET_DEVICE_INFO:
		return SendDeviceInfo(cnt, recvData, recvDataLength);
		break;
	case RESULT_GET_CIS_CORRECTION_TABLE:
		return SendCISCorrectionTable(cnt, recvData, recvDataLength);
		break;
	case RESULT_SET_TIME:
		return SendSetTime(cnt, recvData, recvDataLength);
	case RESULT_ECHO:
		return SendEcho(cnt, recvData, recvDataLength);
	default:
		return false;
		break;
	}
}

bool DeviceProxy::SendResponse(unsigned long cnt, unsigned short status, const void* data, int dataLength)
{
	if (dataLength < 0)
		return false;

	ResponseHeader hd;
	hd.signatures[0] = 'R';
	hd.signatures[1] = 'P';
	hd.count = cnt;
	hd.status = status;
	hd.length = dataLength;

	if (!m_connect.Send(&hd, sizeof(hd)))
		return false;

	if (dataLength > 0)
	{
		if (!m_connect.Send(data, dataLength))
			return false;
	}

	return true;
}

bool DeviceProxy::SendDeviceInfo(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 1; // 失败

	if (recvDataLength == 0)
		status = 0;
	else
		status = 1;

	DeviceInfo deviceInfo;
	ConfigBlock *config = ConfigBlock::GetInstance();
	CString str;
	int len{-1};

	str = config->GetStringParameter(L"DeviceInfo", L"sn", L"");
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, deviceInfo.sn, len, NULL, NULL);

	str = config->GetStringParameter(L"DeviceInfo", L"model", L"");
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, deviceInfo.model, len, NULL, NULL);

	str = config->GetStringParameter(L"DeviceInfo", L"firmwareVersion", L"");
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, str, -1, deviceInfo.firmwareVersion, len, NULL, NULL);

	deviceInfo.numberOfCIS = config->GetIntParameter(L"DeviceInfo", L"numberOfCIS", 0);
	deviceInfo.numberOfIR = config->GetIntParameter(L"DeviceInfo", L"numberOfIR", 0);
	deviceInfo.numberOfMH = config->GetIntParameter(L"DeviceInfo", L"numberOfMH", 0);
	deviceInfo.reserved = config->GetIntParameter(L"DeviceInfo", L"reserved", 0);
	deviceInfo.cisColorFlags = config->GetIntParameter(L"DeviceInfo", L"cisColorFlags", 0);
	deviceInfo.cisImageWidth = config->GetIntParameter(L"DeviceInfo", L"cisImageWidth", 0);
	deviceInfo.cisImageHeight = config->GetIntParameter(L"DeviceInfo", L"cisImageHeight", 0);
	deviceInfo.selfTestState = config->GetIntParameter(L"DeviceInfo", L"selfTestState", 0);

	str = config->GetStringParameter(L"DeviceInfo", L"debugState", L"");
	int index = 0;
	CString temp = L"";
	for (int i = 0; ; i++)
	{
		if (str[i] != ','&&i < str.GetLength())
			temp += str[i];
		else if (str[i] == ',' || i == str.GetLength())
		{
			swscanf(temp, _T("%d"), &(deviceInfo.debugState[index++]));
			temp = L"";
			if (i == str.GetLength())
				break;
		}
	}

	return SendResponse(cnt, status, &deviceInfo, sizeof(deviceInfo));
}

bool DeviceProxy::SendCISCorrectionTable(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 1; // 失败

	if (recvDataLength == 0)
		status = 0;
	else
		status = 1;

	CISCorrectionTable table;
	for (int i = 0; i < CIS_COUNT; i++)
	{
		for (int j = 0; j < COLOR_COUNT; j++)
		{
			for (int k = 0; k < CIS_IMAGE_WIDTH; k++)
			{
				for (int l = 0; l < 256; l++)
				{
					table.data[i][j][k][l] = l;
				}
			}
		}
	}

	return SendResponse(cnt, status, &table, sizeof(table));
}

bool DeviceProxy::SendSetTime(unsigned long cnt, const void* recvData, int recvDataLength)
{
	#pragma pack(push)
	#pragma pack(1)
	// 时间参数
	struct Time
	{
		unsigned short year;
		unsigned short month;
		unsigned short day;
		unsigned short hour;
		unsigned short minute;
		unsigned short second;
		unsigned short millisecond;
	};
	#pragma pack(pop)

	unsigned short status = 1; // 失败

	if (recvDataLength == 14)
	{
		status = 0;
		Time t;
		memcpy(&t, recvData, recvDataLength);
	}
	else
		status = 1;

	return SendResponse(cnt, status, NULL, 0);
}

bool DeviceProxy::SendEcho(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0; // 成功
	return SendResponse(cnt, status, recvData, recvDataLength);
}