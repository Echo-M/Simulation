#include "stdafx.h"
#include <memory>
#include "DeviceProxy.h"
#include "ConfigBlock.h"

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
	Stop();
}

bool DeviceProxy::Process()
{
	// 只有处于关闭状态才继续执行
	if (m_connectState != STATE_DEVICE_CLOSED)
		return false;

	int port = ConfigBlock::GetInstance()->GetIntParameter(L"DeviceAddress", L"port", 0);
	CString ip = ConfigBlock::GetInstance()->GetStringParameter(L"DeviceAddress", L"ip", L"");
	//if (!m_connect.Listen(L"192.168.8.131", 1234))
	if (!m_connect.Listen((LPCTSTR)ip, port))
		return false;
								
	m_connectState = STATE_DEVICE_LISTENING;
	PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_CONNECT_CHANGED, 0, 1);

	// 监听请求
	while (m_connecting)
	{
		SOCKET s = m_connect.Accept(18000);
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
			//TRACE("接收指令与回应失败！\n");
		}
		
		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
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
	m_connect.Close();
	PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_CONNECT_CHANGED, 0, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	if (m_thrConnect.joinable())
		m_thrConnect.join();
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
	m_curCommand.id = id;
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
		break;
	case RESULT_ECHO:
		return SendEcho(cnt, recvData, recvDataLength);
		break;
	case RESULT_UPGRADE:
		return SendUpgrade(cnt, recvData, recvDataLength);
		break;
	case RESULT_UPGRADE_DATA:
		return SendUpgradeData(cnt, recvData, recvDataLength);
		break;
	case RESULT_UPDATE_DEBUG_STATE:
		return SendUpgradeDebugState(cnt, recvData, recvDataLength);
		break;
	case RESULT_RESTART:
		return SendRestart(cnt, recvData, recvDataLength);
		break;
	case RESULT_SET_IR_PARAMETERS:
		return SendSetIRParameters(cnt, recvData, recvDataLength);
		break;
	case RESULT_GET_IR_VALUES:
		return SendGetIRValues(cnt, recvData, recvDataLength);
		break;
	case RESULT_UPDATE_IR_PARAMETERS:
		return SendUpdateIRParameters(cnt, recvData, recvDataLength);
		break;
	case RESULT_START_MOTOR:
		return SendStartMotor(cnt, recvData, recvDataLength);
		break;
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

	m_curCommand.status = status;
	SendMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_RECEIVED_COMMAND, 0, 1);

	return true;
}

bool DeviceProxy::SendDeviceInfo(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0; // 默认成功

	if (recvDataLength != 0)
		return SendResponse(cnt, 1, NULL, 0);

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
	CString temp = L"";
	int index = 0;
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
	unsigned short status = 0; // 默认成功

	if (recvDataLength != 0)
		return SendResponse(cnt, 1, NULL, 0);

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
	unsigned short status = 0; // 默认成功

	if (recvDataLength != 14)
		return SendResponse(cnt, 1, NULL, 0);

	//保存接收到的时间
	/*
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
	Time t;
	memcpy(&t, recvData, recvDataLength);
	*/

	return SendResponse(cnt, status, NULL, 0);
}

bool DeviceProxy::SendEcho(unsigned long cnt, const void* recvData, int recvDataLength)
{
	return SendResponse(cnt, 0, recvData, recvDataLength);
}

bool DeviceProxy::SendUpgrade(unsigned long cnt, const void* recvData, int recvDataLength)
{
	SendMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_RECEIVED_COMMAND_UPGRADE, 0, 1);

	unsigned short status = ConfigBlock::GetInstance()->GetIntParameter(L"UpgradePara", L"flag", 0);
	if (status == 1)
		return SendResponse(cnt, status, NULL, 0);

	if (recvDataLength != 16)//11字节固件版本(字符串) + 1字节0补充 + 4字节固件包长度（unsigned long长整形）
		return SendResponse(cnt, 1, NULL, 0);;

	unsigned char* buf = new unsigned char[16];
	memcpy(buf, recvData, 16);
	// 保存固件包长度
	unsigned long length = buf[15] << 24 | buf[14] << 16 | buf[13] << 8 | buf[12] << 0;
	ConfigBlock::GetInstance()->SetIntParameter(L"UpgradePara", L"length", length);
	// 将版本号保存至配置文件
	CString version(buf);
	ConfigBlock::GetInstance()->SetStringParameter(L"DeviceInfo", L"firmwareVersion", version);
	delete buf;

	// 发送每次需要上传的包的大小
	unsigned long recvLength = ConfigBlock::GetInstance()->GetIntParameter(L"UpgradePara", L"recvLength", 10000);
	return SendResponse(cnt, status, &recvLength, 4);
}

bool DeviceProxy::SendUpgradeData(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = ConfigBlock::GetInstance()->GetIntParameter(L"UpgradePara", L"flag", 1);
	if (status == 1)
		return SendResponse(cnt, status, NULL, 0);

	unsigned long recvLength = ConfigBlock::GetInstance()->GetIntParameter(L"UpgradePara", L"recvLength", 0);
	if (recvDataLength > recvLength)
		return SendResponse(cnt, 1, NULL, 0);

	//保存文件
	if (0 == ConfigBlock::GetInstance()->GetIntParameter(L"UpgradePara", L"saveFile", 0))
	{
		CString savePath = ConfigBlock::GetInstance()->GetStringParameter(L"UpgradePara", L"savePath", L"");
		savePath += "\\upgradeData";
		savePath += ConfigBlock::GetInstance()->GetStringParameter(L"DeviceInfo", L"firmwareVersion", L"");
		savePath += ".dat";
		CFile file(savePath, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite | CFile::modeNoTruncate);
		file.SeekToEnd();
		file.Write(recvData, recvDataLength);
	}

	return SendResponse(cnt, status, NULL, 0);
}

bool DeviceProxy::SendUpgradeDebugState(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0;// 默认成功

	if (recvDataLength != 64)
		return SendResponse(cnt, 1, NULL, 0);
	
	//存到DeviceInfo的debugState中
	int iTemp[16];
	memcpy(iTemp, recvData, recvDataLength);
	CString strTemp;
	strTemp.Format(L"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
		iTemp[0], iTemp[1], iTemp[2], iTemp[3], iTemp[4], iTemp[5], iTemp[6], iTemp[7], iTemp[8], iTemp[9], iTemp[10], iTemp[11], iTemp[12], iTemp[13], iTemp[14], iTemp[15]);
	ConfigBlock::GetInstance()->SetStringParameter(L"DeviceInfo", L"debugState", strTemp);

	//
	ConfigBlock::GetInstance()->SetIntParameter(L"UpgradePara", L"flag", 1);
	ConfigBlock::GetInstance()->SetIntParameter(L"IRCalibrationPara", L"flag", 1);
	
	return SendResponse(cnt, status, NULL, 0);
}

bool DeviceProxy::SendRestart(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0;// 默认成功

	if (recvDataLength != 0)
		return SendResponse(cnt, 1, NULL, 0);

	return SendResponse(cnt, status, NULL, 0);
}

bool DeviceProxy::SendSetIRParameters(unsigned long cnt, const void* recvData, int recvDataLength)
{
	SendMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_RECEIVED_COMMAND_SET_IR_PARA, 0, 1);

	unsigned short status = 0;// 默认成功

	int cntIR = ConfigBlock::GetInstance()->GetIntParameter(L"DeviceInfo", L"numberOfIR", 0);
	if (recvDataLength != sizeof(int)*cntIR) // 应为 4*n，n是红外对管的个数
		return SendResponse(cnt, 1, NULL, 0);

	//在这里添加处理收到的数据的代码...
	//收到的值应该在1300~3500之间

	return SendResponse(cnt, status, recvData, recvDataLength);
}

bool DeviceProxy::SendGetIRValues(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0;// 默认成功

	if (recvDataLength != 0) // 应0
		return SendResponse(cnt, 1, NULL, 0);

	// 填充发送数据
	int cntIR = ConfigBlock::GetInstance()->GetIntParameter(L"DeviceInfo", L"numberOfIR", 0);
	int paper = ConfigBlock::GetInstance()->GetIntParameter(L"IRCalibrationPara", L"paper", 0);

	int* state = new int[cntIR];
	int index = 0;
	CString str = ConfigBlock::GetInstance()->GetStringParameter(L"IRCalibrationPara", L"IRState", L"");
	CString temp = L"";
	for (int i = 0;; i++)
	{
		if (str[i] != ','&&i < str.GetLength())
			temp += str[i];
		else if (str[i] == ',' || i == str.GetLength())
		{
			swscanf(temp, _T("%d"), &(state[index++]));
			temp = L"";
			if (i == str.GetLength())
				break;
		}
	}
	int* buf = new int[cntIR];
	for (int i = 0; i < cntIR; ++i)
	{
		if (state[i] == 0) // 该红外对管是好的
		{
			srand((int)time(NULL) + i*100);
			buf[i] = paper == 0 ? 650 + rand() % 100 : 3750 + rand() % 100; //有纸时是650-700，无纸是3750-3850
		}
		else
		{
			buf[i] = 0; //不管是有纸还是无纸都返回0
		}
	}

	if (SendResponse(cnt, status, buf, sizeof(int)*cntIR))
	{
		delete buf;
		delete state;
		return true;
	}
	else
	{
		delete buf;
		delete state;
		return false;
	}
}

bool DeviceProxy::SendUpdateIRParameters(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0;// 默认成功

	int cntIR = ConfigBlock::GetInstance()->GetIntParameter(L"DeviceInfo", L"numberOfIR", 0);
	if (recvDataLength != sizeof(int)*cntIR) // 应为 4*n，n是红外对管的个数
		return SendResponse(cnt, 1, NULL, 0);

	//在这里添加处理收到的数据的代码...

	return SendResponse(cnt, status, recvData, recvDataLength);
}

bool DeviceProxy::SendStartMotor(unsigned long cnt, const void* recvData, int recvDataLength)
{
	unsigned short status = 0;// 默认成功

	if (recvDataLength != sizeof(int)) // 应为4
		return SendResponse(cnt, 1, NULL, 0);

	ConfigBlock::GetInstance()->SetIntParameter(L"IRCalibrationPara", L"paper", 1);//无纸
	return SendResponse(cnt, status, NULL, 0);
}