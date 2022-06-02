// ServerProcess.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

#define INPUT_SIZE  4096   //内存空间的大小
#define PROCESS_NAME  L"MemoryShare"  //访问地址的标志，类似于字典中的key
#define  _CRT_SECURE_NO_WARNINGS 

using namespace std;

int main()
{
	char input_str[] = "test data";//存放的数据
	//1.创建共享内存
	HANDLE hFileMap_Handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, INPUT_SIZE, PROCESS_NAME);
	//2.得到共享内存的指针
	LPVOID share_memory_ptr = MapViewOfFile(hFileMap_Handle, FILE_MAP_ALL_ACCESS, 0, 0, INPUT_SIZE);
	//3.把需要客户端读取的数据放在共享内存的地址上

	strcpy((char*)share_memory_ptr, input_str);

	Sleep(5000000);//设置等待时间，即为内存的存放时间，等待完就释放

	//4.解除映射
	UnmapViewOfFile(share_memory_ptr);
	//5.关闭句柄
	CloseHandle(hFileMap_Handle);

	cout << "访问结束" << endl;
}