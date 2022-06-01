// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
// Windows ͷ�ļ�: 
#include <windows.h>
#include <fstream>
#include <Python.h>
#include <numpy/arrayobject.h>
#include <opencv.hpp>

//���ýӿ�
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
