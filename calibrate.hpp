/*
 * calibrate.hpp
 *
 *  Created on: Sep 24, 2015
 *      Author: sean
 */

#ifndef CALIBRATE_HPP_
#define CALIBRATE_HPP_

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <getopt.h>

using namespace std;
using namespace cv;

enum Pattern
{
	CHESSBOARD              = 0,
	CIRCLES_GRID            = 1,
	ASYMMETRIC_CIRCLES_GRID = 2,
};

typedef struct Parameter
{
	cv::Size board_size;
	float square_size;
	int  use_camera;
	int  camera_index;
	char image_list[200];
	char output_path[200];
	int  view_distorted;
	int  pattern_type;

}Parameter;

int main_file(Parameter *pParam);
int main_camera(Parameter *pParam);


#endif /* CALIBRATE_HPP_ */
