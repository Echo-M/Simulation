#include "stdafx.h"
#include "DeviceProxy.h"

DeviceProxy* DeviceProxy::m_device = new DeviceProxy();

DeviceProxy::DeviceProxy()
{
}


DeviceProxy::~DeviceProxy()
{
}

bool DeviceProxy::connect()
{
	//PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_DEVICE_CHANGED, 0, 1);
	// ֻ�д��ڹر�״̬�ż���ִ��
	if (m_connectState != STATE_DEVICE_CLOSED)
		return false;

	if (!m_connect.Listen(_T("172.16.100.121"), 1234))
		return false;
								
	m_connectState = STATE_DEVICE_LISTENING;
	PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_DEVICE_CHANGED, 0, 1);

	// ��������
	while (m_connecting)
	{
		SOCKET s = m_connect.Accept(100);
		if (s != INVALID_SOCKET)// �յ����������ҽ��ճɹ�
		{
			m_connect.Attach(s);
			m_connectState = STATE_DEVICE_CONNECTED;
			//SendMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_DEVICE_CHANGED, NULL, 1);
			PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_DEVICE_CHANGED, 0, 1);
			break;
		}	
	}

	// ����Ƿ�Ͽ�����
	while (m_connecting)
	{
		if (m_connect.ListenClosed())
		{
			// ���ӶϿ���
			m_connect.Close();
			m_connectState = STATE_DEVICE_UNCONNECTED;
			PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_DEVICE_CHANGED, 0, 1);
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return true;
}

bool DeviceProxy::TryConnect()
{
	if (m_connecting) // ���߳�connect��������
	{
		return false;
	}
	m_connecting = true;
	m_thrConnect = std::thread([this](){this->connect(); });

	return true;
}

bool DeviceProxy::Close()
{
	m_connecting = false;
	m_connectState = STATE_DEVICE_CLOSED;
	PostMessage(AfxGetApp()->GetMainWnd()->GetSafeHwnd(), WM_STATE_DEVICE_CHANGED, 0, 1);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	if (m_thrConnect.joinable())
		m_thrConnect.join();
	m_connect.Close();
	return true;
}