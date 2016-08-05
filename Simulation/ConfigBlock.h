#pragma once

// 用来操作CONFIG文件
class ConfigBlock
{
private:
	ConfigBlock();
	virtual ~ConfigBlock();
	static ConfigBlock* m_config;
public:
	static ConfigBlock* GetInstance()
	{
		if (m_config == NULL)
		{
			m_config = new ConfigBlock();
		}
		return m_config;
	}
	static void ReleaseInstance()
	{
		if (m_config != NULL)
		{
			delete m_config;
			m_config = NULL;
		}
	}
public:
	int GetIntParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, int defaultValue);
	bool SetIntParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, int value);
	CString GetStringParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR defaultValue);
	bool SetStringParameter(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR value);
	// 检查配置文件是否存在
	bool IsNull() const;
	// 获取配置文件的路径
	CString GetConfigPath() const;

private:
	CString m_configPath;// 存储ini文件的绝对路径
};