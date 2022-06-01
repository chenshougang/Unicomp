#pragma once


//class CImageTranslator
//{
//private:
//	int _netWidth;
//	int _netHeight;
//	string _errorMessage;
//	bool _inited;
//	PyObject* _pyTranslator = nullptr;
//	static std::mutex _sMutex;
//
//public:
//	CImageTranslator(string modelPath, int netWidth, int netHeight);
//	CImageTranslator();
//	bool Init(string modelPath, int netWidth, int netHeight);
//	bool Translate(uchar* imageDataSource, int width, int height, uchar* imageDataTranslated);
//	bool IsInited() { return _inited; }
//	string ErrorString() { return _errorMessage; }
//	~CImageTranslator();
//public:
//	virtual bool Check(uchar* imageSrc) { return false; }
//
//};


class CImageTranslator {
public:
    static CImageTranslator* getInstance(string modelPath, int netWidth, int netHeight);
    /* {
        static CImageTranslator instance(modelPath, netWidth, netHeight);
        return &instance;
        ////第一个检查，如果只是读操作，就不许用加锁
        //if (instance == nullptr) {
        //    std::lock_guard<std::mutex> lck(m_mutex);
        //    //第二个检查，保证只有一个
        //    if (instance == nullptr) {
        //        instance = new CImageTranslator(modelPath, netWidth, netHeight);
        //    }
        //}
        //return instance;
    }*/
    //static void delInstance() {
    //    if (instance != nullptr) {
    //        delete instance;
    //        instance = nullptr;
    //    }
    //}

    bool Translate(uchar* imageDataSource, int width, int height, uchar* imageDataTranslated);
    //bool IsInited() { return _inited; }
    //virtual bool Check(uchar* imageSrc) { return false; }
    //static CImageTranslator* instance;
private:

    int _netWidth;
    int _netHeight;
    string _errorMessage;
    bool _inited;
    PyObject* _pyTranslator;
    mutex _sMutex;

    CImageTranslator(string modelPath, int netWidth, int netHeight);
    CImageTranslator();
    ~CImageTranslator();

   /* static CImageTranslator* instance;
    class GarbageCollector {
    public:
        ~GarbageCollector() {
            if (CImageTranslator::instance) {
                delete CImageTranslator::instance;
                CImageTranslator::instance = 0;
            }
        }
    };
    static GarbageCollector gc;*/


    /*static CImageTranslator* instance;
    static std::mutex m_mutex;*/
};