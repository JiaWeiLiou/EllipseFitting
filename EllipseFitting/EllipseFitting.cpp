// EllipseFitting.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "basic.h"
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>

int main()
{
	std::cout << "Please enter image path : ";
	string imgfile;
	std::cin >> imgfile;

	/* Set Output File Name */

	int pos1 = imgfile.find_last_of('/\\');
	int pos2 = imgfile.find_last_of('.');
	string filepath(imgfile.substr(0, pos1));							//file path
	string infilename(imgfile.substr(pos1 + 1, pos2 - pos1 - 1));		//file name
	string infilefullname(imgfile.substr(pos1 + 1));					//file full name

	/*Read Raw Image*/

	Mat image = cv::imread(imgfile, 0);			//raw image (8UC1)
	if (!image.data) {
		printf("Oh no! reading image error... \n");
		return false;
	}

	std::cout << "Image's width  : " << image.cols << endl;
	std::cout << "Image's height : " << image.rows << endl;

	float rl = image.cols;
	float pl = image.cols;
	

	for (size_t i = 0; i < image.rows; ++i) {
		for (size_t j = 0; j < image.cols; ++j) {
			image.at<uchar>(i, j) = image.at<uchar>(i, j) > 127 ? 255 : 0;
		}
	}

	/* Delete Edge object */

	Mat objectDE;		//8UC1(BW)
	DeleteEdge(image, objectDE);

	/* Fitting Ellipse */

	Mat objectFE;		//8UC1(BW)
	vector<Size2f> ellipse;
	vector<Size2f> square;
	DrawEllipse(objectDE, objectFE, ellipse, square);

	ofstream outfileM, outfileL, outfileSL, outfileSD;
	string outputPathM = filepath + "\\" + "STAND_AGS_M(PSD).txt";
	string outputPathL = filepath + "\\" + "STAND_AGS_L(PSD).txt";
	outfileM.open(outputPathM, ios::out | ios::trunc);
	outfileL.open(outputPathL, ios::out | ios::trunc);

	vector<float> outAxisM, outAxisL;
	for (size_t i = 0; i < ellipse.size(); ++i) {
		float wAxis = ellipse[i].width * rl / pl;
		float hAxis = ellipse[i].height * rl / pl;
		if (wAxis < hAxis) {
			outAxisM.push_back(wAxis);
			outAxisL.push_back(hAxis);
		} else {
			outAxisM.push_back(hAxis);
			outAxisL.push_back(wAxis);
		}
	}

	std::sort(outAxisM.begin(), outAxisM.end());
	std::sort(outAxisL.begin(), outAxisL.end());

	outfileM << infilefullname << ":\t";
	outfileL << infilefullname << ":\t";

	for (size_t i = 0; i < outAxisM.size(); ++i) {
		outfileM << outAxisM[i];
		if (i != outAxisM.size() - 1) {
			outfileM << "\t";
		}
	}


	for (size_t i = 0; i < outAxisL.size(); ++i) {
		outfileL << outAxisL[i];
		if (i != outAxisL.size() - 1) {
			outfileL << "\t";
		}
	}

	outfileM << endl;
	outfileL << endl;

	outfileM.close();
	outfileL.close();

    return 0;
}

