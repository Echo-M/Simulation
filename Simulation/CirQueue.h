#ifndef CCIRQUEUE_H
#define CCIRQUEUE_H

#include <memory>
#include <atomic>

#include "IBuffer.h"

class CCirQueue : public IBuffer
{
    //��������
public:
    ///
    /// \brief push
    /// �������,�ڴ濽��
    /// \param _buf,��������
    /// \param _size,���������
    /// \return, 0=success
    ///
    int push_back(const unsigned char* _buf, int _size);

    ///
    /// \brief Pop
    /// ����
    /// \param _len,����������
    /// \return
    ///
    int pop_front(int _len);

    ///
    /// \brief Front
    /// ȡ���׵����ݣ������ڲ�����ָ��
    /// \param _buf,���ص�ַ
    /// \param _size,��ȡ������
    /// \return,0=success
    ///
    int front(unsigned char*& _buf, int _size);

    ///
    /// \brief ValidSize
    /// ���ض�������Ч������
    /// \return
    ///
    long long validSize();

    ///
    /// \brief FreeSize
    /// ���ض�����ʣ��������
    /// \return
    ///
    long long freeSize();

	long long totalSize() { return m_length; }

    void reset() {
        m_incount  = 0;
        m_outcount = 0;
        m_validNum = 0;
        m_curBlockPoint = 0;
    }

    //��չ����
public:
    CCirQueue();
    ~CCirQueue();

    ///
    /// \brief Initial
    /// ��ʼ���������ڴ棬����״̬
    /// \param _total,��������
    /// \param _outMaxSize,������������
    /// \return, 0=success, others=failed
    ///
    int Initial(long long _total, int _outMaxSize);

    ///
    /// \brief Memset
    /// \param _val
    /// \return
    ///
    int Memset(char _val=0);

    ///
    /// \brief GetInCount
    /// \return,�������������
    ///
    long long GetInCount() {return m_incount;}

    ///
    /// \brief GetOutCount
    /// \return,���س���������
    ///
    long long GetOutCount() {return m_outcount;}

    ///
    /// \brief QueueSize
    /// \return,���ض��д�С
    ///
    long long QueueSize() {return m_length;}

    ///
    /// \brief GetMaxUnitSize
    /// \return,����������������
    ///
    int GetMaxUnitSize() {return m_unitmaxsize;}

    ///
    /// \brief GetBlockPointer
    /// һ��ȡ��һ������ݣ��������Ӳ���,һ��ȡ������������Ҫ�ܱ�������������
    /// \param _size,ȡ��������������Ҫ�ܱ�������������
    /// \param _nRet,���ض��ӵ�������
    /// \return,���������������㹻�����ݣ���������ָ��,���򷵻�nullptr
    ///
    unsigned char* GetBlockPointer(int _size, long long &_nRet);

private:
    std::shared_ptr<unsigned char>	m_dataBuf{nullptr};	//�����ܻ���������
    std::shared_ptr<unsigned char>	m_tmpBuf{nullptr};	//��ʱ����Front������
    long long	m_length{0};	//�ڴ���г���
    int	m_unitmaxsize{0};	//front���������ڴ����󳤶�
    std::atomic<long long>	m_validNum{0};	//��������Ч���ݸ���
    std::atomic<long long>	m_incount{0};	//������ֽ���
    std::atomic<long long>	m_outcount{0};	//�ܳ����ֽ���
    std::atomic<long long>	m_curBlockPoint{0};	//��ǰ���ݻ����ȡλ��
};

#endif // CCIRQUEUE_H
