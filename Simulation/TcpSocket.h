#pragma once

class TcpSocket {
public:
  TcpSocket();
  virtual ~TcpSocket();
  
  bool Connect(LPCTSTR address, int port, int milliseconds = 2000);

  // 输入：本地地址
  // 功能：绑定地址，进入监听状态，允许监听队列中的尚未处理的最大连接数量为1
  bool Listen(LPCTSTR address, int port);
  bool Listen(const sockaddr *addr, int addrLength);

  // 输入：等待毫秒数
  // 功能：若在给定的毫秒内监听到了未处理的连接，就为这个连接创建新的套接字，并返回它的句柄
  SOCKET Accept(int waitMilliseconds);

  // 输入：毫秒
  // 功能：设置接收（send）和发送（recv）的时限
  bool SetTimeout(int milliseconds);

  // 输入：整数size
  // 功能：设置socket缓冲区，并设置收发缓冲区的大小为size字节
  bool SetSendBufferSize(int size);
  bool SetRecvBufferSize(int size);

  // 接收size字节的数据，存放到缓冲区buffer中
  // 可能存在接收的数据量＜size字节的情况
  // 返回值：返回接收到的字节数
  int Receive(void *buffer, int size);
  // 接收size字节的数据，存放到缓冲区buffer中
  // 若 接收的数据量＜size字节 就超时，则 返回错误（但之前接收的部分不会删除）
  bool ReceiveFully(void *buffer, int size);
  
  // 发送数据，若实际发送的数据≠size字节，则返回错误
  bool Send(const void *data, int size);

  // 关闭套接字，并将套接字赋为INVALID
  void Close();

  // 检查套接字是否已经创建
  bool IsOpened() const;

  // 与客户连接后会创建新的套接字，将原来的套接字关闭，并将该新的套接字作为类的套接字
  void Attach(SOCKET s);
  
  // 获得现在套接字的地址和地址长度
  bool GetAddress(sockaddr *addr, int *addrLen);

  // 用于TCP服务器端检查客户端是否放弃了连接
  bool ListenClosed();

private:
  SOCKET  m_socket;
};