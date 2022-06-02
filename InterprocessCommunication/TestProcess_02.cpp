// ClientProcess.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <Windows.h>
#define INPUT_SIZE  4096   //内存地址的大小
#define PROCESS_NAME  L"MemoryShare"
#define  _CRT_SECURE_NO_WARNINGS 

using namespace std;

int main()
{
	//打开共享的文件对象
	HANDLE hFileMap_Handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, PROCESS_NAME);
	//判断句柄是否存在
	if (hFileMap_Handle)
	{
		LPVOID lpBase = MapViewOfFile(hFileMap_Handle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		char szBuffer[INPUT_SIZE] = { 0 };
		strcpy(szBuffer, (char*)lpBase);

		while (1) {
			cout << "读取的数据为：" << szBuffer << endl;
		}

		//解除映射
		UnmapViewOfFile(lpBase);
		//关闭句柄
		CloseHandle(hFileMap_Handle);
	}

}