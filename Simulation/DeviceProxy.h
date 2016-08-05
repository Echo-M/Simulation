#pragma once

#include "TcpSocket.h"
#include <thread>

#define WM_STATE_CONNECT_CHANGED	WM_USER + 1

#define SERIAL_NUMBER_LENGTH		24 //���кų���
#define MODEL_NAME_LENGTH			24 //�ͺų���
#define FIRMWARE_VERSION_LENGTH		12 //�̼��汾����
#define DISABLED_VALUE			    -1

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
	ConnectState m_connectState{ STATE_DEVICE_CLOSED }; // �����ӱ�־
	std::thread m_thrConnect; // �����߳̾��
	bool m_connecting{ false }; //�߳�connect�ɷ��˳��ı�־
	TcpSocket m_connect; // ��������
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
	// ������λ��ʱ��
	bool SendSetTime(unsigned long cnt, const void* recvData, int recvDataLength);
	// Echoָ��
	bool SendEcho(unsigned long cnt, const void* recvData, int recvDataLength);

};

