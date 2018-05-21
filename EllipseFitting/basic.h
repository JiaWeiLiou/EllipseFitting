/******************************************************************************
FileName     [ output.h ]
PackageName  [ AGS ]
Synopsis     [ Output Special Image Processing ]
Author       [ Jia-Wei (Jimmy) Liou ]
Copyright    [ Copyleft(c) 2018-present LaDF, CE-Hydrolic, NTU, Taiwan ]
******************************************************************************/

#ifndef OUTPUT_H
#define OUTPUT_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <queue>
#include <cmath>
#include <limits>

using namespace std;
using namespace cv;

//Find Root
//labeltable : labels table
//label : label value for look up
//return : the root of the label
int findroot(int labeltable[], int label);

//Find Connect Object 
//binary : binary image(8UC1)
//labelImg : labels image(32SC1)
//nears : nears type(4/6/8)
//return : labels total number without 0(background)
int bwlabel(InputArray _binary, OutputArray _labelImg, int nears);

//Delete Edge object
//binary : binary image (8UC1(BW))
//object : object image (8UC1(BW))
void DeleteEdge(InputArray _binary, OutputArray _object);

//Fitting and Draw Ellipse
//object : object image(8UC1(BW))
//ellipse : ellipse image(8UC1(BW))
//ellipse_param : long and short axis of ellipse
//square_param : max length, diagonal
void DrawEllipse(InputArray _object, OutputArray _ellipseImg, vector<Size2f> &ellipse_param, vector<Size2f> &square_param);

#endif // OUTPUT_H