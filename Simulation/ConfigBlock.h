#pragma once

// ��������CONFIG�ļ�
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
	// ��������ļ��Ƿ����
	bool IsNull() const;
	// ��ȡ�����ļ���·��
	CString GetConfigPath() const;

private:
	CString m_configPath;// �洢ini�ļ��ľ���·��
};