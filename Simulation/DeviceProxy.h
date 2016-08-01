#pragma once

#include "TcpSocket.h"
#include <thread>

#define WM_STATE_DEVICE_CHANGED	WM_USER+1


class DeviceProxy
{
public:
	typedef enum
	{
		STATE_DEVICE_CLOSED,
		STATE_DEVICE_LISTENING,
		STATE_DEVICE_CONNECTED,
		STATE_DEVICE_UNCONNECTED
	} ConnectState;
private:
	ConnectState m_connectState{ STATE_DEVICE_CLOSED }; // �����ӱ�־
	std::thread m_thrConnect; // �����߳̾��
	bool m_connecting{ false }; //�߳�connect�ɷ��˳��ı�־
	TcpSocket m_connect; // ��������
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
	bool TryConnect();
	ConnectState GetDeviceState()
	{
		return m_connectState;
	}
	bool Close();
protected:
	bool WINAPI connect();
};
