#pragma once
#include <mutex>
using namespace std;


class CLock {
public:
	void lock() { m_mtx.lock(); }	 //加锁
	void unlock() { m_mtx.unlock(); } //解锁

private:
	mutex m_mtx;  //互斥量
};


class CLogger
{
public:
	CLogger();
	~CLogger();
	bool Init(const string& strLogDir);
	void WriteLog(const string& strMsg);

private:
	CLock m_lockerWrite;			//用于锁写日志
	string m_strLogFilePath;
};

#pragma once
