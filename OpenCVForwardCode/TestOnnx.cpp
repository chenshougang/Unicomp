
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <vector>
#include <string>
#include <corecrt_io.h>

using namespace std;

#define MAX_BUF_LEN 1024


void GetFiles(vector<string>& files, string path, string filter);
string Format(string format, ...);
vector<string> Split(const string& s, const string& delim);
string Trim(const string& s);


int main(int argc, char* argv[])
{
	cv::dnn::Net net = cv::dnn::readNetFromONNX("best_deeplabv3plus_resnet50_voc_os16.onnx");
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);


	std::vector<std::string> files;
	GetFiles(files, "E:\\20220424_KunShanChunHua\\20220517_Image\\new", "*.tif"); //D:/Dataset/ImageNet/n01582220-18, D:/Dataset/Test/Mnist/Caffe/test, D:/Dataset/Voc/2012/test, D:/Dataset/Test/CameraBase/DeepLab-Test
	//GetFiles(files, "E:\\20220424_KunShanChunHua\\20220521_image\\deeplabv3-swintransformer\\datasets\\data\\VOCdevkit\\VOC2012\\JPEGImages", "*.png");
	//files = Utility::GetLines("C:/Asian-Celeb/Caffe/test.txt"); //C:/Asian-Celeb/Caffe/test.txt



	DWORD start = GetTickCount64();
	for (int i = 0; i < files.size(); i++)
	{
		std::string filename = files[i];

		//std::vector<std::string> items = Utility::Split(files[i], " ");
		//filename = "C:/" + items[0];

		// filename = "0001.bmp";
		cv::Mat img0 = cv::imread(filename, cv::IMREAD_COLOR);

		double min, max;
		cv::minMaxIdx(img0, &min, &max);
		cv::Mat result;
		img0.convertTo(result, CV_8UC1, 255.0 / (max - min), -255.0 * min / (max - min));
		cv::minMaxIdx(result, &min, &max);

		cv::Mat img1;
		cv::resize(result, img1, cv::Size(768, 768)); //224, imagenet, 112, ac, mnist, 416 cb, 448 voc and swin-transformer
		//cv::cvtColor(img1, img1, cv::COLOR_BGR2RGB);

		img1.convertTo(img1, CV_32F, 1.0 / 255.0);
#if 1
		cv::Mat channels[3];
		cv::split(img1, channels);
		channels[0] = (channels[0] - 0.485) / 0.229;
		channels[1] = (channels[1] - 0.456) / 0.224;
		channels[2] = (channels[2] - 0.406) / 0.225;
		cv::merge(channels, 3, img1);
#endif
		cv::Mat img2;
		cv::dnn::blobFromImage(img1, img2);
		net.setInput(img2);

		DWORD start = GetTickCount();
		cv::Mat output = net.forward("output");
		DWORD end = GetTickCount();
		printf("%d\n", end - start);
		printf(" ------ 这里 ---- %d, %d, %d, %d\n", output.size[0], output.size[1], output.size[2], output.size[3]);
#if 1
		//cv::Mat img4 = cv::Mat::zeros(img1.size(), CV_32F);
		std::vector<cv::Mat> vec_img4;
		for (int i = 0; i < output.size[1]; i++)
			vec_img4.push_back(cv::Mat::zeros(img1.size(), CV_32F));

		// 处理原图
		for (int y = 0; y < img1.rows; y++)
			for (int x = 0; x < img1.cols; x++)
			{
				float max_value = 0.0;
				for (int z = 0; z < output.size[1]; z++)
				{
					float value = *(((float*)output.data) + z * img1.rows * img1.cols + y * img1.cols + x);
					if (value >= max_value)
					{
						max_value = value;
						//img4.at<float>(y, x) = z
						vec_img4[z].at<float>(y, x) = value;
					}
				}
			}

		cv::imwrite(Format("Output/%d-0.png", i), 255 * vec_img4[0]);
		cv::imwrite(Format("Output/%d-1.png", i), 255 * vec_img4[1]);
		cv::imwrite(Format("Output/%d-2.png", i), 255 * vec_img4[2]);
		cv::imwrite(Format("Output/%d-3.png", i), 255 * vec_img4[3]);
		/*cv::imwrite("1.png",255 * vec_img4[0]);
		cv::imwrite("2.png", 255 * vec_img4[0]);
		cv::imwrite("3.png", 255 * vec_img4[0]);*/

		continue;

		cv::Mat img5 = cv::Mat::zeros(img1.size(), CV_8UC3);
		cv::Mat img6;
		cv::resize(img0, img6, img1.size());
		/*
				for (int y=0; y<img4.rows; y++)
					for (int x=0; x<img4.cols; x++)
					{
						if (img4.at<uchar>(y, x) >= 1)
						{
							int nCls = img4.at<uchar>(y, x) % (sizeof(MyColor)/sizeof(cv::Scalar)) - 1;
							img5.at<cv::Vec3b>(y, x)[0] = MyColor[nCls][0];
							img5.at<cv::Vec3b>(y, x)[1] = MyColor[nCls][1];
							img5.at<cv::Vec3b>(y, x)[2] = MyColor[nCls][2];
							img6.at<cv::Vec3b>(y, x)[0] = img6.at<cv::Vec3b>(y, x)[0]*1/4 + MyColor[nCls][0]*3/4;
							img6.at<cv::Vec3b>(y, x)[1] = img6.at<cv::Vec3b>(y, x)[1]*1/4 + MyColor[nCls][1]*3/4;
							img6.at<cv::Vec3b>(y, x)[2] = img6.at<cv::Vec3b>(y, x)[2]*1/4 + MyColor[nCls][2]*3/4;
						}
					}
		*/
#else
		/*
		for (int x=0; x<output.cols; x++)
			printf("%d %.2f,\t", x, output.at<float>(0, x));
		*/

		cv::Point cls;
		cv::minMaxLoc(output.reshape(1, 1), 0, 0, 0, &cls);

		cv::Mat img6;
		cv::resize(img0, img6, cv::Size(112 * 4, 112 * 4));
		PrintText(img6, Utility::Format("%d", cls.x), cv::Point(img6.cols / 2, img6.rows / 2), cv::FONT_HERSHEY_SIMPLEX, 3, CV_RGB(0, 0, 255), 8);
#endif
#if 0
		cv::imshow("1", img6);
		cv::waitKey();
		break;
#else
		std::vector<std::string> items = Split(filename, "/");
		cv::imwrite(Format("D:/Dataset/Test/Counter/Output/%s", items[items.size() - 1].c_str()), img6);
#endif
		printf("%d, %d, %s\n", i, files.size(), files[i].c_str());
	}

	DWORD end = GetTickCount64();
	if (files.size() >= 1)
		printf("%lu, %lu, %lu, %lu\n", end, start, files.size(), (end - start) / files.size());

	getchar();
	return 1;
}



///////////////////////////
void GetFiles(vector<string>& files, string path, string filter) {
	intptr_t   handle = 0;
	struct _finddata_t info;

	if ((handle = _findfirst((path + "/" + filter).c_str(), &info)) != -1)
	{
		do
		{
			if ((info.attrib & _A_SUBDIR))
			{
				if (strcmp(info.name, ".") != 0 && strcmp(info.name, "..") != 0)
					GetFiles(files, path + "/" + info.name, filter);
			}
			else files.push_back(path + "/" + info.name);
		} while (_findnext(handle, &info) == 0);

		_findclose(handle);
	}
}

string Format(string format, ...)
{
	va_list ap;
	va_start(ap, format);

	char text[1024] = { 0 };
	vsprintf_s(text, format.c_str(), ap);

	va_end(ap);

	return string(text);
}

vector<string> Split(const string& s, const string& delim)
{
	vector<string> dst;
	size_t last = 0;
	size_t X = s.find_first_of(delim, last);

	while (X != string::npos)
	{
		string szTmp = Trim(s.substr(last, X - last));
		if (szTmp.length() >= 1) dst.push_back(szTmp);
		last = X + delim.size();
		X = s.find_first_of(delim, last);
	}

	if (X - last > 0)
	{
		dst.push_back(Trim(s.substr(last, X - last)));
	}

	return dst;
}


string Trim(const string& s)
{
	char dst[MAX_BUF_LEN] = { 0 };
	strcpy_s(dst, s.c_str());

	while (dst[0] == ' ')
	{
		for (int i = 0; i <= strlen(dst); i++)
			dst[i] = dst[i + 1];
	}

	while (dst[strlen(dst) - 1] == '\r' || dst[strlen(dst) - 1] == '\n' || dst[strlen(dst) - 1] == ' ')
	{
		dst[strlen(dst) - 1] = 0;
	}

	return string(dst);
}