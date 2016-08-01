#pragma once

class TcpSocket {
public:
  TcpSocket();
  virtual ~TcpSocket();
  
  bool Connect(LPCTSTR address, int port, int milliseconds = 2000);

  // ���룺���ص�ַ
  // ���ܣ��󶨵�ַ���������״̬��������������е���δ����������������Ϊ1
  bool Listen(LPCTSTR address, int port);
  bool Listen(const sockaddr *addr, int addrLength);

  // ���룺�ȴ�������
  // ���ܣ����ڸ����ĺ����ڼ�������δ��������ӣ���Ϊ������Ӵ����µ��׽��֣����������ľ��
  SOCKET Accept(int waitMilliseconds);

  // ���룺����
  // ���ܣ����ý��գ�send���ͷ��ͣ�recv����ʱ��
  bool SetTimeout(int milliseconds);

  // ���룺����size
  // ���ܣ�����socket���������������շ��������Ĵ�СΪsize�ֽ�
  bool SetSendBufferSize(int size);
  bool SetRecvBufferSize(int size);

  // ����size�ֽڵ����ݣ���ŵ�������buffer��
  // ���ܴ��ڽ��յ���������size�ֽڵ����
  // ����ֵ�����ؽ��յ����ֽ���
  int Receive(void *buffer, int size);
  // ����size�ֽڵ����ݣ���ŵ�������buffer��
  // �� ���յ���������size�ֽ� �ͳ�ʱ���� ���ش��󣨵�֮ǰ���յĲ��ֲ���ɾ����
  bool ReceiveFully(void *buffer, int size);
  
  // �������ݣ���ʵ�ʷ��͵����ݡ�size�ֽڣ��򷵻ش���
  bool Send(const void *data, int size);

  // �ر��׽��֣������׽��ָ�ΪINVALID
  void Close();

  // ����׽����Ƿ��Ѿ�����
  bool IsOpened() const;

  // ��ͻ����Ӻ�ᴴ���µ��׽��֣���ԭ�����׽��ֹرգ��������µ��׽�����Ϊ����׽���
  void Attach(SOCKET s);
  
  // ��������׽��ֵĵ�ַ�͵�ַ����
  bool GetAddress(sockaddr *addr, int *addrLen);

  // ����TCP�������˼��ͻ����Ƿ����������
  bool ListenClosed();

private:
  SOCKET  m_socket;
};