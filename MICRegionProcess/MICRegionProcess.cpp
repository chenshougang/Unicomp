// AARegionProcess.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "SheetMICRegionProcess.h"

#define DLL_EXPORT_API __declspec(dllexport)


extern "C" DLL_EXPORT_API CBaseProcess * CreateProcessModule()
{
	return new CSheetMICRegionProcess();
}
