#include "stdafx.h"
#include "SheetMICRegionProcess.h"
#include <direct.h>

//cv::dnn::Net CSheetMICRegionProcess::net = cv::dnn::Net();

CSheetMICRegionProcess::CSheetMICRegionProcess()
{
	m_strRegionNode = "drawregion";
	m_strRegionName = "MICRegion";
    // 初始化opencv Net模块

    //Mat mat_Resize = Mat::zeros(Size(768, 768), CV_8UC3);

    //mat_Resize.convertTo(mat_Resize, CV_32F, 1.0 / 255.0);

    /*cv::Mat channels[3];
    cv::split(mat_Resize, channels);
    channels[0] = (channels[0] - 0.485) / 0.229;
    channels[1] = (channels[1] - 0.456) / 0.224;
    channels[2] = (channels[2] - 0.406) / 0.225;
    cv::merge(channels, 3, mat_Resize);*/

    //cv::Mat mat_NetInput;
    //cv::dnn::blobFromImage(mat_Resize, mat_NetInput);
    //net.setInput(mat_NetInput);

    //// DWORD start = GetTickCount();
    //net.forward("output");   //// 推理出的结果图，主要是在这个图上做处理

    //for (int i = 0; i < 3; i++);


    //cudaSetDevice(0);

    //_log = new CLogger();
}

CSheetMICRegionProcess::~CSheetMICRegionProcess()
{
    //cudaFree(0);

    //_translator->delInstance();
}

bool CSheetMICRegionProcess::LoadParam(const string& strXmlPath, CDataObject* pDataObject)
{
	//加载XML
	CXMLParse xmlParseObj;
	vector<RegionStruct> vecRegionStruct;
	bool ret = xmlParseObj.ReadXML(strXmlPath, m_strRegionNode, m_strRegionName, vecRegionStruct);
	if (!ret) return false;

	//初始化区域参数
	InitRegionParam(vecRegionStruct);

	//获取处理区ROI
	ret = GetProcessRegionROI(m_vecMICRegion);
	if (!ret) return false;

	////加载mask图
	//string strMaskPath = GetFileDirectory(strXmlPath) + "\\Mask";
	//const char* cMaskPath = strMaskPath.c_str();
	//HTuple hv_MaskPath(cMaskPath);

    //加载ROI Model图
    string strModelImgPath = GetFileDirectory(strXmlPath) + "ROI";
    const char* cModelPath = strModelImgPath.c_str();
    HTuple hv_ModelPath(cModelPath);

	try
	{
		//ReadImage(&ho_MaskImage, hv_MaskPath);
        ReadImage(&ho_ModelImage, hv_ModelPath);
	}
	catch (exception ex)
	{
		return false;
	}

	//初始化
	string strModelPath = GetFileDirectory(strXmlPath) + "model.pth";

    _translator = CImageTranslator::getInstance(strModelPath, 256, 256);
    
    //_translator = new CImageTranslator(strModelPath, 256, 256);

	_log.WriteLog("Succeed to Load Pb File!");

	return true;
}


bool CSheetMICRegionProcess::Process(CDataObject* pDataObject)
{
	if (pDataObject->IsOverMaxDefNum()) return false;  //已超最大缺陷数量
    
	const HObject ho_SrcImg = pDataObject->GetSrcImage().m_hoImage;
	if (false == ho_SrcImg.IsInitialized()) return false;

	HTuple hv_Num;
	CountChannels(ho_SrcImg, &hv_Num);
	if (hv_Num[0].I() != 1)
	{
		return false;  //只处理单通道图像
	}

	vector<DefectInfo> vecDefects;
	bool ret = true;

	ret = MICRegionProcess(pDataObject->GetSrcImage(), ho_ModelImage, m_vecMICRegion, vecDefects, pDataObject);

	if (!ret) return false;

	//写结果
	pDataObject->WriteDefsResult(vecDefects);

	return true;
}


void CSheetMICRegionProcess::Release()
{
	ClearProcessRegion(m_vecMICRegion);
    //if (NULL != _translator)
    //{
    //    delete _translator;
    //    _translator = NULL;
    //}

    //_translator->delInstance();
}


void CSheetMICRegionProcess::InitRegionParam(const vector<RegionStruct>& vecRegionStruct)
{
	ClearProcessRegion(m_vecMICRegion);

	for (int i = 0; i < vecRegionStruct.size(); i++)
	{
		ProcessRegion processregion;
		processregion.strRegionName = m_strRegionName;

		//guid
		processregion.strGUID = vecRegionStruct[i].strGUID;

		//绘制数据
		GetDrawParam(vecRegionStruct[i].vecDrawData, processregion.vecDrawParam);

		//算法参数
		processregion.pRegionAlgoParam = new MICRegionParam();
		GetMICRegionParam(vecRegionStruct[i], processregion.pRegionAlgoParam);

		m_vecMICRegion.push_back(processregion);
	}
}


void CSheetMICRegionProcess::GetMICRegionParam(const RegionStruct& regionStruct, void* pAlgoParam)
{
    string strParam[] = { "iROINum", "iCircleIndex", "iScaleRangeMin", "iScaleRangeMax", "iScaleRangeAIMin", "iScaleRangeAIMax", "fPadVoidRatio",
        "fPadSolderRatioSmall", "fPadSolderRatioBig", "fCircleVoidRatio", "iPadGrayMeanMax", "iDarkestGrayValue", "iZeroRegionArea", "iTinBeadsGray", "iTinBeadsArea"};
	int len = sizeof(strParam) / sizeof(strParam[0]);
	for (int i = 0; i < len; i++)
	{
		map<string, string>::const_iterator iter;
		iter = regionStruct.mapAlgorithm.find(strParam[i]);
		if (iter == regionStruct.mapAlgorithm.end()) continue;

		if (strParam[i] == "iROINum")
		{
			((MICRegionParam*)pAlgoParam)->iROINum = atoi(iter->second.c_str());
		}
        else if (strParam[i] == "iCircleIndex")
        {
            ((MICRegionParam*)pAlgoParam)->iCircleIndex = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iScaleRangeMin")
        {
            ((MICRegionParam*)pAlgoParam)->iScaleRangeMin = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iScaleRangeMax")
        {
            ((MICRegionParam*)pAlgoParam)->iScaleRangeMax = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iScaleRangeAIMin")
        {
            ((MICRegionParam*)pAlgoParam)->iScaleRangeAIMin = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iScaleRangeAIMax")
        {
            ((MICRegionParam*)pAlgoParam)->iScaleRangeAIMax = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "fPadVoidRatio")
        {
            ((MICRegionParam*)pAlgoParam)->fPadVoidRatio = atof(iter->second.c_str());
        }                                               
        else if (strParam[i] == "fPadSolderRatioSmall")              
        {                                               
            ((MICRegionParam*)pAlgoParam)->fPadSolderRatioSmall = atof(iter->second.c_str());
        }                                               
        else if (strParam[i] == "fPadSolderRatioBig")              
        {                                               
            ((MICRegionParam*)pAlgoParam)->fPadSolderRatioBig = atof(iter->second.c_str());
        }                                               
        else if (strParam[i] == "fCircleVoidRatio")              
        {                                               
            ((MICRegionParam*)pAlgoParam)->fCircleVoidRatio = atof(iter->second.c_str());
        }
        else if (strParam[i] == "iPadGrayMeanMax")
        {
            ((MICRegionParam*)pAlgoParam)->iPadGrayMeanMax = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iDarkestGrayValue")
        {
            ((MICRegionParam*)pAlgoParam)->iDarkestGrayValue = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iZeroRegionArea")
        {
            ((MICRegionParam*)pAlgoParam)->iZeroRegionArea = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iTinBeadsGray")
        {
            ((MICRegionParam*)pAlgoParam)->iTinBeadsGray = atoi(iter->second.c_str());
        }
        else if (strParam[i] == "iTinBeadsArea")
        {
            ((MICRegionParam*)pAlgoParam)->iTinBeadsArea = atoi(iter->second.c_str());
        }
	}
}


bool CSheetMICRegionProcess::MICRegionProcess(const ImageObject& objSrcImg, const HObject& ho_Model, vector<ProcessRegion> vecMICRegion,
	vector<DefectInfo>& vecDefsResult, CDataObject* pDataObject)
{
	const HObject ho_SrcImg = objSrcImg.m_hoImage;
	HObject ho_ModelImg = ho_Model.Clone();

	vector<NGRegion> vecNGRegions;

	const HObject ho_RegionROI = vecMICRegion[0].ho_RegionROI; //在加载参数时已判断是否为空
	const HObject ho_RegionROIMask = vecMICRegion[0].ho_RegionMask;   //单个区域的屏蔽区域

	const MICRegionParam* pAlgoParam = (MICRegionParam*)vecMICRegion[0].pRegionAlgoParam;

    //裁剪ROI
    HObject ho_ImageScale;
    scale_image_range(ho_SrcImg, &ho_ImageScale, pAlgoParam->iScaleRangeMin, pAlgoParam->iScaleRangeMax);   // 提取某个灰度值内的范围
    //WriteImage(ho_ImageScale, "tiff", 0, "C:/Users/csz/Desktop/新建文件夹/ho_ImageScale");
    ConvertImageType(ho_ImageScale, &ho_ImageScale, "byte");

    /*ScaleImageMax(ho_SrcImg, &ho_ImageScale);
    scale_image_range(ho_ImageScale, &ho_ImageScale, 40, 110);*/

    HObject ho_SrcImages, ho_DetectRegion, ho_MicPart, ho_ROTImage;
    HTuple  hv_HomMat2D, hv_HomMat2DBack, hv_DetectRegionNumber;

    ReduceDomain(ho_ImageScale, ho_RegionROI, &ho_ROTImage);

    CSheetMICRegionProcess::GetROI(ho_ROTImage, &ho_SrcImages, &ho_DetectRegion, &ho_MicPart, &hv_HomMat2D, &hv_HomMat2DBack);
    //WriteImage(ho_MicPart, "tiff", 0, "C:/Users/csz/Desktop/新建文件夹/ho_MicPart");
    HTuple hv_OKFlag;
    string sNGDes;
    CountObj(ho_DetectRegion, &hv_DetectRegionNumber);  // 有可能检测到一部分 MIC

    //void AreaCenter(const HObject & Regions, HTuple * Area, HTuple * Row, HTuple * Column)

    HTuple hv_Area_chen, hv_Row_chen, hv_Column_chen;
    AreaCenter(ho_DetectRegion, &hv_Area_chen, &hv_Row_chen, &hv_Column_chen);   // hv_Area_chen，Area_MIC = 196681 大概应该是这么多

    //int test_Area_chen = hv_Area_chen.I();

    //int test_hv_DetectRegionNumber = hv_DetectRegionNumber.I();

    if (hv_Area_chen <= 150000) {
        hv_DetectRegionNumber = 0;
    }
    if (hv_Area_chen > 150000) {
        hv_DetectRegionNumber = 1;
    }

    //test_Area_chen = hv_Area_chen.I();
    //test_hv_DetectRegionNumber = hv_DetectRegionNumber.I();
    
    if (0 != (hv_DetectRegionNumber == 0))  //提取BGA区域
    {
        hv_OKFlag = 0;
        DeletelistFiles("./folder_01");
        Sleep(50);
        WriteImage(ho_SrcImg, "tiff", 0, "./folder_01/temp");  // 写到文件夹中的图片是1536*1536的图片，16位图
        Sleep(50);
        ifstream fin_result("./folder_02/result.png");
        ifstream fin_src("./folder_02/src.png");
        //ifstream fin_result;
        //ifstream fin_src;
        while (!fin_result && !fin_src)
        {
            std::cout << " 两张图片有一张不存在 " << endl;
            Sleep(50);
            //fin_result("./folder_02/result.png");
            //fin_src("./folder_02/src.png");
            fin_result.open("./folder_02/result.png");
            fin_src.open("./folder_02/src.png");
            fin_result.good();
            fin_src.good();
        }
        if (fin_result && fin_src) {
            std::cout << "两张图片都存在有图片" << std::endl;
            fin_result.close();
            fin_src.close();
        }

        HObject ho_image_result, ho_image_src;
        Sleep(50);
        ReadImage(&ho_image_result, "./folder_02/result.png");   // 读进来的是检测结果图 分割图 
        Sleep(50);
        ReadImage(&ho_image_src, "./folder_02/src.png");   // 读进来的是检测结果图 分割图 
        Sleep(50);

        Mat mat_src_image = imread("./folder_02/src.png", 1);
        // Mat kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk = imread("./folder_02/result.png", 1);
        DeletelistFiles("./folder_02");

        // Local iconic variables
        HObject  ho_Region;
        HObject  ho_RegionErosion, ho_ConnectedRegions, ho_ObjectSelected;
        HObject  ho_ImageReduced, ho_ImageMean, ho_ImageScaleMax;
        HObject  ho_RegionVoid;
        HObject  ho_ImageZoom;

        // Local control variables
        HTuple  hv_Number, hv_VoidRatio, hv_Index, hv_RegionArea;
        HTuple  hv_VoidArea;

        Threshold(ho_image_result, &ho_Region, 255, 255);
        //ErosionCircle(ho_Region, &ho_RegionErosion, 0);   // 腐蚀参数可改
        Connection(ho_Region, &ho_ConnectedRegions);
        CountObj(ho_ConnectedRegions, &hv_Number);

        int temp_hv_Number = hv_Number.I();

        hv_VoidRatio = 0;
        vector<HTuple> vecVoidRatio;
        // 有 7 个 pad
        if (hv_Number.I() ==7)  // 说明七个BGA全部检测到了
        {
        
            HTuple end_val14 = hv_Number;
            HTuple step_val14 = 1;

            for (hv_Index = 1; hv_Index.Continue(end_val14, step_val14); hv_Index += step_val14)
            {
                SelectObj(ho_ConnectedRegions, &ho_ObjectSelected, hv_Index);
                ReduceDomain(ho_image_src, ho_ObjectSelected, &ho_ImageReduced);
                RegionFeatures(ho_ObjectSelected, "area", &hv_RegionArea);
                MeanImage(ho_ImageReduced, &ho_ImageMean, 3, 3);
                ScaleImageMax(ho_ImageReduced, &ho_ImageScaleMax);
                Threshold(ho_ImageScaleMax, &ho_RegionVoid, 160, 255);   // 气泡提取阈值可改 
                RegionFeatures(ho_RegionVoid, "area", &hv_VoidArea);
                hv_VoidRatio = hv_VoidArea / hv_RegionArea;
                vecVoidRatio.push_back(hv_VoidRatio);
                // WriteString(hv_WindowID, "NG");
            }
            hv_OKFlag = 0;

            vector<int> str_result;
            for (int i = 0; i < vecVoidRatio.size(); i++) {
                str_result.push_back(vecVoidRatio[i].D() * 100);
            }

            //cv::Point origin(50, 50);
            int k = 0;
            for (int i = 0; i < str_result.size(); i++) {
                k += 50;
                string temp_str = "pad " + to_string(i) + "= " + to_string(str_result[i]) + "%";
                putText(mat_src_image, temp_str, Point(20, k), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2, 2);
            }
        }

        if (hv_Number.I() > 7) {

            // 直接报NG
            hv_OKFlag = 1;
            putText(mat_src_image, "NG", Point(20, 300), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 0, 255), 2, 2);
        }


        if (hv_Number.I() < 7) {

            hv_OKFlag = 1;
            // 出现了短路或者虚焊的情况
            putText(mat_src_image, "NG", Point(40, 300), FONT_HERSHEY_SIMPLEX, 4, Scalar(0, 0, 255), 2, 2);
        }

        HObject final_result = Mat2HObject(mat_src_image);

        pDataObject->SetAffinedSrcImage(final_result, objSrcImg.m_iImageWid, objSrcImg.m_iImageHei);
        pDataObject->SetDetectImg(final_result, objSrcImg.m_iImageWid, objSrcImg.m_iImageHei);

        CalDefectInfo(ho_SrcImg, vecMICRegion[0].strGUID, sNGDes, hv_OKFlag.I(), REGION_NONE, DEF_MissOrExtra, vecDefsResult);
    }
    else
    {

    ///////////////////////////////
        scale_image_range(ho_MicPart, &ho_MicPart, pAlgoParam->iScaleRangeAIMin, pAlgoParam->iScaleRangeAIMax);
        //AI分割
        Mat imgAIInput;  // 定义输出图像
        imgAIInput = HObject2Mat(ho_MicPart);
        Size ROISize = Size(imgAIInput.cols, imgAIInput.rows);
        cv::resize(imgAIInput, imgAIInput, Size(256, 256), 0, 0, INTER_LINEAR);
        cv::Mat imgAIOutput = imgAIInput.clone();
		//cv::imwrite("C:/Users/csz/Desktop/xfwd/ho_ImageScale.tif", imgAIInput);
        //pDataObject->GetTranslator()->Translate(imgAIInput.data, imgAIInput.cols, imgAIInput.rows, imgAIOutput.data);
        _translator->Translate(imgAIInput.data, imgAIInput.cols, imgAIInput.rows, imgAIOutput.data);
        Mat outMask;
        normalize(imgAIOutput, outMask, 0, 255, NORM_MINMAX);
        outMask.convertTo(outMask, CV_8UC1);
        resize(outMask, outMask, ROISize);
        //mat转Hobject
        HObject ho_outMask = Mat2HObject(outMask);

        //WriteImage(ho_outMask, "bmp", 0, "C:/Users/csz/Desktop/新建文件夹/ho_outMask");

        //生成输入参数
        //IsRotateImage， ROIRowArrayInput，ROIColArrayInput，RingBreak_Switch，Connected_Switch
        HTuple hv_IsRotateImage, hv_ROIRowArrayInput, hv_ROIColArrayInput, hv_RingBreak_Switch, hv_Connected_Switch, hv_ROINameArrayInput, hv_Buble_thresh, hv_GroupName, hv_ToolName;
        HObject ho_MaskCircleRegion, ho_MaskCircleRegionConnect, ho_MaskPad, ho_MaskPadConnect, ho_RegionROIS, ho_ConnectedROISRegions;
        HTuple hv_MaskImageWidth, hv_MaskImageHeight, hv_AreaCircle, hv_RowCircle, hv_ColumnCircle, hv_MaskPadNumber;
        HTuple hv_ROIArea, hv_ROIRow, hv_ROIColumn;

        GetImageSize(ho_outMask, &hv_MaskImageWidth, &hv_MaskImageHeight);

        Threshold(ho_outMask, &ho_MaskCircleRegion, 128, 255);
        Connection(ho_MaskCircleRegion, &ho_MaskCircleRegionConnect);
        SelectShape(ho_MaskCircleRegionConnect, &ho_MaskPad, (HTuple("area").Append("row")), "and", (HTuple(250).Append(0)),
            HTuple(10000).TupleConcat(hv_MaskImageHeight / 2));

        DilationCircle(ho_MaskPad, &ho_MaskPad, 5);
        Connection(ho_MaskPad, &ho_MaskPadConnect);

        CountObj(ho_MaskPadConnect, &hv_MaskPadNumber);
        if (0 != (hv_MaskPadNumber > 2))
        {
            hv_IsRotateImage = 1;
            RotateImage(ho_ModelImg, &ho_ModelImg, 180, "constant");
        }
        else
        {
            hv_IsRotateImage = 0;
        }
       /* Threshold(ho_outMask, &ho_MaskCircleRegion, 0, 0);
        OpeningCircle(ho_MaskCircleRegion, &ho_ho_MaskCircleRegionOpening, 30);
        AreaCenter(ho_ho_MaskCircleRegionOpening, &hv_AreaCircle, &hv_RowCircle, &hv_ColumnCircle);
        if (0 != (hv_RowCircle < (hv_MaskImageHeight / 2)))
        {

            hv_IsRotateImage = 1;
            RotateImage(ho_ModelImg, &ho_ModelImg, 180, "constant");
        }
        else
        {

            hv_IsRotateImage = 0;
        }*/

        //ROIRowArrayInput，ROIColArrayInput
        Threshold(ho_ModelImg, &ho_RegionROIS, 128, 255);
        Connection(ho_RegionROIS, &ho_ConnectedROISRegions);
        AreaCenter(ho_ConnectedROISRegions, &hv_ROIArea, &hv_ROIRow, &hv_ROIColumn);
        hv_ROIRowArrayInput = hv_ROIRow;
        hv_ROIColArrayInput = hv_ROIColumn;

        //RingBreak_Switch，Connected_Switch
        hv_RingBreak_Switch = HTuple();
        hv_Connected_Switch = HTuple();
        {
            for (int i = 1; i <= pAlgoParam->iROINum; i++)
            {
                if (i == pAlgoParam->iCircleIndex)
                {
                    hv_RingBreak_Switch = hv_RingBreak_Switch.TupleConcat(1);
                    hv_Connected_Switch = hv_Connected_Switch.TupleConcat(0);
                }
                else
                {
                    hv_RingBreak_Switch = hv_RingBreak_Switch.TupleConcat(0);
                    hv_Connected_Switch = hv_Connected_Switch.TupleConcat(1);
                }
            }
        }
        
        hv_ROINameArrayInput[0] = 1;
        TupleGenConst(pAlgoParam->iROINum, 128, &hv_Buble_thresh);
        hv_GroupName = "";
        hv_ToolName = "";

        //输出
        HObject ho_ResImage;
        HTuple hv_ResultArray, hv_ResRowArray, hv_ResColArray, hv_BubbleAreaCircle, hv_TinArea, hv_TinAreaRatio, hv_BubbleArea, hv_BubbleRatio, hv_PadMeanArray, hv_GRRGrayArray, hv_CrackLength;

        //检测
        CSheetMICRegionProcess::MicCheckUnet(ho_ImageScale, ho_ModelImg, ho_DetectRegion, ho_outMask, &ho_ResImage,
            hv_ROINameArrayInput, hv_ROIRowArrayInput, hv_ROIColArrayInput, hv_RingBreak_Switch,
            hv_Connected_Switch, hv_HomMat2D, hv_HomMat2DBack, hv_IsRotateImage, hv_Buble_thresh,
            hv_GroupName, hv_ToolName, &hv_ResultArray, &hv_ResRowArray, &hv_ResColArray,
            &hv_BubbleAreaCircle, &hv_TinArea, &hv_TinAreaRatio, &hv_BubbleArea, &hv_BubbleRatio,
            &hv_PadMeanArray, &hv_GRRGrayArray, &hv_CrackLength);
        HTuple type;
        GetImageType(ho_ResImage, &type);
		//WriteImage(ho_ResImage, type, 0, "C:/Users/csz/Desktop/新建文件夹/res");
        

        pDataObject->SetAffinedSrcImage(ho_ResImage, objSrcImg.m_iImageWid, objSrcImg.m_iImageHei);
        pDataObject->SetDetectImg(ho_ResImage, objSrcImg.m_iImageWid, objSrcImg.m_iImageHei);

        

        HTuple hv_Sum;
        //CalDefectInfo(ho_SrcImg, vecMICRegion[i].strGUID, sNGDes, hv_OKFlag.I(), REGION_NONE, DEF_MissOrExtra, vecDefsResult);
        TupleSum(hv_ResultArray, &hv_Sum);
        if (hv_Sum >= 1)
        {
            hv_OKFlag = 1;
        }
        else
        {
            hv_OKFlag = 0;
        }

        HTuple hv_stringOut;
        hv_stringOut = "";
        {
            for (int outIndex = 1; outIndex <= pAlgoParam->iROINum; outIndex++)
            {
                hv_stringOut = ((((((((((((((((((((((hv_stringOut + "Index=") + outIndex) + HTuple(",")) + "Result=") +
                    HTuple(hv_ResultArray[outIndex - 1])) + HTuple(",")) + "TinArea=") + (HTuple(hv_TinArea[outIndex - 1]).TupleString(".1f"))) +
                    HTuple(",")) + "TinAreaRatio=") + (HTuple(hv_TinAreaRatio[outIndex - 1]).TupleString(".4f"))) + HTuple(",")) + "BubbleArea=") +
                    (HTuple(hv_BubbleArea[outIndex - 1]).TupleString(".1f"))) + HTuple(",")) + "BubbleRatio=") +
                    (HTuple(hv_BubbleRatio[outIndex - 1]).TupleString(".4f"))) + HTuple(",")) + "PadMeanArray=") +
                    (HTuple(hv_PadMeanArray[outIndex - 1]).TupleString(".1f"))) + HTuple(",")) + "GRRGrayArray=") +
                    (HTuple(hv_GRRGrayArray[outIndex - 1]).TupleString(".1f"));
                if (outIndex != pAlgoParam->iROINum)
                {
                    hv_stringOut += HTuple("|");
                }
            }
        }

        sNGDes = hv_stringOut.S();
        CalDefectInfo(ho_SrcImg, vecMICRegion[0].strGUID, sNGDes, hv_OKFlag.I(), REGION_NONE, DEF_MissOrExtra, vecDefsResult);

    }

	//CalDefectInfo(ho_SrcImg, vecBlackWhiteRegion[i].strGUID, hv_OKFlag.I(), REGION_NONE, DEF_MissOrExtra, vecDefsResult);

	if (vecNGRegions.size() > 0)
	{
		pDataObject->WriteNGRegion(vecNGRegions);
	}

	return true;
}


void CSheetMICRegionProcess::GetROI(HObject ho_InputImg, HObject* ho_SrcImages, HObject* ho_DetectRegion,
	HObject* ho_ImagePart, HTuple* hv_HomMat2D, HTuple* hv_HomMat2DBack)
{

	// Local iconic variables
	HObject  ho_Region, ho_ImageAffinTrans, ho_ImageReduced;

	// Local control variables
	HTuple  hv_Row, hv_Column, hv_Phi, hv_Length1;
	HTuple  hv_Length2, hv_Area, hv_Row1, hv_Column1, hv_Exception;

	try
	{
		CopyImage(ho_InputImg, &(*ho_SrcImages));
		Threshold(ho_InputImg, &ho_Region, 0, 90);
		FillUp(ho_Region, &(*ho_DetectRegion));
		Connection((*ho_DetectRegion), &(*ho_DetectRegion));

		SelectShape((*ho_DetectRegion), &(*ho_DetectRegion), (HTuple("area").Append("rectangularity")),
			"and", (HTuple(60000).Append(0.9)), (HTuple(199999).Append(1)));

		//select_shape_std (DetectRegion, DetectRegion, 'max_area', 70)

		SmallestRectangle2((*ho_DetectRegion), &hv_Row, &hv_Column, &hv_Phi, &hv_Length1,
			&hv_Length2);
		GenRectangle2(&(*ho_DetectRegion), hv_Row, hv_Column, hv_Phi, hv_Length1, hv_Length2);
		AreaCenter((*ho_DetectRegion), &hv_Area, &hv_Row1, &hv_Column1);
		if (0 != (hv_Area < 50000))
		{
			Threshold(ho_InputImg, &ho_Region, 0, 105);
			FillUp(ho_Region, &(*ho_DetectRegion));
			Connection((*ho_DetectRegion), &(*ho_DetectRegion));
			SelectShapeStd((*ho_DetectRegion), &(*ho_DetectRegion), "max_area", 70);
			SmallestRectangle2((*ho_DetectRegion), &hv_Row, &hv_Column, &hv_Phi, &hv_Length1,
				&hv_Length2);
			GenRectangle2(&(*ho_DetectRegion), hv_Row, hv_Column, hv_Phi, hv_Length1, hv_Length2);
		}
		if (0 != (hv_Phi < 0))
		{
			VectorAngleToRigid(hv_Row, hv_Column, hv_Phi, hv_Length1, hv_Length2, -3.14 / 2,
				&(*hv_HomMat2D));
			VectorAngleToRigid(hv_Length1, hv_Length2, -3.14 / 2, hv_Row, hv_Column, hv_Phi,
				&(*hv_HomMat2DBack));
		}
		else
		{
			VectorAngleToRigid(hv_Row, hv_Column, hv_Phi, hv_Length1, hv_Length2, 3.14 / 2,
				&(*hv_HomMat2D));
			VectorAngleToRigid(hv_Length1, hv_Length2, 3.14 / 2, hv_Row, hv_Column, hv_Phi,
				&(*hv_HomMat2DBack));
		}
		AffineTransRegion((*ho_DetectRegion), &(*ho_DetectRegion), (*hv_HomMat2D), "nearest_neighbor");
		AffineTransImage(ho_InputImg, &ho_ImageAffinTrans, (*hv_HomMat2D), "bilinear",
			"false");
		ReduceDomain(ho_ImageAffinTrans, (*ho_DetectRegion), &ho_ImageReduced);
		CropDomain(ho_ImageReduced, &(*ho_ImagePart));
		//
	}
	// catch (Exception) 
	catch (HException& HDevExpDefaultException)
	{
		HDevExpDefaultException.ToHTuple(&hv_Exception);
		CopyImage(ho_InputImg, &(*ho_ImagePart));
		return;
	}
	return;
}


void CSheetMICRegionProcess::MicCheckUnet(HObject ho_SrcImage, HObject ho_ImageModel, HObject ho_DetectRegion,
    HObject ho_ImageAffinTransMask, HObject* ho_ResImage, HTuple hv_ROINameArrayInput,
    HTuple hv_ROIRowArrayInput, HTuple hv_ROIColArrayInput, HTuple hv_RingBreak_Switch,
    HTuple hv_Connected_Switch, HTuple hv_HomMat2D, HTuple hv_HomMat2DBack, HTuple hv_IsRotateImage,
    HTuple hv_Buble_thresh, HTuple hv_GroupName, HTuple hv_ToolName, HTuple* hv_ResultArray,
    HTuple* hv_ResRowArray, HTuple* hv_ResColArray, HTuple* hv_BubbleAreaCircle,
    HTuple* hv_Out_TinArea, HTuple* hv_Out_TinAreaRatio, HTuple* hv_Out_BubbleArea,
    HTuple* hv_Out_BubbleRatio, HTuple* hv_PadMeanArray, HTuple* hv_GRRGrayArray,
    HTuple* hv_CrackLength)
{

    // Local iconic variables
    HObject  ho_EmptyObject, ho_EmptyRegion, ho_ImageAffinTrans;
    HObject  ho_DetectImage, ho_MaskImage, ho_Region, ho_ConnectedRegions;
    HObject  ho_ImageScaled, ho_MaskImageScaled, ho_ImageModelScaled;
    HObject  ho_Cross, ho_RegionModelScaled, ho_ConnectedModelScaled;
    HObject  ho_ImageMedian, ho_ImageEmphasize, ho_Partitioned;
    HObject  ho_RegionAI, ho_BoundingObj, ho_VoidOBJ, ho_TinOBJ;
    HObject  ho_CirOuterReal, ho_Circle, ho_SelectedROI, ho_BubbleRegion;
    HObject  ho_xizhaRegion, ho_ObjectSelected1, ho_ActivaRation;
    HObject  ho_RegionFillUp, ho_RegionDifference, ho_RegionTrans;
    HObject  ho_RegionErosion, ho_ImageReduced, ho_Region1, ho_ConnectedRegions1;
    HObject  ho_RegionTransConvex, ho_RegionDifferenceConvex;
    HObject  ho_RegionAIVoidTemp, ho_ImageAIVoidTemp, ho_RegionVoid;
    HObject  ho_PadBubbleRegion, ho_RegionDilation1, ho_RegionIntersection;
    HObject  ho_RegionDilation, ho_RegionTrans1, ho_ImageReducedTinAI;
    HObject  ho_RegionTinAI, ho_ImageReduced2, ho_ImageMean;

    //0308 人造连锡新增
    HObject ho_ImageReducedROI, ho_RegionZero, ho_RegionErosionROI, ho_RegionZeroerosion;
    HTuple hv_MinROI, hv_MaxROI, hv_RangeROI, hv_RegionZeroArea;

    // Local control variables
    HTuple  hv_Width, hv_Height, hv_Area1, hv_Row5;
    HTuple  hv_Column5, hv_AreaModel, hv_Row, hv_Column, hv_Scale;
    HTuple  hv_ScaleWidth, hv_ScaleHeight, hv_HomMat2DIdentity;
    HTuple  hv_HomMat2DScale, hv_ROIRowArrayScaled, hv_ROIColArrayScaled;
    HTuple  hv_ROINumber, hv_Width1, hv_Height1, hv_ROIIndex;
    HTuple  hv_CirROICenRow, hv_CirROICenCol, hv_BreakError;
    HTuple  hv_Numberxizha, hv_CircleTinArea, hv_Number, hv_ObjNum;
    HTuple  hv_ObjArea, hv_Row1, hv_Column1, hv_BubbleArea;
    HTuple  hv_UsedThreshold, hv_inner_radius_Value, hv_outer_radius_Value;
    HTuple  hv_Indices1, hv_I_O_radius_Value, hv_Less, hv_Row4;
    HTuple  hv_Column4, hv_Row3, hv_Column3, hv_NumberBubbleRegion;
    HTuple  hv_Row2, hv_Column2, hv_Number5, hv_Mean, hv_Deviation;
    HTuple  hv_Mean1, hv_Deviation1, hv_ReScale, hv_ReHomMat2DScale;
    HTuple  hv_RowTrans, hv_ColTrans, hv_okngFlag, hv_ResultArrayReJudge;
    HTuple  hv_Exception;

    const MICRegionParam* pAlgoParam = (MICRegionParam*)m_vecMICRegion[0].pRegionAlgoParam;

    try
    {
        //* 输出
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), -1, &(*hv_ResultArray));
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), 0.0, &(*hv_Out_TinArea));
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), 0.0, &(*hv_Out_TinAreaRatio));
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), 0.0, &(*hv_Out_BubbleArea));
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), 0.0, &(*hv_Out_BubbleRatio));
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), 0.0, &(*hv_PadMeanArray));
        TupleGenConst(hv_RingBreak_Switch.TupleLength(), 0.0, &(*hv_GRRGrayArray));
        (*hv_ResRowArray) = hv_ROIRowArrayInput;
        (*hv_ResColArray) = hv_ROIColArrayInput;
        //copy_image (SrcImage, DupImage1)
        //copy_image (SrcImage, DupImage2)
        //copy_image (SrcImage, DupImage3)

        //compose3 (DupImage1, DupImage2, DupImage3, ResImage)

        GenEmptyObj(&ho_EmptyObject);
        GenEmptyRegion(&ho_EmptyRegion);
        AffineTransImage(ho_SrcImage, &ho_ImageAffinTrans, hv_HomMat2D, "constant", "false");
        ReduceDomain(ho_ImageAffinTrans, ho_DetectRegion, &ho_DetectImage);
        ReduceDomain(ho_ImageAffinTransMask, ho_DetectRegion, &ho_MaskImage);
        CropDomain(ho_DetectImage, &ho_DetectImage);
        CropDomain(ho_MaskImage, &ho_MaskImage);
        GetImageSize(ho_DetectImage, &hv_Width, &hv_Height);
        ZoomImageSize(ho_ImageModel, &ho_ImageModel, hv_Width, hv_Height, "constant");
        Threshold(ho_ImageModel, &ho_Region, 128, 255);
        Connection(ho_Region, &ho_ConnectedRegions);
        AreaCenter(ho_ConnectedRegions, &hv_Area1, &hv_Row5, &hv_Column5);
        SortRegion(ho_ConnectedRegions, &ho_ConnectedRegions, "character", "true", "row");
        AreaCenter(ho_ConnectedRegions, &hv_AreaModel, &hv_Row, &hv_Column);

        if (0 != hv_IsRotateImage)
        {
            TupleInverse(hv_AreaModel, &hv_AreaModel);
            TupleInverse(hv_Row, &hv_Row);
            TupleInverse(hv_Column, &hv_Column);
        }

        hv_ROIRowArrayInput = hv_Row;
        hv_ROIColArrayInput = hv_Column;
        //*放缩
        hv_Scale = (HTuple(500).TupleReal()) / hv_Width;
        hv_ScaleWidth = 500;
        hv_ScaleHeight = hv_Scale * hv_Height;

        HomMat2dIdentity(&hv_HomMat2DIdentity);
        HomMat2dScale(hv_HomMat2DIdentity, hv_Scale, hv_Scale, 0, 0, &hv_HomMat2DScale);
        AffineTransImage(ho_DetectImage, &ho_ImageScaled, hv_HomMat2DScale, "bilinear",
            "true");
        AffineTransImage(ho_MaskImage, &ho_MaskImageScaled, hv_HomMat2DScale, "bilinear",
            "true");
        AffineTransImage(ho_ImageModel, &ho_ImageModelScaled, hv_HomMat2DScale, "bilinear",
            "true");

		/*WriteImage(ho_DetectImage, "tiff", 0, "C:/Users/csz/Desktop/新建文件夹/ho_DetectImage");
		WriteImage(ho_MaskImage, "tiff", 0, "C:/Users/csz/Desktop/新建文件夹/ho_MaskImage");
		WriteImage(ho_ImageModel, "tiff", 0, "C:/Users/csz/Desktop/新建文件夹/ho_ImageModel");*/

        hv_ROIRowArrayScaled = hv_ROIRowArrayInput * hv_Scale;
        hv_ROIColArrayScaled = hv_ROIColArrayInput * hv_Scale;
        GenCrossContourXld(&ho_Cross, hv_ROIRowArrayScaled, hv_ROIColArrayScaled, 6,
            0.785398);

        Threshold(ho_ImageModelScaled, &ho_RegionModelScaled, 128, 255);
        Connection(ho_RegionModelScaled, &ho_ConnectedModelScaled);
        CountObj(ho_ConnectedModelScaled, &hv_ROINumber);

        MedianImage(ho_ImageScaled, &ho_ImageMedian, "circle", 3, "mirrored");
        Emphasize(ho_ImageMedian, &ho_ImageEmphasize, 25, 25, 1);
        GetImageSize(ho_ImageScaled, &hv_Width1, &hv_Height1);
        PartitionRectangle(ho_ImageScaled, &ho_Partitioned, hv_Width1, hv_Height1 / 2);

        Threshold(ho_MaskImageScaled, &ho_RegionAI, 128, 255);
        Connection(ho_RegionAI, &ho_BoundingObj);

        GenEmptyObj(&ho_VoidOBJ);
        GenEmptyObj(&ho_TinOBJ);

        {
            HTuple end_val70 = hv_ROINumber - 1;
            HTuple step_val70 = 1;
            for (hv_ROIIndex = 0; hv_ROIIndex.Continue(end_val70, step_val70); hv_ROIIndex += step_val70)
            {
                //ROIIndex := 2
                GenEmptyObj(&ho_CirOuterReal);

                GenCircle(&ho_Circle, HTuple(hv_ROIRowArrayScaled[hv_ROIIndex]), HTuple(hv_ROIColArrayScaled[hv_ROIIndex]),
                    20);
                SelectShapeProto(ho_ConnectedModelScaled, ho_Circle, &ho_SelectedROI, "overlaps_abs",
                    1, 1500);

                if (0 != (HTuple(hv_RingBreak_Switch[hv_ROIIndex]) == 1))
                {
                    hv_CirROICenRow = HTuple(hv_ROIRowArrayScaled[hv_ROIIndex]);
                    hv_CirROICenCol = HTuple(hv_ROIColArrayScaled[hv_ROIIndex]);
                    RingBreakCheckUnet(ho_ImageScaled, ho_ImageEmphasize, ho_SelectedROI, ho_MaskImageScaled,
                        &ho_CirOuterReal, &ho_BubbleRegion, &ho_xizhaRegion, hv_IsRotateImage,
                        HTuple(hv_Buble_thresh[hv_ROIIndex]), hv_ROINameArrayInput, &hv_BreakError,
                        &(*hv_BubbleAreaCircle), &(*hv_CrackLength));
                    CountObj(ho_xizhaRegion, &hv_Numberxizha);
                    if (0 != (hv_Numberxizha > 0))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 7;
                    }
                    if (0 != (hv_BreakError == 1))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 8;
                    }
                    else if (0 != (hv_BreakError == 2))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 8;
                    }
                    if (0 != (HTuple((*hv_ResultArray)[hv_ROIIndex]) != 8))
                    {
                        if (0 != (HTuple((*hv_ResultArray)[hv_ROIIndex]) != 7))
                        {
                            (*hv_ResultArray)[hv_ROIIndex] = 0;
                        }
                    }
                    else if (0 != (HTuple((*hv_ResultArray)[hv_ROIIndex]) == 8))
                    {
                    }

                    //*新增
                    ConcatObj(ho_TinOBJ, ho_CirOuterReal, &ho_TinOBJ);
                    ConcatObj(ho_VoidOBJ, ho_BubbleRegion, &ho_VoidOBJ);
                    if (0 != (HTuple(HTuple((*hv_ResultArray)[hv_ROIIndex]) == 8).TupleAnd(hv_GroupName == HTuple("Block6"))))
                    {
                        (*hv_Out_TinArea)[hv_ROIIndex] = 4030.792307;
                    }
                    else
                    {
                        RegionFeatures(ho_CirOuterReal, "area", &hv_CircleTinArea);
                        (*hv_Out_TinArea)[hv_ROIIndex] = (((hv_CircleTinArea * 1.) / hv_Scale) * 1.) / hv_Scale;
                        (*hv_Out_BubbleArea)[hv_ROIIndex] = ((((*hv_BubbleAreaCircle) * 1.) / hv_Scale) * 1.) / hv_Scale;
                        (*hv_Out_BubbleRatio)[hv_ROIIndex] = ((*hv_BubbleAreaCircle) * 1.) / hv_CircleTinArea;
                    }

                    if (0 != (HTuple((*hv_ResultArray)[hv_ROIIndex]) == 8))
                    {
                        (*hv_CrackLength) = 1;
                    }
                }
                if (0 != (HTuple(hv_Connected_Switch[hv_ROIIndex]) == 1))
                {
                    SelectShapeProto(ho_BoundingObj, ho_SelectedROI, &ho_ObjectSelected1, "overlaps_rel",
                        10, 100);
                    SelectShapeProto(ho_ConnectedModelScaled, ho_ObjectSelected1, &ho_ActivaRation,
                        "overlaps_abs", 1000, 800000);
                    CountObj(ho_ActivaRation, &hv_Number);
                    if (0 != (hv_Number == 0))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 5;
                    }
                    if (0 != (hv_Number > 1))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 3;
                    }
                    if (0 != (HTuple((*hv_ResultArray)[hv_ROIIndex]) == -1))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 0;
                    }
                    else
                    {

                    }
                    //*********焊盘比例
                    CountObj(ho_ObjectSelected1, &hv_ObjNum);
                    if (0 != (hv_ObjNum != 0))
                    {
                        Union1(ho_ObjectSelected1, &ho_ObjectSelected1);
                        AreaCenter(ho_ObjectSelected1, &hv_ObjArea, &hv_Row1, &hv_Column1);
                        AreaHoles(ho_ObjectSelected1, &hv_BubbleArea);
                        FillUp(ho_ObjectSelected1, &ho_RegionFillUp);
                        Difference(ho_RegionFillUp, ho_ObjectSelected1, &ho_RegionDifference);
                        ShapeTrans(ho_ObjectSelected1, &ho_RegionTrans, "rectangle1");
                        Union1(ho_RegionTrans, &ho_RegionTrans);

                        ConcatObj(ho_TinOBJ, ho_RegionTrans, &ho_TinOBJ);

                        ErosionRectangle1(ho_RegionTrans, &ho_RegionErosion, 5, 5);
                        ReduceDomain(ho_MaskImageScaled, ho_RegionErosion, &ho_ImageReduced);
                        Threshold(ho_ImageReduced, &ho_Region1, 0, 50);//??open
                        Connection(ho_Region1, &ho_ConnectedRegions1);
                        ShapeTrans(ho_ObjectSelected1, &ho_RegionTransConvex, "convex");
                        Difference(ho_RegionTransConvex, ho_ObjectSelected1, &ho_RegionDifferenceConvex
                        );
                        Intersection(ho_RegionDifferenceConvex, ho_Region1, &ho_RegionAIVoidTemp
                        );

                        ReduceDomain(ho_ImageEmphasize, ho_RegionAIVoidTemp, &ho_ImageAIVoidTemp
                        );
                        BinaryThreshold(ho_ImageAIVoidTemp, &ho_RegionVoid, "max_separability",
                            "light", &hv_UsedThreshold);
                        Threshold(ho_ImageAIVoidTemp, &ho_RegionVoid, hv_UsedThreshold * 0.85, 255);//???open

                        FillUp(ho_RegionVoid, &ho_RegionVoid);
                        Connection(ho_RegionVoid, &ho_ConnectedRegions1);
                        //connection (RegionDifference, ConnectedRegions1)

                        RegionFeatures(ho_ConnectedRegions1, "inner_radius", &hv_inner_radius_Value);
                        RegionFeatures(ho_ConnectedRegions1, "outer_radius", &hv_outer_radius_Value);
                        hv_Indices1 = -1;
                        if (0 != (hv_inner_radius_Value != 0))
                        {
                            hv_I_O_radius_Value = hv_outer_radius_Value / hv_inner_radius_Value;
                            TupleLessElem(hv_I_O_radius_Value, 2.0, &hv_Less);
                            TupleFind(hv_Less, 1, &hv_Indices1);
                        }
                        GenEmptyObj(&ho_PadBubbleRegion);
                        if (0 != (hv_Indices1 != -1))
                        {
                            SelectObj(ho_ConnectedRegions1, &ho_PadBubbleRegion, hv_Indices1 + 1);
                            SelectShape(ho_PadBubbleRegion, &ho_PadBubbleRegion, "area", "and", 30,
                                99999);

                            SelectGray(ho_PadBubbleRegion, ho_ImageScaled, &ho_PadBubbleRegion, "mean",
                                "and", 60, 255);
                            Union1(ho_PadBubbleRegion, &ho_PadBubbleRegion);
                        }
                        if (0 != (hv_Number > 1))
                        {
                            DilationRectangle1(ho_SelectedROI, &ho_RegionDilation1, 50, 50);
                            Intersection(ho_RegionDilation1, ho_ObjectSelected1, &ho_RegionIntersection
                            );
                            AreaCenter(ho_RegionIntersection, &hv_ObjArea, &hv_Row4, &hv_Column4);
                            (*hv_Out_TinArea)[hv_ROIIndex] = (((hv_ObjArea * 1.) / hv_Scale) * 1.) / hv_Scale;
                            (*hv_Out_TinAreaRatio)[hv_ROIIndex] = ((((hv_ObjArea * 1.) / hv_Scale) * 1.) / hv_Scale) / HTuple(hv_AreaModel[hv_ROIIndex]);
                            //concat_obj (TinOBJ, RegionIntersection, TinOBJ)
                        }
                        else
                        {
                            Union1(ho_ObjectSelected1, &ho_ObjectSelected1);
                            DilationRectangle1(ho_ObjectSelected1, &ho_RegionDilation, 10, 10);
                            ShapeTrans(ho_RegionDilation, &ho_RegionTrans1, "rectangle1");
                            ReduceDomain(ho_MaskImageScaled, ho_RegionTrans1, &ho_ImageReducedTinAI
                            );
                            Threshold(ho_ImageReducedTinAI, &ho_RegionTinAI, HTuple(hv_Buble_thresh[hv_ROIIndex]),
                                255);
                            AreaCenter(ho_RegionTinAI, &hv_ObjArea, &hv_Row3, &hv_Column3);
                            (*hv_Out_TinArea)[hv_ROIIndex] = (((hv_ObjArea * 1.) / hv_Scale) * 1.) / hv_Scale;
                            (*hv_Out_TinAreaRatio)[hv_ROIIndex] = ((((hv_ObjArea * 1.) / hv_Scale) * 1.) / hv_Scale) / HTuple(hv_AreaModel[hv_ROIIndex]);
                            //concat_obj (TinOBJ, RegionTinAI, TinOBJ)
                        }
                        CountObj(ho_PadBubbleRegion, &hv_NumberBubbleRegion);
                        if (0 != (hv_NumberBubbleRegion != 0))
                        {
                            AreaCenter(ho_PadBubbleRegion, &hv_BubbleArea, &hv_Row2, &hv_Column2);
                            (*hv_Out_BubbleArea)[hv_ROIIndex] = (((hv_BubbleArea * 1.) / hv_Scale) * 1.) / hv_Scale;
                            (*hv_Out_BubbleRatio)[hv_ROIIndex] = (hv_BubbleArea * 1.) / hv_ObjArea;
                        }
                        else
                        {
                            GenEmptyRegion(&ho_PadBubbleRegion);
                        }

                        ConcatObj(ho_VoidOBJ, ho_PadBubbleRegion, &ho_VoidOBJ);

                    }
                    else
                    {

                        ConcatObj(ho_TinOBJ, ho_SelectedROI, &ho_TinOBJ);
                    }
                    //select_shape_proto (BoundingObj, Circle, ObjectSelected1, 'overlaps_abs', 1, 5000)
                    SelectShapeProto(ho_BoundingObj, ho_SelectedROI, &ho_ObjectSelected1, "overlaps_rel",
                        20, 100);
                    CountObj(ho_ObjectSelected1, &hv_Number5);
                    if (0 != (hv_Number5 == 0))
                    {
                        (*hv_ResultArray)[hv_ROIIndex] = 5;//少焊
                    }
                    else
                    {
                        Union1(ho_ObjectSelected1, &ho_ObjectSelected1);
                        Intensity(ho_ObjectSelected1, ho_ImageScaled, &hv_Mean, &hv_Deviation);
                        (*hv_PadMeanArray)[hv_ROIIndex] = hv_Mean;
                        ReduceDomain(ho_ImageScaled, ho_ObjectSelected1, &ho_ImageReduced2);
                        MeanImage(ho_ImageReduced2, &ho_ImageMean, 3, 3);
                        Intensity(ho_ObjectSelected1, ho_ImageMean, &hv_Mean1, &hv_Deviation1);
                        (*hv_GRRGrayArray)[hv_ROIIndex] = hv_Mean1;

                        if (hv_Mean > pAlgoParam->iPadGrayMeanMax)//虚焊阈值
                        {
                            (*hv_ResultArray)[hv_ROIIndex] = 6;
                        }

                        ReduceDomain(ho_ImageScaled, ho_SelectedROI, &ho_ImageReducedROI);
                        MinMaxGray(ho_ImageReducedROI, ho_ImageReducedROI, 0, &hv_MinROI, &hv_MaxROI,
                            &hv_RangeROI);

                        Threshold(ho_ImageReducedROI, &ho_RegionZero, 0, pAlgoParam->iDarkestGrayValue);
                        ErosionRectangle1(ho_ImageReducedROI, &ho_RegionErosionROI, 15, 15);
                        Intersection(ho_RegionErosionROI, ho_RegionZero, &ho_RegionZeroerosion);
                        RegionFeatures(ho_RegionZeroerosion, "area", &hv_RegionZeroArea);

                        if (0 != (HTuple(hv_MinROI < pAlgoParam->iDarkestGrayValue).TupleAnd(hv_RegionZeroArea > pAlgoParam->iZeroRegionArea)))//open
                        {
                            (*hv_ResultArray)[hv_ROIIndex] = 3;//人造短路
                        }
                    }
                }
            }
        }

        //*输出区域转换
        hv_ReScale = 1. / hv_Scale;
        HomMat2dScale(hv_HomMat2DIdentity, hv_ReScale, hv_ReScale, 0, 0, &hv_ReHomMat2DScale);

        AffineTransRegion(ho_TinOBJ, &ho_TinOBJ, hv_ReHomMat2DScale, "nearest_neighbor");
        AffineTransRegion(ho_VoidOBJ, &ho_VoidOBJ, hv_ReHomMat2DScale, "nearest_neighbor");
        AffineTransRegion(ho_TinOBJ, &ho_TinOBJ, hv_HomMat2DBack, "nearest_neighbor");
        AffineTransRegion(ho_VoidOBJ, &ho_VoidOBJ, hv_HomMat2DBack, "nearest_neighbor");

        AffineTransRegion(ho_xizhaRegion, &ho_xizhaRegion, hv_ReHomMat2DScale, "nearest_neighbor");
        AffineTransRegion(ho_xizhaRegion, &ho_xizhaRegion, hv_HomMat2DBack, "nearest_neighbor");

        //*****输出坐标***
        AffineTransPixel(hv_HomMat2DBack, hv_ROIRowArrayInput, hv_ROIColArrayInput, &hv_RowTrans,
            &hv_ColTrans);
        (*hv_ResRowArray) = hv_RowTrans;
        (*hv_ResColArray) = hv_ColTrans;

        //*绘制结果
        DrawResult(ho_SrcImage, ho_VoidOBJ, ho_TinOBJ, ho_xizhaRegion, &(*ho_ResImage),
            hv_ROINumber, (*hv_Out_BubbleRatio), (*hv_Out_TinAreaRatio), (*hv_ResultArray),
            (*hv_BubbleAreaCircle), hv_RingBreak_Switch, &hv_okngFlag, &hv_ResultArrayReJudge);

        (*hv_ResultArray) = hv_ResultArrayReJudge;


    }
    // catch (Exception) 
    catch (HException& HDevExpDefaultException)
    {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
        return;
    }
    return;

}


void CSheetMICRegionProcess::pts_circle(HObject* ho_Circle, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ArcType,
    HTuple hv_ActiveNum, HTuple* hv_RowCenter, HTuple* hv_ColCenter, HTuple* hv_Radius,
    HTuple* hv_StartPhi, HTuple* hv_EndPhi, HTuple* hv_PointOrder, HTuple* hv_ArcAngle)
{

    // Local iconic variables
    HObject  ho_Contour, ho_Circle1, ho_Circle2;

    // Local control variables
    HTuple  hv_Length, hv_Length1, hv_DistanceMin1;
    HTuple  hv_DistanceMax1, hv_DistanceMin2, hv_DistanceMax2;
    HTuple  hv_Sum1, hv_Sum2, hv_Row, hv_Col, hv_CircleLength;
    HTuple  hv_Exception;

    //初始化
    try
    {


        (*hv_RowCenter) = 0;
        (*hv_ColCenter) = 0;
        (*hv_Radius) = 0;
        //产生一个空的直线对象，用于保存拟合后的圆
        GenEmptyObj(&(*ho_Circle));
        //计算边缘数量
        TupleLength(hv_Cols, &hv_Length);
        //当边缘数量不小于有效点数时进行拟合
        if (0 != (HTuple(hv_Length >= hv_ActiveNum).TupleAnd(hv_ActiveNum > 2)))
        {
            //halcon的拟合是基于xld的，需要把边缘连接成xld
            if (0 != (hv_ArcType == HTuple("circle")))
            {
                //如果是闭合的圆，轮廓需要首尾相连
                GenContourPolygonXld(&ho_Contour, hv_Rows.TupleConcat(HTuple(hv_Rows[0])),
                    hv_Cols.TupleConcat(HTuple(hv_Cols[0])));
            }
            else
            {
                GenContourPolygonXld(&ho_Contour, hv_Rows, hv_Cols);
            }
            //拟合圆。使用的算法是''geotukey''，其他算法请参考fit_circle_contour_xld的描述部分。
            FitCircleContourXld(ho_Contour, "geotukey", -1, 0, 0, 3, 2, &(*hv_RowCenter),
                &(*hv_ColCenter), &(*hv_Radius), &(*hv_StartPhi), &(*hv_EndPhi), &(*hv_PointOrder));
            //判断拟合结果是否有效：如果拟合成功，数组中元素的数量大于0
            TupleLength((*hv_StartPhi), &hv_Length1);
            if (0 != (hv_Length1 < 1))
            {
                return;
            }
            //根据拟合结果，产生直线xld
            if (0 != (hv_ArcType == HTuple("arc")))
            {
                //判断圆弧的方向：顺时针还是逆时针
                //halcon求圆弧会出现方向混乱的问题
                GenCircleContourXld(&ho_Circle1, (*hv_RowCenter), (*hv_ColCenter), (*hv_Radius),
                    (*hv_StartPhi), (*hv_EndPhi), "positive", 1);
                GenCircleContourXld(&ho_Circle2, (*hv_RowCenter), (*hv_ColCenter), (*hv_Radius),
                    (*hv_StartPhi), (*hv_EndPhi), "negative", 1);

                DistancePc(ho_Circle1, hv_Rows, hv_Cols, &hv_DistanceMin1, &hv_DistanceMax1);
                DistancePc(ho_Circle2, hv_Rows, hv_Cols, &hv_DistanceMin2, &hv_DistanceMax2);
                TupleSum(hv_DistanceMin1, &hv_Sum1);
                TupleSum(hv_DistanceMin2, &hv_Sum2);
                if (0 != (hv_Sum1 < hv_Sum2))
                {
                    (*hv_PointOrder) = "positive";
                }
                else
                {
                    (*hv_PointOrder) = "negative";
                }
                GenCircleContourXld(&(*ho_Circle), (*hv_RowCenter), (*hv_ColCenter), (*hv_Radius),
                    (*hv_StartPhi), (*hv_EndPhi), (*hv_PointOrder), 1);
                GetContourXld((*ho_Circle), &hv_Row, &hv_Col);
                AngleLl((*hv_RowCenter), (*hv_ColCenter), HTuple(hv_Row[0]), HTuple(hv_Col[0]),
                    (*hv_RowCenter), (*hv_ColCenter), HTuple(hv_Row[(hv_Row.TupleLength()) - 1]),
                    HTuple(hv_Col[(hv_Row.TupleLength()) - 1]), &(*hv_ArcAngle));
                if (0 != 0)
                {
                    LengthXld((*ho_Circle), &hv_CircleLength);
                    (*hv_ArcAngle) = (*hv_EndPhi) - (*hv_StartPhi);
                    if (0 != (hv_CircleLength > ((HTuple(180).TupleRad()) * (*hv_Radius))))
                    {
                        if (0 != (((*hv_ArcAngle).TupleAbs()) < (HTuple(180).TupleRad())))
                        {
                            if (0 != ((*hv_ArcAngle) > 0))
                            {
                                (*hv_ArcAngle) = (HTuple(360).TupleRad()) - (*hv_ArcAngle);
                            }
                            else
                            {
                                (*hv_ArcAngle) = (HTuple(360).TupleRad()) + (*hv_ArcAngle);
                            }
                        }
                    }
                    else
                    {
                        if (0 != (hv_CircleLength < ((HTuple(180).TupleRad()) * (*hv_Radius))))
                        {
                            if (0 != (((*hv_ArcAngle).TupleAbs()) > (HTuple(180).TupleRad())))
                            {
                                if (0 != ((*hv_ArcAngle) > 0))
                                {
                                    (*hv_ArcAngle) = (*hv_ArcAngle) - (HTuple(360).TupleRad());
                                }
                                else
                                {
                                    (*hv_ArcAngle) = (HTuple(360).TupleRad()) + (*hv_ArcAngle);
                                }
                            }
                        }

                    }
                }
            }
            else
            {
                (*hv_StartPhi) = 0;
                (*hv_EndPhi) = HTuple(360).TupleRad();
                (*hv_ArcAngle) = HTuple(360).TupleRad();
                GenCircleContourXld(&(*ho_Circle), (*hv_RowCenter), (*hv_ColCenter), (*hv_Radius),
                    (*hv_StartPhi), (*hv_EndPhi), (*hv_PointOrder), 1);
            }
        }

    }
    // catch (Exception) 
    catch (HException& HDevExpDefaultException)
    {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
    }
    return;
}


void CSheetMICRegionProcess::RingBreakCheckUnet(HObject ho_ImageScaled, HObject ho_ImageEmphasize, HObject ho_ROICir,
    HObject ho_MaskImageScaled, HObject* ho_CirOuterReal, HObject* ho_BubbleRegion,
    HObject* ho_xizhaRegion, HTuple hv_IsRotateImage, HTuple hv_Buble_tresh, HTuple hv_ROINameArrayInput,
    HTuple* hv_BreakError, HTuple* hv_BubbleAreaRation, HTuple* hv_CrackLength)
{

    // Local iconic variables
    HObject  ho_EmptyObject, ho_EmptyRegion, ho_RadiusLine;
    HObject  ho_RegionFitRing, ho_RegionCheck, ho_Circle, ho_RegionFillUp5;
    HObject  ho_RegionBorder, ho_InerCirEdgePoints, ho_OutCirEdgePoints;
    HObject  ho_Rectangle, ho_DiffRect, ho_ConnectedRegions;
    HObject  ho_ObjectSelected, ho_RegionIntersection, ho_ConnectedPoints;
    HObject  ho_InterEdgePoint, ho_OutEdgePoint, ho_CirInter;
    HObject  ho_CirOuter, ho_RegionCirInter, ho_RegionCirOuter;
    HObject  ho_RegionIntersection2, ho_ConnectedRegions6, ho_SelectedRegions3;
    HObject  ho_Circle1, ho_InCirRegion, ho_OutCirRegion, ho_RegionROIRing;
    HObject  ho_ROIRingBorder, ho_RingBorderDilation, ho_ConnectedRingBorderDila;
    HObject  ho_RegionRing, ho_RegionFillUp, ho_RegionDifference;
    HObject  ho_ConnectedDiff, ho_ObjectsSelect, ho_ObjRingBorder;
    HObject  ho_RingImage, ho_RegionUnion1, ho_RegionDifference1;
    HObject  ho_ImageReducedVoidTemp, ho_RegionDark, ho_ConnectedRegions5;
    HObject  ho_RegionFillUp1, ho_RegionClosing, ho_RingRealArea;
    HObject  ho_RingROI, ho_Region1, ho_RegionOpening, ho_ConnectedRegions1;
    HObject  ho_RegionFillUp2, ho_SelectedRegions1, ho_ObjectSelected1;
    HObject  ho_ConnectedRegions3, ho_RegionErosion, ho_ObjectSelected2;
    HObject  ho_PolarTransRegion, ho_ConnectedRegions8, ho_Rectangle1;
    HObject  ho_RegionLines4, ho_ArrowRegion, ho_xizhaCheckRegion;
    HObject  ho_ImageReduced, ho_Region;

    // Local control variables
    HTuple  hv_Area, hv_CenterRow, hv_CenterCol, hv_i;
    HTuple  hv_j, hv_IsEqual, hv_Number2, hv_PointsArea, hv_PointRow;
    HTuple  hv_PointColumn, hv_CenterRowtuple, hv_CenterColtuple;
    HTuple  hv_Distance, hv_Indices, hv_Area1, hv_Row3, hv_Column3;
    HTuple  hv_Area2, hv_Row4, hv_Column4, hv_RowCenter, hv_ColCenter;
    HTuple  hv_Radius1, hv_StartPhi, hv_EndPhi, hv_PointOrder;
    HTuple  hv_ArcAngle, hv_RowCenter1, hv_ColCenter1, hv_Radius2;
    HTuple  hv_StartPhi1, hv_EndPhi1, hv_PointOrder1, hv_ArcAngle1;
    HTuple  hv_InnerRadiusValue, hv_OuterRadiusValue, hv_Number;
    HTuple  hv_Area3, hv_PointOrder2, hv_CirInterNum, hv_Num;
    HTuple  hv_ConnectedRingBorderDilaNumber, hv_inner_radius_Value;
    HTuple  hv_outer_radius_Value, hv_I_O_radius_Value, hv_Indices1;
    HTuple  hv_Less, hv_BubbleRegionNum, hv_Area6, hv_Row6;
    HTuple  hv_Column6, hv_Row1, hv_Column1, hv_Area4, hv_Row;
    HTuple  hv_Column, hv_Width, hv_Height, hv_Row13, hv_Column13;
    HTuple  hv_Row23, hv_Column23, hv_Number3, hv_Row14, hv_Column14;
    HTuple  hv_Row24, hv_Column24, hv_diffRow, hv_maxval, hv_Index;
    HTuple  hv_RingArea, hv_Row2, hv_Column2, hv_Exception;


    const MICRegionParam* pAlgoParam = (MICRegionParam*)m_vecMICRegion[0].pRegionAlgoParam;
    try
    {
        GenEmptyObj(&ho_EmptyObject);
        GenEmptyRegion(&ho_EmptyRegion);
        GenEmptyObj(&(*ho_BubbleRegion));
        GenEmptyObj(&(*ho_xizhaRegion));
        GenEmptyObj(&ho_RadiusLine);

        (*hv_BreakError) = 0;
        (*hv_BubbleAreaRation) = 0;
        (*hv_CrackLength) = 0;
        Threshold(ho_MaskImageScaled, &ho_RegionFitRing, 128, 255);
        Threshold(ho_MaskImageScaled, &ho_RegionCheck, HTuple(hv_ROINameArrayInput[0]),
            255);
        //*拟合内圆
        AreaCenter(ho_ROICir, &hv_Area, &hv_CenterRow, &hv_CenterCol);
        GenCircle(&ho_Circle, hv_CenterRow, hv_CenterCol, 5);

        FillUpShape(ho_RegionFitRing, &ho_RegionFillUp5, "area", 1, 5000);
        Boundary(ho_RegionFillUp5, &ho_RegionBorder, "outer");

        GenEmptyObj(&ho_InerCirEdgePoints);
        GenEmptyObj(&ho_OutCirEdgePoints);
        for (hv_i = 0; hv_i <= 3; hv_i += 1)
        {
            GenRectangle2(&ho_Rectangle, hv_CenterRow, hv_CenterCol, (hv_i * 45).TupleRad(),
                250, 1);
            Difference(ho_Rectangle, ho_Circle, &ho_DiffRect);
            Connection(ho_DiffRect, &ho_ConnectedRegions);
            for (hv_j = 1; hv_j <= 2; hv_j += 1)
            {
                SelectObj(ho_ConnectedRegions, &ho_ObjectSelected, hv_j);
                Intersection(ho_RegionBorder, ho_ObjectSelected, &ho_RegionIntersection);
                Connection(ho_RegionIntersection, &ho_ConnectedPoints);
                TestEqualRegion(ho_ConnectedPoints, ho_EmptyRegion, &hv_IsEqual);
                CountObj(ho_ConnectedPoints, &hv_Number2);
                if (0 != (HTuple(hv_IsEqual == 0).TupleAnd(hv_Number2 > 1)))
                {
                    AreaCenter(ho_ConnectedPoints, &hv_PointsArea, &hv_PointRow, &hv_PointColumn);
                    TupleGenConst(hv_PointRow.TupleLength(), hv_CenterRow, &hv_CenterRowtuple);
                    TupleGenConst(hv_PointRow.TupleLength(), hv_CenterCol, &hv_CenterColtuple);
                    //distance_pp (PointRow[0], PointColumn[0], PointRow[1], PointColumn[1], Distance1)
                    //if (Distance1<45 or Distance1>95)
                      //continue
                    //endif
                    DistancePp(hv_CenterRowtuple, hv_CenterColtuple, hv_PointRow, hv_PointColumn,
                        &hv_Distance);
                    TupleSortIndex(hv_Distance, &hv_Indices);
                    SelectObj(ho_ConnectedPoints, &ho_InterEdgePoint, HTuple(hv_Indices[0]) + 1);
                    ConcatObj(ho_InerCirEdgePoints, ho_InterEdgePoint, &ho_InerCirEdgePoints
                    );
                    SelectObj(ho_ConnectedPoints, &ho_OutEdgePoint, HTuple(hv_Indices[1]) + 1);
                    ConcatObj(ho_OutCirEdgePoints, ho_OutEdgePoint, &ho_OutCirEdgePoints);
                }
            }
        }

        AreaCenter(ho_InerCirEdgePoints, &hv_Area1, &hv_Row3, &hv_Column3);
        AreaCenter(ho_OutCirEdgePoints, &hv_Area2, &hv_Row4, &hv_Column4);
        pts_circle(&ho_CirInter, hv_Row3, hv_Column3, "circle", 3, &hv_RowCenter, &hv_ColCenter,
            &hv_Radius1, &hv_StartPhi, &hv_EndPhi, &hv_PointOrder, &hv_ArcAngle);
        pts_circle(&ho_CirOuter, hv_Row4, hv_Column4, "circle", 3, &hv_RowCenter1, &hv_ColCenter1,
            &hv_Radius2, &hv_StartPhi1, &hv_EndPhi1, &hv_PointOrder1, &hv_ArcAngle1);
        //*     pts_to_best_circle (CirInter, Row3, Column3, 'circle', 3, RowCenter, ColCenter, Radius1, StartPhi, EndPhi, PointOrder, ArcAngle)
        //*     pts_to_best_circle (CirOuter, Row4, Column4, 'circle', 3, RowCenter1, ColCenter1, Radius2, StartPhi1, EndPhi1, PointOrder1, ArcAngle1)

        GenRegionContourXld(ho_CirInter, &ho_RegionCirInter, "filled");
        GenRegionContourXld(ho_CirOuter, &ho_RegionCirOuter, "filled");

        RegionFeatures(ho_RegionCirInter, "inner_radius", &hv_InnerRadiusValue);
        RegionFeatures(ho_RegionCirOuter, "inner_radius", &hv_OuterRadiusValue);


        CountObj(ho_CirOuter, &hv_Number);
        if (0 != (HTuple(hv_Number == 0).TupleAnd((hv_Radius1.TupleLength()) > 0)))
        {
            GenCircleContourXld(&ho_CirOuter, hv_RowCenter, hv_ColCenter, hv_Radius1 * 1.45,
                0, 6.28318, "positive", 1);
            AreaCenterXld(ho_CirOuter, &hv_Area3, &hv_RowCenter1, &hv_ColCenter1, &hv_PointOrder2);
            hv_Radius2 = hv_Radius1 * 1.45;
        }
        /*if (HDevWindowStack::IsOpen())
            DispObj(ho_ImageScaled, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
            DispObj(ho_CirInter, HDevWindowStack::GetActive());
        if (HDevWindowStack::IsOpen())
            DispObj(ho_CirOuter, HDevWindowStack::GetActive());*/
        CountObj(ho_CirInter, &hv_CirInterNum);
        if (0 != (HTuple(hv_CirInterNum == 0).TupleAnd((hv_Radius2.TupleLength()) > 0)))
        {
            GenCircleContourXld(&ho_CirInter, hv_RowCenter1, hv_ColCenter1, hv_Radius2 / 1.45,
                0, 6.28318, "positive", 1);
            AreaCenterXld(ho_CirOuter, &hv_Area3, &hv_RowCenter, &hv_ColCenter, &hv_PointOrder);
            hv_Radius1 = hv_Radius2 * 1.45;
            //BreakError := 2
        }
        else
        {
            //distance_pp (RowCenter, ColCenter, CirROICenRow, CirROICenCol, CenterDistance)
            //if (CenterDistance>25)
              //BreakError := 2
              //return ()
            //endif
        }
        if (0 != (HTuple(HTuple(hv_Number == 0).TupleAnd(hv_CirInterNum == 0)).TupleOr(HTuple(hv_InnerRadiusValue < 70).TupleAnd(hv_OuterRadiusValue < 180))))
        {
            Intersection(ho_ROICir, ho_RegionFitRing, &ho_RegionIntersection2);
            Connection(ho_RegionIntersection2, &ho_ConnectedRegions6);
            SelectShapeStd(ho_ConnectedRegions6, &ho_SelectedRegions3, "max_area", 70);
            SmallestCircle(ho_SelectedRegions3, &hv_RowCenter1, &hv_ColCenter1, &hv_Radius2);
            GenCircle(&ho_Circle1, hv_RowCenter1, hv_ColCenter1, hv_Radius2);
            GenContourRegionXld(ho_Circle1, &ho_CirOuter, "border");
            GenCircleContourXld(&ho_CirInter, hv_RowCenter1, hv_ColCenter1, hv_Radius2 / 1.45,
                0, 6.28318, "positive", 1);
            AreaCenterXld(ho_CirOuter, &hv_Area3, &hv_RowCenter, &hv_ColCenter, &hv_PointOrder);
            //Radius1 := Radius2*1.45

            hv_Radius1 = hv_Radius2 / 1.45;
        }
        //*检测
        GenRegionContourXld(ho_CirInter, &ho_InCirRegion, "filled");
        GenRegionContourXld(ho_CirOuter, &ho_OutCirRegion, "filled");

        DilationCircle(ho_InCirRegion, &ho_InCirRegion, 3);

        Difference(ho_OutCirRegion, ho_InCirRegion, &ho_RegionROIRing);
        Boundary(ho_RegionROIRing, &ho_ROIRingBorder, "inner");
        DilationCircle(ho_ROIRingBorder, &ho_RingBorderDilation, 2.5);
        Connection(ho_RingBorderDilation, &ho_ConnectedRingBorderDila);

        Intersection(ho_OutCirRegion, ho_RegionCheck, &ho_RegionRing);
        FillUpShape(ho_RegionRing, &ho_RegionFillUp, "area", 1, 5000);

        OpeningCircle(ho_RegionFillUp, &(*ho_CirOuterReal), 2);

        Difference(ho_RegionROIRing, (*ho_CirOuterReal), &ho_RegionDifference);
        Connection(ho_RegionDifference, &ho_ConnectedDiff);
        CountObj(ho_ConnectedDiff, &hv_Num);
        CopyObj(ho_ConnectedDiff, &ho_ObjectsSelect, 1, hv_Num);

        CountObj(ho_ConnectedRingBorderDila, &hv_ConnectedRingBorderDilaNumber);
        {
            HTuple end_val120 = hv_ConnectedRingBorderDilaNumber;
            HTuple step_val120 = 1;
            for (hv_i = 1; hv_i.Continue(end_val120, step_val120); hv_i += step_val120)
            {
                SelectObj(ho_ConnectedRingBorderDila, &ho_ObjRingBorder, hv_i);
                SelectShapeProto(ho_ObjectsSelect, ho_ObjRingBorder, &ho_ObjectsSelect, "overlaps_abs",
                    10, 9999999);
            }
        }

        TestEqualObj(ho_ObjectsSelect, ho_EmptyObject, &hv_IsEqual);
        //***********极坐标变换**********************
        //illuminate (ImageEmphasize, ImageIlluminate, 101, 101, 0.7)
        //reduce_domain (ImageIlluminate, RegionROIRing, RingImage)
        //polar_trans_image_ext (RingImage, PolarTransImage, RowCenter, ColCenter, 0, 6.28319, Radius1+5, Radius2-5, 512, 512, 'bilinear')
        //intensity (PolarTransImage, PolarTransImage, Mean, Deviation)
        //fast_threshold (PolarTransImage, Region2, 0, Mean+Deviation, 20)
        //fill_up (Region2, RegionFillUp3)
        //opening_rectangle1 (RegionFillUp3, Region2, 1, 10)
        //connection (Region2, ConnectedRegions2)
        //select_shape (ConnectedRegions2, SelectedRegions2, ['area','rect2_len1'], 'and', [5800, 245], [999999,256])
        //count_obj (SelectedRegions2, Number1)
        //if (Number1>1)
          //BreakError := 1
          //difference (PolarTransImage, SelectedRegions2, RegionDifference3)
          //opening_rectangle1 (RegionDifference3, RegionOpening1, 1, 15)
          //connection (RegionOpening1, ConnectedRegions4)
          //select_shape (ConnectedRegions4, SelectedRegions, 'rect2_len1', 'and', 245, 256)
          //polar_trans_region_inv (SelectedRegions, XYTransRegion, RowCenter, ColCenter, 0, 6.28319, Radius1+5, Radius2-5, 512, 512, 500, 932, 'nearest_neighbor')
          //difference (CirOuterReal, XYTransRegion, CirOuterReal)
        //endif
        //**************
        if (0 != (hv_IsEqual == 0))
        {
            //*存在断痕
            (*hv_BreakError) = 1;
        }
        //**************气泡检测***************
        if (0 != 1)
        {
            Union2(ho_RegionROIRing, ho_RegionFitRing, &ho_RegionUnion1);
            Difference(ho_RegionUnion1, ho_RegionFitRing, &ho_RegionDifference1);

            ReduceDomain(ho_ImageEmphasize, ho_RegionDifference1, &ho_ImageReducedVoidTemp
            );
            Threshold(ho_ImageReducedVoidTemp, &ho_RegionDark, 0, 30);
            Difference(ho_RegionDifference1, ho_RegionDark, &ho_RegionDifference1);

            Connection(ho_RegionDifference1, &ho_ConnectedRegions5);
            FillUp(ho_RegionFillUp, &ho_RegionFillUp1);
            ClosingCircle(ho_RegionFillUp1, &ho_RegionClosing, 310);
            Difference(ho_RegionClosing, ho_InCirRegion, &ho_RingRealArea);
            ReduceDomain(ho_ImageEmphasize, ho_RingRealArea, &ho_RingROI);

            //mean_image (RingROI, ImageMean1, 5, 5)
            //mean_image (RingROI, ImageMean2, 29, 29)
            //dyn_threshold (ImageMean1, ImageMean2, RegionDynThresh, 10, 'light')

            //intersection (RegionDynThresh, RegionDifference1, RegionIntersection1)

            //enhanceGuidHDR (RingROI, ImageResult, 5000, 1, 5, 0.01)


            FastThreshold(ho_RingROI, &ho_Region1, hv_Buble_tresh, 255, 10);
            OpeningCircle(ho_Region1, &ho_RegionOpening, 1.5);
            Connection(ho_RegionOpening, &ho_ConnectedRegions1);
            FillUp(ho_ConnectedRegions1, &ho_RegionFillUp2);
            ConcatObj(ho_RegionFillUp2, ho_ConnectedRegions5, &ho_RegionFillUp2);
            SelectShape(ho_RegionFillUp2, &ho_SelectedRegions1, ((HTuple("area").Append("inner_radius")).Append("outer_radius")),
                "and", ((HTuple(15).Append(1)).Append(1)), ((HTuple(99999).Append(55)).Append(55)));
            RegionFeatures(ho_SelectedRegions1, "inner_radius", &hv_inner_radius_Value);
            RegionFeatures(ho_SelectedRegions1, "outer_radius", &hv_outer_radius_Value);

            if (0 != (hv_inner_radius_Value != 0))
            {
                hv_I_O_radius_Value = hv_outer_radius_Value / hv_inner_radius_Value;
                hv_Indices1 = -1;
                TupleLessElem(hv_I_O_radius_Value, 4, &hv_Less);
                TupleFind(hv_Less, 1, &hv_Indices1);
                if (0 != (HTuple(hv_Indices1 != -1).TupleOr(hv_IsEqual == 0)))
                {
                    if (0 != (hv_Indices1 != -1))
                    {
                        SelectObj(ho_SelectedRegions1, &ho_ObjectSelected1, hv_Indices1 + 1);
                        Union1(ho_ObjectSelected1, &(*ho_BubbleRegion));
                    }
                    if (0 != (hv_IsEqual == 0))
                    {
                        OpeningCircle(ho_ObjectsSelect, &ho_ObjectsSelect, 5);
                        Connection(ho_ObjectsSelect, &ho_ConnectedRegions3);
                        SelectShapeStd(ho_ConnectedRegions3, &ho_ObjectsSelect, "max_area", 70);
                        ErosionCircle(ho_ObjectsSelect, &ho_RegionErosion, 3.5);
                        ConcatObj((*ho_BubbleRegion), ho_RegionErosion, &(*ho_BubbleRegion));
                        Union1((*ho_BubbleRegion), &(*ho_BubbleRegion));
                    }
                    CountObj((*ho_BubbleRegion), &hv_BubbleRegionNum);
                    AreaCenter((*ho_BubbleRegion), &hv_Area6, &hv_Row6, &hv_Column6);
                    if (0 != (hv_BubbleRegionNum.TupleAnd(hv_Area6 != 0)))
                    {
                        AreaCenter((*ho_BubbleRegion), &(*hv_BubbleAreaRation), &hv_Row1, &hv_Column1);
                        Connection((*ho_BubbleRegion), &ho_ConnectedRegions6);
                        //region_features (ConnectedRegions6, 'area', Value)
                        //tuple_max (Value, Max)
                        //tuple_find (Value, Max, Indices2)
                        //select_obj (ConnectedRegions6, ObjectSelected2, Indices2[0]+1)
                        SelectShapeStd(ho_ConnectedRegions6, &ho_ObjectSelected2, "max_area",
                            70);
                        //***********h1+h2算法
                        //distance_rr_min (InCirRegion, ObjectSelected2, MinDistance, Row11, Column11, Row21, Column21)
                        //gen_region_line (RegionLines, Row11, Column11, Row21, Column21)
                        //distance_rr_min (OutCirRegion, ObjectSelected2, MinDistance1, Row12, Column12, Row22, Column22)
                        //gen_region_line (RegionLines1, Row12, Column12, Row22, Column22)
                        //concat_obj (BubbleRegion, RegionLines, BubbleRegion)
                        //concat_obj (BubbleRegion, RegionLines1, BubbleRegion)
                        //***********************************
                        AreaCenter(ho_ObjectSelected2, &hv_Area4, &hv_Row, &hv_Column);
                        if (0 != (HTuple(hv_Row != 0).TupleAnd(hv_RowCenter != 0)))
                        {
                            //*************************
                            ReduceDomain(ho_ImageScaled, ho_RegionROIRing, &ho_RingImage);
                            GetImageSize(ho_RingImage, &hv_Width, &hv_Height);
                            //polar_trans_image_ext (RingImage, PolarTransImage, RowCenter, ColCenter, 0, 6.28319, Radius1+5, Radius2-5, 2*Radius2-10, Radius2-Radius1-10, 'bilinear')
                            PolarTransRegion(ho_ObjectSelected2, &ho_PolarTransRegion, hv_RowCenter,
                                hv_ColCenter, 0, 6.28319, hv_Radius1 + 5, hv_Radius2 - 5, (2 * hv_Radius2) - 10,
                                (hv_Radius2 - hv_Radius1) - 10, "bilinear");
                            SmallestRectangle1(ho_PolarTransRegion, &hv_Row13, &hv_Column13, &hv_Row23,
                                &hv_Column23);
                            Connection(ho_PolarTransRegion, &ho_ConnectedRegions8);
                            CountObj(ho_ConnectedRegions8, &hv_Number3);
                            if (0 != (hv_Number3 > 1))
                            {
                                SmallestRectangle1(ho_ConnectedRegions8, &hv_Row14, &hv_Column14,
                                    &hv_Row24, &hv_Column24);
                                hv_diffRow = hv_Row24 - hv_Row14;
                                hv_maxval = hv_diffRow.TupleMax();
                                TupleFindFirst(hv_diffRow, hv_maxval, &hv_Index);
                                hv_Row13 = HTuple(hv_Row14[hv_Index]);
                                hv_Column13 = HTuple(hv_Column14[hv_Index]);
                                hv_Row23 = HTuple(hv_Row24[hv_Index]);
                                hv_Column23 = HTuple(hv_Column24[hv_Index]);
                            }
                            GenRectangle1(&ho_Rectangle1, hv_Row13, hv_Column13, hv_Row23, hv_Column23);
                            if (0 != ((*hv_BreakError) == 1))
                            {
                                (*hv_CrackLength) = 1.0;
                            }
                            else
                            {
                                (*hv_CrackLength) = (hv_Row23 - hv_Row13) / ((hv_Radius2 - hv_Radius1) - 10);
                            }
                            //read_tuple ('./VoidLength', VoidLength)
                            //if (VoidLength[0])
                              //CrackLength := (Row23-Row13)/(Radius2-Radius1-10)
                              //if (CrackLength>(VoidLength[1]*1.0/100))
                                //BreakError := CrackLength
                              //endif
                            //endif
                            GenRegionLine(&ho_RegionLines4, hv_Row13, (hv_Column13 + hv_Column23) / 2,
                                hv_Row23, (hv_Column13 + hv_Column23) / 2);
                            PolarTransRegionInv(ho_RegionLines4, &ho_ArrowRegion, hv_RowCenter,
                                hv_ColCenter, 0, 6.28319, hv_Radius1 + 5, hv_Radius2 - 5, (2 * hv_Radius2) - 10,
                                (hv_Radius2 - hv_Radius1) - 10, hv_Width, hv_Height, "nearest_neighbor");
                            ConcatObj((*ho_BubbleRegion), ho_ArrowRegion, &(*ho_BubbleRegion));
                            //****************************
                            //gen_region_line (RegionLines2, RowCenter, ColCenter, Row, Column)
                            //line_orientation (RowCenter, ColCenter, Row, Column, Phi)
                            //vector_angle_to_rigid (RowCenter, ColCenter, 0, RowCenter, ColCenter, Phi, HomMat2D)
                            //gen_region_line (RegionLines3, RowCenter, ColCenter-400, RowCenter, ColCenter+400)
                            //affine_trans_region (RegionLines3, RegionAffineTrans, HomMat2D, 'nearest_neighbor')
                            //boundary (ObjectSelected2, RegionBorder1, 'outer')
                            //intersection (RegionAffineTrans, RegionBorder1, RegionIntersection2)
                            //connection (RegionIntersection2, ConnectedRegions7)
                            //area_center (ConnectedRegions7, Area5, Row5, Column5)
                            //if (|Row5|>=2)
                              //gen_arrow_contour_xld (Arrow, Row5[0], Column5[0], Row5[1], Column5[1], 3, 3)
                              //gen_region_contour_xld (Arrow, ArrowRegion, 'filled')
                              //concat_obj (BubbleRegion, ArrowRegion, BubbleRegion)
                              //distance_pp (Row5[0], Column5[0], Row5[1], Column5[1], Distance2)
                              //CrackLength := Distance2/(Radius2-Radius1) + 0.03
                            //endif
                        }
                    }
                    AreaCenter(ho_RegionROIRing, &hv_RingArea, &hv_Row2, &hv_Column2);
                    (*hv_BubbleAreaRation) = (*hv_BubbleAreaRation) * 1.0;
                }
            }
        }
        else
        {
            (*hv_BubbleAreaRation) = 0.0;
        }
        //*********锡渣*******
        if (0 != 1)
        {
            ErosionCircle(ho_InCirRegion, &ho_xizhaCheckRegion, 20);
            ReduceDomain(ho_ImageScaled, ho_xizhaCheckRegion, &ho_ImageReduced);

            Threshold(ho_ImageReduced, &ho_Region, 0, pAlgoParam->iTinBeadsGray);//锡渣
            Connection(ho_Region, &ho_ConnectedRegions5);
            SelectShape(ho_ConnectedRegions5, &(*ho_xizhaRegion), (HTuple("area").Append("anisometry")),
                "and", (HTuple(pAlgoParam->iTinBeadsArea).Append(0)), (HTuple(9999999).Append(5)));//锡渣筛选
        }
        //**********圆环偏移******
    }
    // catch (Exception) 
    catch (HException& HDevExpDefaultException)
    {
        HDevExpDefaultException.ToHTuple(&hv_Exception);
        (*hv_BreakError) = 1;
        (*hv_CrackLength) = 1.0;
        return;
    }

    return;
}


void CSheetMICRegionProcess::DrawResult(HObject ho_SrcImage, HObject ho_VoidOBJ, HObject ho_TinOBJ, HObject ho_xizhaRegion,
    HObject* ho_ResImage, HTuple hv_ROINumber, HTuple hv_Out_BubbleRatio, HTuple hv_Out_TinAreaRatio,
    HTuple hv_ResultArray, HTuple hv_BubbleAreaCircle, HTuple hv_RingBreakSwitch, 
    HTuple* hv_okngFlag, HTuple* hv_ResultArrayReJudge)
{

    // Local iconic variables
    HObject  ho_VoidSelected, ho_TinSelected;

    // Local control variables
    HTuple  hv_VoidFlag, hv_CircleIndex, hv_SrcWidth, hv_SrcHeight, hv_WindowID;
    HTuple  hv_Font, hv_FontWithSize, hv_TinOBJNumber, hv_VoidOBJNumber;
    HTuple  hv_ROIIndex, hv_string, hv_stringCircle, hv_xizhaNumber;

    //********pad：   0-OK    1-气泡    3-连锡    5-少焊   6-虚焊
    //********circle：0-OK    1-气泡    7-锡渣    8-断环
    TupleFindFirst(hv_RingBreakSwitch, 1, &hv_CircleIndex);
    //TupleSortIndex(hv_RingBreakSwitch, &hv_Indices);
    //hv_Out_TinAreaRatio = HTuple(hv_Out_TinAreaRatio[hv_Indices]);
    //hv_Out_BubbleRatio = HTuple(hv_Out_BubbleRatio[hv_Indices]);
    //hv_ResultArray = HTuple(hv_ResultArray[hv_Indices]);

    const MICRegionParam* pAlgoParam = (MICRegionParam*)m_vecMICRegion[0].pRegionAlgoParam;

    (*hv_ResultArrayReJudge) = hv_ResultArray;
    (*hv_okngFlag) = 0;
    GetImageSize(ho_SrcImage, &hv_SrcWidth, &hv_SrcHeight);
    OpenWindow(0, 0, hv_SrcWidth, hv_SrcHeight, 0, "buffer", "", &hv_WindowID);
    SetPart(hv_WindowID, 0, 0, hv_SrcWidth - 1, hv_SrcHeight - 1);

    DispImage(ho_SrcImage, hv_WindowID);
    //原图

    //***********pad
    QueryFont(hv_WindowID, &hv_Font);
    hv_FontWithSize = HTuple(hv_Font[0]) + "-30";
    SetFont(hv_WindowID, hv_FontWithSize);
    SetDraw(hv_WindowID, "margin");

    CountObj(ho_TinOBJ, &hv_TinOBJNumber);
    CountObj(ho_VoidOBJ, &hv_VoidOBJNumber);

    hv_VoidFlag = 0;
    {
        HTuple end_val21 = hv_ROINumber - 1;
        HTuple step_val21 = 1;
        for (hv_ROIIndex = 0; hv_ROIIndex.Continue(end_val21, step_val21); hv_ROIIndex += step_val21)
        {
            if (0 != (hv_ROIIndex == hv_CircleIndex)) //***********圆环
            {
                //***********圆环
                //*气泡
                if (0 != (hv_VoidOBJNumber >= hv_ROINumber))
                {
                    SetColor(hv_WindowID, "yellow");
                    SelectObj(ho_VoidOBJ, &ho_VoidSelected, hv_VoidFlag + 1);
                    DispRegion(ho_VoidSelected, hv_WindowID);
                    hv_VoidFlag = hv_VoidFlag + 1;
                }

                if (0 != (HTuple(hv_BubbleAreaCircle > 0).TupleAnd(hv_VoidOBJNumber >= (hv_ROINumber + 1))))
                {
                    SetColor(hv_WindowID, "blue");
                    SelectObj(ho_VoidOBJ, &ho_VoidSelected, hv_VoidFlag + 1);
                    DispRegion(ho_VoidSelected, hv_WindowID);
                    hv_VoidFlag = hv_VoidFlag + 1;
                }

                SetTposition(hv_WindowID, 250 + (50 * hv_ROIIndex), 50);
                hv_stringCircle = (("circle" + hv_ROIIndex) + ":") + (HTuple(hv_Out_BubbleRatio[hv_ROIIndex]).TupleString(".4f"));

                if (0 != (HTuple(HTuple(hv_Out_BubbleRatio[hv_ROIIndex]) > pAlgoParam->fCircleVoidRatio).TupleAnd(HTuple((*hv_ResultArrayReJudge)[hv_ROIIndex]) != 8)))
                {
                    if (0 != (HTuple(hv_ResultArray[hv_ROIIndex]) == 0))
                    {
                        (*hv_ResultArrayReJudge)[hv_ROIIndex] = 1;
                    }
                    else
                    {

                    }
                    hv_stringCircle = (hv_stringCircle + "\t") + "气泡";
                }

                //*锡渣
                CountObj(ho_xizhaRegion, &hv_xizhaNumber);
                if (0 != (hv_xizhaNumber > 0))
                {
                    SetColor(hv_WindowID, "red");
                    SetDraw(hv_WindowID, "fill");
                    (*hv_okngFlag) = 1;
                    DispRegion(ho_xizhaRegion, hv_WindowID);
                    SetDraw(hv_WindowID, "margin");
                    hv_stringCircle = (hv_stringCircle + "\t") + "锡渣";
                }

                if (0 != (HTuple((*hv_ResultArrayReJudge)[hv_ROIIndex]) == 8))
                {
                    hv_stringCircle = (hv_stringCircle + "\t") + "断环";
                }

                if (0 != (HTuple((*hv_ResultArrayReJudge)[hv_ROIIndex]) != 0))
                {
                    SetColor(hv_WindowID, "red");
                    (*hv_okngFlag) = 1;
                }
                else
                {
                    SetColor(hv_WindowID, "green");

                }
                WriteString(hv_WindowID, hv_stringCircle);
                SelectObj(ho_TinOBJ, &ho_TinSelected, hv_ROIIndex + 1);
                DispRegion(ho_TinSelected, hv_WindowID);
                //***********圆环
            }
            else //*气泡
            {
                SetColor(hv_WindowID, "yellow");
                SelectObj(ho_VoidOBJ, &ho_VoidSelected, hv_VoidFlag + 1);
                DispRegion(ho_VoidSelected, hv_WindowID);

                SetTposition(hv_WindowID, 250 + (50 * hv_ROIIndex), 50);
                hv_string = (((("pad" + hv_ROIIndex) + ":") + (HTuple(hv_Out_BubbleRatio[hv_ROIIndex]).TupleString(".4f"))) + "\t") + (HTuple(hv_Out_TinAreaRatio[hv_ROIIndex]).TupleString(".4f"));

                //*气泡
                if (0 != (HTuple(hv_Out_BubbleRatio[hv_ROIIndex]) > pAlgoParam->fPadVoidRatio))//气泡率
                {
                    if (0 != (HTuple(hv_ResultArray[hv_ROIIndex]) == 0))
                    {
                        (*hv_ResultArrayReJudge)[hv_ROIIndex] = 1;
                    }
                    else
                    {

                    }
                    hv_string = (hv_string + "\t") + "气泡";
                }

                if (0 != (HTuple(HTuple(hv_Out_TinAreaRatio[hv_ROIIndex]) > pAlgoParam->fPadSolderRatioBig).TupleOr(HTuple(hv_ResultArray[hv_ROIIndex]) == 3)))//连锡
                {
                    (*hv_ResultArrayReJudge)[hv_ROIIndex] = 3;
                    hv_string = (hv_string + "\t") + "连锡";
                }

                if (hv_Out_TinAreaRatio[hv_ROIIndex] < pAlgoParam->fPadSolderRatioSmall)//少焊
                {
                    (*hv_ResultArrayReJudge)[hv_ROIIndex] = 5;
                    hv_string = (hv_string + "\t") + "少焊";
                }

                if (hv_ResultArray[hv_ROIIndex] == 6)
                {
                    (*hv_ResultArrayReJudge)[hv_ROIIndex] = 6;
                    hv_string = (hv_string + "\t") + "虚焊";
                }

                if (0 != (HTuple((*hv_ResultArrayReJudge)[hv_ROIIndex]) != 0))
                {
                    SetColor(hv_WindowID, "red");
                    (*hv_okngFlag) = 1;
                }
                else
                {
                    SetColor(hv_WindowID, "green");

                }
                WriteString(hv_WindowID, hv_string);

                //*锡
                SelectObj(ho_TinOBJ, &ho_TinSelected, hv_ROIIndex + 1);
                DispRegion(ho_TinSelected, hv_WindowID);
                hv_VoidFlag = hv_VoidFlag + 1;
            }
        }
    }
    //***********pad

    

    //*写字 :OKNG
    SetTposition(hv_WindowID, 50, 50);
    //Specify font name and size
    hv_FontWithSize = HTuple(hv_Font[0]) + "-80";
    SetFont(hv_WindowID, hv_FontWithSize);

    if (0 != ((*hv_okngFlag) == 1))
    {
        SetColor(hv_WindowID, "red");
        WriteString(hv_WindowID, "NG");
    }
    else
    {
        SetColor(hv_WindowID, "green");
        WriteString(hv_WindowID, "OK");
    }

    DumpWindowImage(&(*ho_ResImage), hv_WindowID);
    CloseWindow(hv_WindowID);
    return;
}

void CSheetMICRegionProcess::DeletelistFiles(string dir)
{
    //在目录后面加上"\\*.*"进行第一次搜索
    string newDir = dir + "\\*.*";
    //用于查找的句柄
    intptr_t handle;
    struct _finddata_t fileinfo;
    //第一次查找
    handle = _findfirst(newDir.c_str(), &fileinfo);

    if (handle == -1) {
        cout << "无文件" << endl;
        system("pause");
        return;
    }

    do
    {
        if (fileinfo.attrib & _A_SUBDIR) {//如果为文件夹，加上文件夹路径，再次遍历
            if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
                continue;

            // 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
            newDir = dir + "\\" + fileinfo.name;
            DeletelistFiles(newDir.c_str());//先遍历删除文件夹下的文件，再删除空的文件夹
            cout << newDir.c_str() << endl;
            if (_rmdir(newDir.c_str()) == 0) {//删除空文件夹
                cout << "delete empty dir success" << endl;
            }
            else {
                cout << "delete empty dir error" << endl;
            }
        }
        else {
            string file_path = dir + "\\" + fileinfo.name;
            cout << file_path.c_str() << endl;
            if (remove(file_path.c_str()) == 0) {//删除文件
                cout << "delete file success" << endl;
            }
            else {
                cout << "delete file error" << endl;
            }
        }
    } while (!_findnext(handle, &fileinfo));

    _findclose(handle);
    return;
}
