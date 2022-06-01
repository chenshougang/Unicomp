#include "stdafx.h"
#include "Logger.h"

CLogger::CLogger()
{
}

CLogger::~CLogger()
{
}

bool CLogger::Init(const string& strLogDir)
{
	if (!DirectoryIfExists(strLogDir))
	{
		if (!CreateAllDirectory(strLogDir)) return false;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	char buf[50];
	sprintf_s(buf, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
	string strLogFileName = buf;
	m_strLogFilePath = strLogDir + "\\" + strLogFileName + ".log";

	return true;
}


void CLogger::WriteLog(const string& strMsg)
{
	m_lockerWrite.lock();

	ofstream outfile;
	outfile.open(m_strLogFilePath, ios::out | ios::app | ios::ate);
	if (outfile.is_open())
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		char buf[50];
		sprintf_s(buf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
		string strTime = buf;
		outfile << strTime.c_str() << "  " << strMsg << endl;
		outfile.close();
	}

	m_lockerWrite.unlock();
}