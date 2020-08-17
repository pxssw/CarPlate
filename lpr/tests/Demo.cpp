#include "../include/Pipeline.h"
#include<fstream>
#include<vector>
using namespace std;


#include "io.h"
void filesearch(string path, int layer, vector <string> &filepathnames)
{
	struct _finddata_t   filefind;
	//string  curr = path + "\\*.*";
	string  curr = path + "\\*.*";
	int   done = 0;
	intptr_t handle;
	if ((handle = _findfirst(curr.c_str(), &filefind)) == -1)
		return;

	while (!(done = _findnext(handle, &filefind)))
	{
		//printf("���Ե�--%s\n", filefind.name);
		if (!strcmp(filefind.name, "..")) {
			continue;
		}
		if ((_A_SUBDIR == filefind.attrib))
		{
			curr = path + "\\" + filefind.name;
			filesearch(curr, layer + 1, filepathnames);
		}
		else
		{
			curr = path + "\\" + filefind.name;
			int pos = curr.find_last_of(".");
			string format1 = curr.substr(pos + 1);
			if (format1 == "jpg" || format1 == "bmp" || format1 == "JPG" || format1 == "jpeg" || format1 == "png" || format1 == "avi" || format1 == "gif")
				filepathnames.push_back(curr);
		}
	}
	_findclose(handle);
}

#include <iostream>  
#include <fstream>
#include <windows.h>
//<!string->LPCWSTR
LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring;
}

pr::PipelinePR* init()
{
	string cascade_dir = "../lpr/model/cascade.xml";
	string horFineMapProt_dir = "../lpr/model/HorizonalFinemapping.prototxt";
	string horFineMapMode_dir = "../lpr/model/HorizonalFinemapping.caffemodel";
	string segPort_dir = "../lpr/model/Segmentation.prototxt";
	string segMode_dir = "../lpr/model/Segmentation.caffemodel";
	string charRegProt_dir = "../lpr/model/CharacterRecognization.prototxt";
	string charRegMode_dir = "../lpr/model/CharacterRecognization.caffemodel";
	string segmenProt_dir = "../lpr/model/SegmenationFree-Inception.prototxt";
	string segmenMode_dir = "../lpr/model/SegmenationFree-Inception.caffemodel";

	pr::PipelinePR* prc = new pr::PipelinePR(cascade_dir.c_str(),horFineMapProt_dir.c_str(),horFineMapMode_dir.c_str(),
											segPort_dir.c_str(),segMode_dir.c_str(),
											charRegProt_dir.c_str(), charRegMode_dir.c_str(),
											segmenProt_dir.c_str(),segmenMode_dir.c_str());
	if (prc != nullptr)
		return prc;
	else
		return nullptr;
}


int release(pr::PipelinePR* testPr)
{	
	if (testPr != nullptr)
	{
		delete testPr;
		testPr = nullptr;
		return 0;
	}
	return 1;
}

int main()
{	
	auto* testPr = init();
	if (testPr)
	{
		string picture_path = "../Data/Test1";
		string errorSavePath = "../Result/Error";
		vector<string> files;
		filesearch(picture_path, 0, files);
		int testImgNum = files.size();
		int accCount = 0;
		int errCount = 0;
		
		for (int i = 0; i < files.size(); i++)
		{
			string input_img_dir = files[i];
			string input_name = input_img_dir.substr(input_img_dir.find("_")+1, input_img_dir.length());
			input_name = input_name.substr(0, input_name.find("_"));

			cv::Mat image = cv::imread(input_img_dir);
			std::vector<pr::PlateInfo> res = testPr->RunPiplineAsImage(image, pr::SEGMENTATION_FREE_METHOD);

			bool correct = false;
			for (auto st : res)
			{
				if (st.confidence > 0.75)
				{
					//std::cout << st.getPlateName() << " " << st.confidence << std::endl;
					string result = st.getPlateName();
					cv::Rect region = st.getPlateRect();
					cv::rectangle(image, cv::Point(region.x, region.y), cv::Point(region.x + region.width, region.y + region.height), cv::Scalar(255, 255, 0), 2);

					if (result == input_name)
					{	
						correct = true;
						accCount++;
						break;
					}				
				}
			}
			cv::imshow("image", image);
			cv::waitKey(1);

			/*if (!correct)
			{
				errCount++;
				string errorResulSaveDir = errorSavePath + "/"+ input_name+".jpg";
				CopyFile(stringToLPCWSTR(input_img_dir), stringToLPCWSTR(errorResulSaveDir), FALSE);
			}*/
			cout << "total img is " << testImgNum << " Processing " << (i + 1) << " the acc is " << (float(accCount) / float(i + 1)) << endl;
		}
	}	
	release(testPr);
}
