#pragma once

#include "TcpSocket.h"
#include "thread.h"
#include <thread>

#define WM_STATE_CONNECT_CHANGED			WM_USER + 1
#define WM_RECEIVED_COMMAND					WM_USER + 2
#define WM_RECEIVED_COMMAND_UPGRADE			WM_USER + 3
#define WM_RECEIVED_COMMAND_START_MOTOR		WM_USER + 4
#define WM_RECEIVED_COMMAND_SET_IR_PARA		WM_USER + 5
#define WM_RUN_CASH_STOPPED					WM_USER + 6


#define SERIAL_NUMBER_LENGTH		24 //���кų���
#define MODEL_NAME_LENGTH			24 //�ͺų���
#define FIRMWARE_VERSION_LENGTH		12 //�̼��汾����
#define DISABLED_VALUE			    -1

// ָ��ID USHORT
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
//#define RESULT_GET_CIS_PARAMETER			0x0009
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

// ����״̬
typedef enum ConnectState_
{
	STATE_DEVICE_CLOSED,
	STATE_DEVICE_LISTENING,
	STATE_DEVICE_CONNECTED,
	STATE_DEVICE_UNCONNECTED
} ConnectState;

#pragma pack(push)
#pragma pack(1)
// �豸��Ϣ
struct DeviceInfo 
{
	char sn[SERIAL_NUMBER_LENGTH]; // �㳮�����к�
	char model[MODEL_NAME_LENGTH]; // �㳮���ͺ�
	char firmwareVersion[FIRMWARE_VERSION_LENGTH]; // �̼��汾
	byte numberOfCIS; // CIS����
	byte numberOfIR; // ����Թܵĸ���
	byte numberOfMH; // ��ͷ����
	byte reserved;
	int cisColorFlags; //  ir/r/b/g  = bit  3/2/1/0
	short cisImageWidth;
	short cisImageHeight;
	int selfTestState; // �����Լ�״̬
	int debugState[16]; // ����״̬
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
// ��λ����Ӧָ��
typedef struct ResponseHeader_
{
	unsigned char signatures[2];  //��ʾһ����Ӧ��,�̶�ΪRP
	unsigned short status;   //��ʾָ��ִ��״̬��0��ʾ�ɹ�������Ϊ������롣
	unsigned long count;	//��ʾ�������
	unsigned long length;   //��ʾ�������ݵĳ���
} ResponseHeader;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
// ��λ������ָ��
typedef struct RequestHeader_
{
	unsigned char signatures[2];   //��ʾһ�������,�̶�ΪRQ
	unsigned short shortid;  //��ʾ���ָ��ı�ʶ
	unsigned long count;  //��ʾ�������
	unsigned long length;  //��ʾ��η��͵ĸ��صĳ���
} RequestHeader;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
// ��Ʊ��Ϣ
struct CashInfo
{
	int count; //���
	int denomination; //���
	int version; //�汾
	int direction; //����
	int error; // �������
	char sn[32]; //���ֺ�
	int snImageSize; //
	unsigned int snImage[12][32];
};
#pragma pack(pop)

enum {
	COLOR_GREEN,
	COLOR_IR,
	COLOR_COUNT,
};
enum {
	CIS_TOP,
	CIS_BOTTOM,
	CIS_COUNT,
};
enum {
	CIS_IMAGE_WIDTH = 720,
	CIS_IMAGE_HEIGHT = 360,
	CIS_DATA_WIDTH = 16,
};

#pragma pack(push)
#pragma pack(1)
// ͼ��У׼��4ά����
struct CISCorrectionTable 
{
	unsigned char data[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH][256];
};
#pragma pack(pop)

enum WaveDataLevel
{
	WAVE_NO_DATA,
	WAVE_HAS_DATA,
};

enum ImageDataLevel
{
	IMAGE_NO_DATA,
	IMAGE_FULL_DATA,
	IMAGE_DOWNSAMPLE_DATA,
};

#pragma pack(push)
#pragma pack(1)
struct DataLevel
{
	sockaddr_in    hostAddr;
	WaveDataLevel  waveDataLevel;
	ImageDataLevel imageDataLevel;
};
#pragma pack(pop)

// class ReceiveResult
class ReceiveResult 
{
public:
	ReceiveResult();
	virtual ~ReceiveResult();

	int GetID() const;
	int GetCount() const;
	int GetDataLength() const;
	const void *GetData() const;
	int GetData(void *buffer, int size);
	void *GetDataBuffer(int length);
	void SetID(int code);
	void SetCount(int cnt);

private:
	enum { STATIC_BUFFER_SIZE = 128 };

	int   m_id;
	int   m_count;
	char  m_staticBuffer[STATIC_BUFFER_SIZE];
	char  *m_dataBuffer;
	int   m_dataLength;
};

// class DeviceProxy
class DeviceProxy
{
private:
	DeviceProxy();
	virtual ~DeviceProxy();
	static DeviceProxy* m_device;
public:
	static DeviceProxy* GetInstance()
	{
		if (m_device == NULL)
		{
			m_device = new DeviceProxy();
		}
		return m_device;
	}
	static void ReleaseInstance()
	{
		if (m_device != NULL)
		{
			delete m_device;
			m_device = NULL;
		}
	}
	struct CurCommand//���ڴ洢��ǰ���͵�ָ��id��״̬
	{
		unsigned short id;
		unsigned short status;
	};
private:
	ConnectState m_connectState{ STATE_DEVICE_CLOSED }; // �����ӱ�־
	std::thread m_thrConnect; // �����߳̾��
	std::thread m_thrSendCash; // ���ͳ�Ʊ���ݾ��
	bool m_connecting{ false }; //�߳�connect�ɷ��˳��ı�־
	bool m_sendingCash{ false }; //�߳�sendcash�ɷ��˳��ı�־
	TcpSocket m_connect; // �������ӵ�sock
	TcpSocket m_sendCash; // ���ڷ��ͳ�Ʊ���ݵ�sock
	CurCommand m_curCommand; // ��ǰ���ڽ��е�ָ��
	CriticalSection m_criSecConnect;
	CriticalSection m_criSecSendCash;
	unsigned long long m_cashCnt{ 0 };
public:
	bool Start();
	bool Stop();
	ConnectState GetConnectState()
	{
		return m_connectState;
	}
	CurCommand GetCurCommand()
	{
		return m_curCommand;
	}

	bool StopSendingCash();
private:
	//ָ������봦���߳�
	bool WINAPI Process();

	bool StartSendingCash();
	bool SendCashDataProc();
	bool SendStartRunCashSignal();
	bool SendStopRunCashSignal();
	bool SendADCData(CString filepath);
	bool SendCISData(CString filepath);
	bool SendCashInfo(CashInfo cashInfo);

	bool ReceiveCommand(ReceiveResult* result);

	bool SendResponse(ReceiveResult* result);
	bool SendResponse(unsigned short id, unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendResponse(unsigned long cnt, unsigned short status, const void* data, int dataLength);

	bool SendEcho(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendUpgradeDebugState(unsigned long cnt, const void* recvData, int recvDataLength);

	bool SendDeviceInfo(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendCISCorrectionTable(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendSetTime(unsigned long cnt, const void* recvData, int recvDataLength);

	bool SendUpgrade(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendUpgradeData(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendRestart(unsigned long cnt, const void* recvData, int recvDataLength);

	bool SendSetIRParameters(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendGetIRValues(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendUpdateIRParameters(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendStartMotor(unsigned long cnt, const void* recvData, int recvDataLength);

	bool SendStartRunCashDetect(unsigned long cnt, const void* recvData, int recvDataLength);
};

