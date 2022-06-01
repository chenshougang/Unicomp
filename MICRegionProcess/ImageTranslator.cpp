#include "stdafx.h"
#include "ImageTranslator.h"

int init_numpy()
{
	import_array();
}


CImageTranslator::CImageTranslator(string modelPath, int netWidth, int netHeight)
{
	_netWidth = netWidth;
	_netHeight = netHeight;
	_inited = false;

	Py_SetPythonHome(L"D:\\anaconda3\\envs\\shilan");
	Py_Initialize();
	init_numpy();

	/*if (PyArray_API == NULL) {
		_import_array();
	}*/
	//_import_array();

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");
	//PyRun_SimpleString("sys.path.append('C:/项目/4.昆山淳华/2.C++代码/x64/Debug')");

	// 检查初始化是否成功  
	if (!Py_IsInitialized()) {
		_errorMessage = "python init fail";
		return;
	}
	else
	{
		//PyEval_InitThreads();   // 开启多线程支持
		//int nInit = PyEval_ThreadsInitialized();
		//if (nInit)
		//{
		//	PyEval_SaveThread();
		//}
	}

	//设置exe目录为当前目录
	//char path[MAX_PATH];
	//GetModuleFileNameA(NULL, path, MAX_PATH);
	//(strrchr(path, '\\'))[1] = 0;
	//SetCurrentDirectoryA(path);
	//GetCurrentDirectoryA(MAX_PATH, path);

	//int nHold = PyGILState_Check();  //检测当前线程是否拥有GIL
	//PyGILState_STATE gstate;
	//if (!nHold)
	//{
	//	gstate = PyGILState_Ensure(); // 如果没有GIL，则申请获取GIL
	//}
	//Py_BEGIN_ALLOW_THREADS;
	//Py_BLOCK_THREADS;

	PyObject* pyModel = PyImport_ImportModule("image_translator");
	if (!pyModel)
	{
		_errorMessage = "py import model fail";
		return;
	}
	PyObject* pDict = PyModule_GetDict(pyModel);
	if (!pDict)
	{
		_errorMessage = "PyModule_GetDict fail";
		return;
	}
	PyObject* pClass = PyDict_GetItemString(pDict, "ImageTranslator");
	if (!pClass || !PyCallable_Check(pClass))
	{
		_errorMessage = "PyDict_GetItemString fail";
		return;
	}

	PyObject* pArgs = PyTuple_New(3);
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", (const char*)modelPath.c_str()));
	PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", netWidth));
	PyTuple_SetItem(pArgs, 2, Py_BuildValue("i", netHeight));
	_pyTranslator = PyObject_CallObject(pClass, pArgs);
	//_pyTranslator = PyObject_CallFunctionObjArgs(pClass, pArgs);
	if (!_pyTranslator)
	{
		_errorMessage = "PyObject_CallObject fail";
		return;
	}
	//Py_UNBLOCK_THREADS;
	//Py_END_ALLOW_THREADS;
	//if (!nHold)
	//{
	//	PyGILState_Release(gstate);//释放当前线程的GIL
	//}

	//Py_DECREF(pyModel);
	//Py_DECREF(pDict);
	//Py_DECREF(pClass);
	//Py_DECREF(pArgs);

	_inited = true;
}

CImageTranslator::CImageTranslator()
{
	return;
}

bool CImageTranslator::Translate(uchar* imageDataSource, int width, int height, uchar* imageDataTranslated)
{
	_sMutex.lock();
	//int nHold = PyGILState_Check();  //检测当前线程是否拥有GIL
	//PyGILState_STATE gstate;
	//if (!nHold)
	//{
	//	gstate = PyGILState_Ensure(); // 如果没有GIL，则申请获取GIL
	//}
	//Py_BEGIN_ALLOW_THREADS;
	//Py_BLOCK_THREADS;
	npy_intp Dims[2] = { height, width };
	PyObject* PyArray = PyArray_SimpleNewFromData(2, Dims, NPY_UBYTE, imageDataSource);
	PyObject* ArgArray = PyTuple_New(1);
	PyTuple_SetItem(ArgArray, 0, PyArray);

	PyObject* pReturn = PyObject_CallMethod(_pyTranslator, "translate", "O", ArgArray);
	PyArrayObject* pArray;
	PyArray_OutputConverter(pReturn, &pArray);
	npy_intp* shape = PyArray_SHAPE(pArray);
	uchar* array_3 = (uchar*)PyArray_DATA(pArray);
	memcpy(imageDataTranslated, array_3, width * height);

	Py_DECREF(pReturn);
	//Py_DECREF(array_3);

	_sMutex.unlock();
	//Py_UNBLOCK_THREADS;
	//Py_END_ALLOW_THREADS;
	//if (!nHold)
	//{
	//	PyGILState_Release(gstate);//释放当前线程的GIL
	//}
	return true;
}


CImageTranslator::~CImageTranslator()
{
	//if (NULL != _pyTranslator)
	//{
	//	Py_DECREF(_pyTranslator);
	//	_pyTranslator = NULL;
	//}
	//Py_DECREF(_pyTranslator);
	//Py_Finalize();
	// 
	//Py_CLEAR(_pyTranslator);
	//Py_DECREF(_pyTranslator);
	//PyImport_Cleanup();
}

CImageTranslator* CImageTranslator:: getInstance(string modelPath, int netWidth, int netHeight)
{
	static CImageTranslator instance(modelPath, netWidth, netHeight);
	return &instance;
}

