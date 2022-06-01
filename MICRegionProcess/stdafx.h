// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件: 
#include <windows.h>
#include <fstream>
#include <Python.h>
#include <numpy/arrayobject.h>
#include <opencv.hpp>

//公用接口
#include "../Common/XMLParse.h"
#include "../Common/GeneralFunction.h"
#include "../Common/DataObject.h"
#include "../Common/BaseProcess.h"
#include "../Common/CommonStruct.h"
#include "../Common/CommonFun.h"
#include "../Common/ParamDL.h"

using namespace cv;
using namespace std;
using namespace HalconCpp;
