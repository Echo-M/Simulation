#include "stdafx.h"
#include "ConfigBlock.h"

ConfigBlock* ConfigBlock::m_config = new ConfigBlock();

ConfigBlock::ConfigBlock()
{
	//获取执行程序路径
	TCHAR szPath[MAX_PATH];
	::GetModuleFileName(NULL, szPath, MAX_PATH);
	m_configPath = szPath;
	int index = m_configPath.ReverseFind(_T('\\'));
	m_configPath = m_configPath.Left(index + 1);
	m_configPath += L"CONFIG.ini";

	// 初始化
	////SetStringParameter(L"DeviceAddress", L"ip", L"192.168.8.131");
	////SetIntParameter(L"DeviceAddress", L"port", 1234);
	//SetIntParameter(L"UpgradePara", L"saveFile", 0);
	//SetStringParameter(L"UpgradePara", L"savePath", m_configPath.Left(m_configPath.ReverseFind(_T('\\')) + 1) + L"firmware.dat");
	//SetIntParameter(L"UpgradePara", L"recvLength", 1000);
	//SetIntParameter(L"UpgradePara", L"length", 10000);
	//SetIntParameter(L"DebugState", L"saveFile", 0);
	//SetStringParameter(L"DebugState", L"savePath", m_configPath.Left(m_configPath.ReverseFind(_T('\\')) + 1) + L"debugState.dat");
	//GetIntParameter(L"UpgradePara", L"flag", 0);
}

ConfigBlock::~ConfigBlock()
{

}

int ConfigBlock::GetIntParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, int defaultValue)
{
	if (IsNull())
	{
		return defaultValue;
	}
	
	return GetPrivateProfileInt(lpAppName, lpKeyName, defaultValue, m_configPath);
}

bool ConfigBlock::SetIntParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, int value)
{
	if (IsNull())
	{
		return false;
	}

	TCHAR valBuf[16];
	_itot(value, valBuf, 10);
	return WritePrivateProfileString(lpAppName, lpKeyName, valBuf, m_configPath);
}

CString ConfigBlock::GetStringParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR defaultValue)
{
	if (IsNull())
	{
		return false;
	}

	TCHAR szValue[MAX_PATH] = { 0 };
	GetPrivateProfileString(lpAppName, lpKeyName, defaultValue, szValue, _countof(szValue), m_configPath);
	if (_tcscmp(szValue, TEXT("")) == 0)
	{
		return defaultValue;
	}
	else
	{
		return CString(szValue);
	}
}

bool ConfigBlock::SetStringParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR value)
{
	if (IsNull())
	{
		return false;
	}

	// 再在当前配置下读取数据
	return WritePrivateProfileString(lpAppName, lpKeyName, value, m_configPath);
}

bool ConfigBlock::IsNull() const
{
	if (_tcscmp(m_configPath, TEXT("")) == 0)
	{
		return true;
	}

	// 检查配置文件是否存在
	return !PathFileExists(m_configPath);
}