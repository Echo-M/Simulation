#pragma once

#include "TcpSocket.h"
#include <thread>

#define WM_STATE_CONNECT_CHANGED	WM_USER + 1

#define SERIAL_NUMBER_LENGTH		24 //序列号长度
#define MODEL_NAME_LENGTH			24 //型号长度
#define FIRMWARE_VERSION_LENGTH		12 //固件版本长度
#define DISABLED_VALUE			    -1

// 连接状态
typedef enum ConnectState_
{
	STATE_DEVICE_CLOSED,
	STATE_DEVICE_LISTENING,
	STATE_DEVICE_CONNECTED,
	STATE_DEVICE_UNCONNECTED
} ConnectState;

#pragma pack(push)
#pragma pack(1)
// 设备信息
struct DeviceInfo 
{
	char sn[SERIAL_NUMBER_LENGTH]; // 点钞机序列号
	char model[MODEL_NAME_LENGTH]; // 点钞机型号
	char firmwareVersion[FIRMWARE_VERSION_LENGTH]; // 固件版本
	byte numberOfCIS; // CIS个数
	byte numberOfIR; // 红外对管的个数
	byte numberOfMH; // 磁头个数
	byte reserved;
	int cisColorFlags; //  ir/r/b/g  = bit  3/2/1/0
	short cisImageWidth;
	short cisImageHeight;
	int selfTestState; // 机器自检状态
	int debugState[16]; // 调试状态
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
// 下位机回应指令
typedef struct ResponseHeader_
{
	unsigned char signatures[2];  //表示一个响应包,固定为RP
	unsigned short status;   //表示指令执行状态：0表示成功，否则为错误代码。
	unsigned long count;	//表示请求计数
	unsigned long length;   //表示返回数据的长度
} ResponseHeader;
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
// 上位机发送指令
typedef struct RequestHeader_
{
	unsigned char signatures[2];   //表示一个请求包,固定为RQ
	unsigned short shortid;  //表示这个指令的标识
	unsigned long count;  //表示请求计数
	unsigned long length;  //表示这次发送的负载的长度
} RequestHeader;
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
// 图像校准表，4维数组
struct CISCorrectionTable 
{
	unsigned char data[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH][256];
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
private:
	ConnectState m_connectState{ STATE_DEVICE_CLOSED }; // 已连接标志
	std::thread m_thrConnect; // 连接线程句柄
	bool m_connecting{ false }; //线程connect可否退出的标志
	TcpSocket m_connect; // 用于连接
public:
	bool Start();
	bool Stop();
	ConnectState GetDeviceState()
	{
		return m_connectState;
	}
private:
	bool WINAPI Process();
	bool ReceiveCommand(ReceiveResult* result);
	bool SendResponse(ReceiveResult* result);
	bool SendResponse(unsigned short id, unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendResponse(unsigned long cnt, unsigned short status, const void* data, int dataLength);
	bool SendDeviceInfo(unsigned long cnt, const void* recvData, int recvDataLength);
	bool SendCISCorrectionTable(unsigned long cnt, const void* recvData, int recvDataLength);
	// 设置下位机时间
	bool SendSetTime(unsigned long cnt, const void* recvData, int recvDataLength);
	// Echo指令
	bool SendEcho(unsigned long cnt, const void* recvData, int recvDataLength);

};

