#pragma once
#include <mutex>
using namespace std;


class CLock {
public:
	void lock() { m_mtx.lock(); }	 //����
	void unlock() { m_mtx.unlock(); } //����

private:
	mutex m_mtx;  //������
};


class CLogger
{
public:
	CLogger();
	~CLogger();
	bool Init(const string& strLogDir);
	void WriteLog(const string& strMsg);

private:
	CLock m_lockerWrite;			//������д��־
	string m_strLogFilePath;
};

#pragma once
