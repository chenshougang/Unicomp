#pragma once
#include "ImageTranslator.h"
#include "Logger.h"

class CSheetMICRegionProcess : public CBaseProcess
{
public:
	CSheetMICRegionProcess();
	~CSheetMICRegionProcess();
	bool LoadParam(const string& strXmlPath, CDataObject* pDataObject);
	bool Process(CDataObject* pDataObject);
	void Release();

private:
	void InitRegionParam(const vector<RegionStruct>& vecRegionStruct);
	void GetMICRegionParam(const RegionStruct& regionStruct, void* pAlgoParam);
	bool MICRegionProcess(const ImageObject& objSrcImg, const HObject& ho_Mask, vector<ProcessRegion> m_vecBlackWhiteRegion, vector<DefectInfo>& vecDefsResult, CDataObject* pDataObject);

	//ROI区域自动提取
	void GetROI(HObject ho_InputImg, HObject* ho_SrcImages, HObject* ho_DetectRegion,
		HObject* ho_ImagePart, HTuple* hv_HomMat2D, HTuple* hv_HomMat2DBack);

	//检测
	void MicCheckUnet(HObject ho_SrcImage, HObject ho_ImageModel, HObject ho_DetectRegion,
		HObject ho_ImageAffinTransMask, HObject* ho_ResImage, HTuple hv_ROINameArrayInput,
		HTuple hv_ROIRowArrayInput, HTuple hv_ROIColArrayInput, HTuple hv_RingBreak_Switch,
		HTuple hv_Connected_Switch, HTuple hv_HomMat2D, HTuple hv_HomMat2DBack, HTuple hv_IsRotateImage,
		HTuple hv_Buble_thresh, HTuple hv_GroupName, HTuple hv_ToolName, HTuple* hv_ResultArray,
		HTuple* hv_ResRowArray, HTuple* hv_ResColArray, HTuple* hv_BubbleAreaCircle,
		HTuple* hv_Out_TinArea, HTuple* hv_Out_TinAreaRatio, HTuple* hv_Out_BubbleArea,
		HTuple* hv_Out_BubbleRatio, HTuple* hv_PadMeanArray, HTuple* hv_GRRGrayArray,
		HTuple* hv_CrackLength);

	//拟合圆
	void pts_circle(HObject* ho_Circle, HTuple hv_Rows, HTuple hv_Cols, HTuple hv_ArcType,
		HTuple hv_ActiveNum, HTuple* hv_RowCenter, HTuple* hv_ColCenter, HTuple* hv_Radius,
		HTuple* hv_StartPhi, HTuple* hv_EndPhi, HTuple* hv_PointOrder, HTuple* hv_ArcAngle);

	//断环检测
	void RingBreakCheckUnet(HObject ho_ImageScaled, HObject ho_ImageEmphasize, HObject ho_ROICir,
		HObject ho_MaskImageScaled, HObject* ho_CirOuterReal, HObject* ho_BubbleRegion,
		HObject* ho_xizhaRegion, HTuple hv_IsRotateImage, HTuple hv_Buble_tresh, HTuple hv_ROINameArrayInput,
		HTuple* hv_BreakError, HTuple* hv_BubbleAreaRation, HTuple* hv_CrackLength);

	//结果绘制
	void DrawResult(HObject ho_SrcImage, HObject ho_VoidOBJ, HObject ho_TinOBJ, HObject ho_xizhaRegion,
		HObject* ho_ResImage, HTuple hv_ROINumber, HTuple hv_Out_BubbleRatio, HTuple hv_Out_TinAreaRatio,
		HTuple hv_ResultArray, HTuple hv_BubbleAreaCircle, HTuple hv_RingBreakSwitch, HTuple* hv_okngFlag, HTuple* hv_ResultArrayReJudge);

	void DeletelistFiles(string dir);

private:
	string m_strRegionNode;      //区域节点
	string m_strRegionName;      //区域名
	vector<ProcessRegion> m_vecMICRegion;  //检测区域vector

	CImageTranslator* _translator = nullptr;
	//CImageTranslator* _translator;

	//HObject ho_MaskImage;   //Mask图像
	HObject ho_ModelImage;  //ROI图像

	CLogger _log;

	//static cv::dnn::Net net;
};
