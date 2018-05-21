/******************************************************************************
FileName     [ output.h ]
PackageName  [ AGS ]
Synopsis     [ Output Special Image Processing ]
Author       [ Jia-Wei (Jimmy) Liou ]
Copyright    [ Copyleft(c) 2018-present LaDF, CE-Hydrolic, NTU, Taiwan ]
******************************************************************************/

#include "stdafx.h"
#include "basic.h"

int findroot(int labeltable[], int label)
{
	int x = label;
	while (x != labeltable[x]) {
		x = labeltable[x];
	}
	return x;
}

int bwlabel(InputArray _binary, OutputArray _labelImg, int nears)
{
	Mat binary = _binary.getMat();
	CV_Assert(binary.type() == CV_8UC1);

	_labelImg.create(binary.size(), CV_32SC1);
	Mat labelImg = _labelImg.getMat();
	labelImg = Scalar(0);

	if (nears != 4 && nears != 6 && nears != 8) { nears = 8; }

	int nobj = 0;    // number of objects found in image  
	int *labeltable = new int[binary.rows*binary.cols]();		// initialize label table with zero  
	int ntable = 0;

	//	labeling scheme
	//	+ - + - + - +
	//	| D | C | E |
	//	+ - + - + - +
	//	| B | A |   |
	//	+ - + - + - +
	//	A is the center pixel of a neighborhood.In the 3 versions of connectedness :
	//	4 : A connects to B and C
	//	6 : A connects to B, C, and D
	//	8 : A connects to B, C, D, and E

	for (size_t i = 0; i < binary.rows; ++i) {
		for (size_t j = 0; j < binary.cols; ++j) {
			if (binary.at<uchar>(i, j)) {   // if A is an object  
											// get the neighboring labels B, C, D, and E
				int B = (!j) ? 0 : findroot(labeltable, labelImg.at<int>(i, j - 1));
				int C = (!i) ? 0 : findroot(labeltable, labelImg.at<int>(i - 1, j));
				int D = (!i || !j) ? 0 : findroot(labeltable, labelImg.at<int>(i - 1, j - 1));
				int E = (!i || j == binary.cols - 1) ? 0 : findroot(labeltable, labelImg.at<int>(i - 1, j + 1));

				// apply 4 connectedness  
				if (nears == 4) {
					if (B && C) {	// B and C are labeled  
						labelImg.at<int>(i, j) = B;
						if (B != C) { labeltable[C] = B; }
					} else if (B) { // B is object but C is not  
						labelImg.at<int>(i, j) = B;
					} else if (C) {	// C is object but B is not  
						labelImg.at<int>(i, j) = C;
					} else {	// B, C, D not object - new object label and put into table  
						labelImg.at<int>(i, j) = labeltable[ntable] = ++ntable;
					}
					// apply 6 connected ness  
				} else if (nears == 6) {
					if (D) {	// D object, copy label and move on  
						labelImg.at<int>(i, j) = D;
					} else if (B && C) {	// B and C are labeled  
						if (B == C) {
							labelImg.at<int>(i, j) = B;
						} else {
							int tlabel = B < C ? B : C;
							labeltable[B] = tlabel;
							labeltable[C] = tlabel;
							labelImg.at<int>(i, j) = tlabel;
						}
					} else if (B) {	// B is object but C is not  
						labelImg.at<int>(i, j) = B;
					} else if (C) {	// C is object but B is not   
						labelImg.at<int>(i, j) = C;
					} else { // B, C, D not object - new object label and put into table
						labelImg.at<int>(i, j) = labeltable[ntable] = ++ntable;
					}
					// apply 8 connectedness  
				} else if (nears == 8) {
					if (B || C || D || E) {
						int tlabel;

						if (B) {
							tlabel = B;
						} else if (C) {
							tlabel = C;
						} else if (D) {
							tlabel = D;
						} else if (E) {
							tlabel = E;
						}

						labelImg.at<int>(i, j) = tlabel;

						if (B && B != tlabel) { labeltable[B] = tlabel; }
						if (C && C != tlabel) { labeltable[C] = tlabel; }
						if (D && D != tlabel) { labeltable[D] = tlabel; }
						if (E && E != tlabel) { labeltable[E] = tlabel; }
					} else { // label and put into table
						labelImg.at<int>(i, j) = labeltable[ntable] = ++ntable;
					}
				}
			} else { // A is not an object so leave it
				labelImg.at<int>(i, j) = 0;
			}
		}
	}

	// consolidate component table  
	for (size_t i = 0; i <= ntable; ++i) {
		labeltable[i] = findroot(labeltable, i);
	}

	// run image through the look-up table  
	for (size_t i = 0; i < binary.rows; ++i) {
		for (size_t j = 0; j < binary.cols; ++j) {
			labelImg.at<int>(i, j) = labeltable[labelImg.at<int>(i, j)];
		}
	}

	// count up the objects in the image 
	//clear all table label
	for (size_t i = 0; i <= ntable; ++i) {
		labeltable[i] = 0;
	}
	//calculate all label numbers
	for (size_t i = 0; i < binary.rows; ++i) {
		for (size_t j = 0; j < binary.cols; ++j) {
			++labeltable[labelImg.at<int>(i, j)];
		}
	}

	labeltable[0] = 0;		//clear 0 label
							// number the objects from 1 through n objects  and reset label table
	for (size_t i = 1; i <= ntable; ++i) {
		if (labeltable[i] > 0) {
			labeltable[i] = ++nobj;
		}
	}

	// run through the look-up table again  
	for (size_t i = 0; i < binary.rows; ++i) {
		for (size_t j = 0; j < binary.cols; ++j) {
			labelImg.at<int>(i, j) = labeltable[labelImg.at<int>(i, j)];
		}
	}

	delete[] labeltable;

	return nobj;
}

void DeleteEdge(InputArray _binary, OutputArray _object)
{
	Mat binary = _binary.getMat();

	_object.create(binary.size(), CV_8UC1);
	Mat object = _object.getMat();
	binary.copyTo(object);

	for (size_t i = 0; i < object.cols; ++i) {
		if (object.at<uchar>(0, i) == 255) {
			floodFill(object, Point(i, 0), 0);
		}
		if (object.at<uchar>(object.rows - 1, i) == 255) {
			floodFill(object, Point(i, object.rows - 1), 0);
		}
	}
	for (size_t i = 0; i < object.rows; ++i) {
		if (object.at<uchar>(i, 0) == 255) {
			floodFill(object, Point(0, i), 0);
		}
		if (object.at<uchar>(i, object.cols - 1) == 255) {
			floodFill(object, Point(object.cols - 1, i), 0);
		}
	}
}

void DrawEllipse(InputArray _object, OutputArray _ellipseImg, vector<Size2f> &ellipse_param, vector<Size2f> &square_param)
{
	Mat object = _object.getMat();

	_ellipseImg.create(object.size(), CV_8UC1);
	Mat ellipseImg = _ellipseImg.getMat();
	ellipseImg = Scalar(0);

	Mat labels;
	int objectNum = bwlabel(object, labels, 4);

	Mat elementO = (Mat_<uchar>(3, 3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
	Mat objectErode;
	morphologyEx(object, objectErode, MORPH_ERODE, elementO);

	for (size_t i = 0; i < object.rows; ++i) {
		for (size_t j = 0; j < object.cols; ++j) {
			labels.at<int>(i, j) = objectErode.at<uchar>(i, j) ? 0 : labels.at<int>(i, j);
		}
	}

	vector<vector<Point2i>> pointset1;
	vector<vector<Point2i>> pointset2;
	for (size_t i = 0; i < objectNum; ++i) {
		pointset1.push_back(vector<Point2i>());
		pointset2.push_back(vector<Point2i>());

		pointset2[i].push_back(Point2i(object.cols, 0));	//left
		pointset2[i].push_back(Point2i(0, object.rows));	//top
		pointset2[i].push_back(Point2i(0, 0));				//right
		pointset2[i].push_back(Point2i(0, 0));				//down
	}

	for (size_t i = 0; i < labels.rows; ++i) {
		for (size_t j = 0; j < labels.cols; ++j) {
			if (labels.at<int>(i, j)) {
				pointset1[labels.at<int>(i, j) - 1].push_back(Point2i(j, i));

				if (j < pointset2[labels.at<int>(i, j) - 1][0].x) {
					pointset2[labels.at<int>(i, j) - 1][0] = Point2i(j, i);
				} else if (j > pointset2[labels.at<int>(i, j) - 1][2].x) {
					pointset2[labels.at<int>(i, j) - 1][2] = Point2i(j, i);
				}

				if (i < pointset2[labels.at<int>(i, j) - 1][1].y) {
					pointset2[labels.at<int>(i, j) - 1][1] = Point2i(j, i);
				} else if (i > pointset2[labels.at<int>(i, j) - 1][3].y) {
					pointset2[labels.at<int>(i, j) - 1][3] = Point2i(j, i);
				}
			}
		}
	}

	for (size_t i = 0; i < objectNum; ++i) {
		float length = 0;
		if (pointset1[i].size() > 5) {
			RotatedRect ellipse_obj = fitEllipse(pointset1[i]);

			//draw ellipse
			ellipse(ellipseImg, ellipse_obj, Scalar(255), 1, CV_AA);

			// store ellipse
			ellipse_param.push_back(ellipse_obj.size);

			for (size_t j = 0; j < 3; ++j) {
				for (size_t k = j + 1; k < 4; ++k) {
					float temp = sqrt(pow(pointset2[i][j].x - pointset2[i][k].x, 2) + pow(pointset2[i][j].y - pointset2[i][k].y, 2));
					length = length > temp ? length : temp;
				}
			}

			Size2f param = Size2f(length, sqrt((float)(pow(pointset2[i][2].x - pointset2[i][0].x, 2) + pow(pointset2[i][3].y - pointset2[i][1].y, 2))));
			square_param.push_back(param);
		}
	}
}