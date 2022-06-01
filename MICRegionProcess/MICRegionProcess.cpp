// AARegionProcess.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "SheetMICRegionProcess.h"

#define DLL_EXPORT_API __declspec(dllexport)


extern "C" DLL_EXPORT_API CBaseProcess * CreateProcessModule()
{
	return new CSheetMICRegionProcess();
}
