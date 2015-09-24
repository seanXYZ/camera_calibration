/*
 * calibrate.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: sean
 */


#include "./calibrate.hpp"

static double computeReprojectionErrors(
        const vector<vector<Point3f> >& objectPoints,
        const vector<vector<Point2f> >& imagePoints,
        const vector<Mat>& rvecs, const vector<Mat>& tvecs,
        const Mat& cameraMatrix, const Mat& distCoeffs,
        vector<float>& perViewErrors )
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); i++ )
    {
        projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i],
                      cameraMatrix, distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err*err/n);
        totalErr += err*err;
        totalPoints += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

static void calcChessboardCorners(Size boardSize, float squareSize, vector<Point3f>& corners, Pattern patternType = CHESSBOARD)
{
    corners.resize(0);

    switch(patternType)
    {
      case CHESSBOARD:
      case CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
        {
        	for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float(j*squareSize), float(i*squareSize), 0));
        }
        break;

      case ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
        {
        	for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
        }
        break;

      default:
        CV_Error(CV_StsBadArg, "Unknown pattern type\n");
    }
}

static int runCalibration( vector<vector<Point2f> > imagePoints,
                    Size imageSize, Size boardSize, Pattern patternType,
                    float squareSize, float aspectRatio,
                    int flags, Mat& cameraMatrix, Mat& distCoeffs,
                    vector<Mat>& rvecs, vector<Mat>& tvecs,
                    vector<float>& reprojErrs,
                    double& totalAvgErr)
{
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( flags & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = aspectRatio;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcChessboardCorners(boardSize, squareSize, objectPoints[0], patternType);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                    distCoeffs, rvecs, tvecs, flags|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
                    ///*|CV_CALIB_FIX_K3*/|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    printf("RMS error reported by calibrateCamera: %g\n", rms);

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    if(ok == false)
    	return -1;

    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return 0;
}


static int getFileName(const char* file_list, vector<string> &file_names)
{
	FILE *pf;
	string temp;
	char *pstr = NULL;

	pf = fopen(file_list, "r");

    size_t len = 0;
    ssize_t read;

	if(pf == NULL)
	{
		printf("fail to open file %s\n", file_list);
		return -1;
	}

	do
	{
		read = getline(&pstr, &len, pf);

		if(read > 0)
		{
			char tempchar[100];
			sprintf(tempchar, "%s", pstr);
			tempchar[read -1] = '\0'; // remove the last character '\n'
			temp.assign(tempchar);
			file_names.push_back(temp);
		}
		else
		{
			break;
		}


	}while(1);


	fclose(pf);

	return 0;
}

int save_result(const char* output_path, Mat& cameraMatrix, Mat& distCoeffs)
{
	time_t     now;
	struct tm *timenow;
	char name1[200];
	char name2[200];
	char time_str[50];

	time(&now);
	timenow   =   localtime(&now);
	printf("Local time is %s \n",asctime(timenow));

	sprintf(time_str, "%d_%d_%d_%d_%d",
			timenow->tm_year + 1900,
			timenow->tm_mon,
			timenow->tm_mday,
			timenow->tm_hour,
			timenow->tm_min);
	sprintf(name1, "%s/camera_%s.data", output_path, time_str);
	sprintf(name2, "%s/camera_%s.yml", output_path, time_str);

	FILE* pSave = fopen(name1, "w");

	if(pSave != NULL)
	{
		cout << cameraMatrix<<endl;
		cout << distCoeffs<<endl;

		fwrite(cameraMatrix.data, cameraMatrix.elemSize(), cameraMatrix.cols*cameraMatrix.rows, pSave);
		fwrite(distCoeffs.data, distCoeffs.elemSize(), distCoeffs.cols*distCoeffs.rows, pSave);

		printf("size_t of cameraMatrix = %d  size_t of distCoeffs = %d \n", cameraMatrix.elemSize(), distCoeffs.elemSize());

		fclose(pSave);
	}

	cv::FileStorage fs(name2, FileStorage::WRITE);
	fs <<"cameraMatrix"<<cameraMatrix;
	fs <<"distCoeffs"<<distCoeffs;

	return 0;
}


int main_file(Parameter *pParam)
{
	char *file_list = pParam->image_list;//"/home/sean/Pictures/calib_test1/dir.txt";

	Size boardSize(8,6);
	Size imageSize;
	int   flags = CV_CALIB_FIX_ASPECT_RATIO;
	float squareSize = pParam->square_size;
	float aspectRatio = 1.f;
	Mat cameraMatrix;
	Mat distCoeffs;
	int result = 0;

	// read frames from data file
	vector<vector<Point2f> > imagePointsSet;
	vector<Point2f> imagePoints;
	vector<string>  fileNames;

	fileNames.clear();
	imagePointsSet.clear();

	getFileName(file_list, fileNames);

	for(unsigned int i = 0; i < fileNames.size(); i++)
	{
		Mat framecv;
		int found = 0;

		framecv = imread(fileNames[i].c_str(), 0);

		if(framecv.cols <= 0 || framecv.rows <= 0 || framecv.data == NULL )
		{
			printf("finish chess board detection \n");
			break;
		}

		imagePoints.clear();
		imageSize.width = framecv.cols;
		imageSize.height = framecv.rows;

		found = findChessboardCorners(
						framecv,
						boardSize,
						imagePoints,
						CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

		if(found)
		{
			cornerSubPix(
					framecv,
					imagePoints,
					Size(11,11),
					Size(-1,-1),
					TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

			if(found)
			{
				drawChessboardCorners( framecv, boardSize, Mat(imagePoints), found);
				imshow("framecv_xx", framecv);
				waitKey(10);
			}

			imagePointsSet.push_back(imagePoints);

		}

	}


	// calibrate cameras
	if(1)
    {
	    vector<Mat> rvecs, tvecs;
	    vector<float> reprojErrs;
	    double totalAvgErr = 0;

	    result = runCalibration(imagePointsSet, imageSize, boardSize, CHESSBOARD, squareSize,
                   aspectRatio, flags, cameraMatrix, distCoeffs,
                   rvecs, tvecs, reprojErrs, totalAvgErr);
    }

	// test calibrate
	if(1)
	{
        Mat view, rview, map1, map2;
        int i;
        Size imageSize2;

        imageSize2.width  = 2 * imageSize.width;
        imageSize2.height = 2 * imageSize.height;

        initUndistortRectifyMap(cameraMatrix,
        						distCoeffs,
								Mat(),
                                getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
                                imageSize, CV_16SC2, map1, map2);


        for(i = 0; i < fileNames.size(); i++)
        {
        	view = imread(fileNames[i].c_str());
        	remap(view, rview, map1, map2, INTER_LINEAR);

        	imshow("rview", rview);
        	waitKey(0);
        }
	}

	// save
	if(result == 0 )
	{
		save_result(pParam->output_path, cameraMatrix, distCoeffs);
	}

}

int main_camera(Parameter *pParam)
{

	Size boardSize(8,6);
	Size imageSize;
	int   flags = CV_CALIB_FIX_ASPECT_RATIO;
	float squareSize = pParam->square_size;
	float aspectRatio = 1.f;
	Mat cameraMatrix;
	Mat distCoeffs;
	Mat frame;
	VideoCapture video;

	int flag_finish = 0;

	int result = 0;

	// read frames from data file
	vector<vector<Point2f> > imagePointsSet;
	vector<Point2f> imagePoints;
	vector<string>  fileNames;

	fileNames.clear();
	imagePointsSet.clear();

	video.open(pParam->camera_index);

	if(video.isOpened() != true)
	{
		printf("fail to open camera %d\n", pParam->camera_index);

		video.open(-1);

		if(video.isOpened() != true)
		{
			printf("fail to open the default camera, please make sure an accessible camera is connected \n");
			return -1;
		}
		else
		{
			printf("open the default camera\n");
		}
	}

	while(flag_finish == 0)
	{
		Mat framecv;
		int found = 0;

		video >> frame;

		cvtColor(frame, framecv, CV_RGB2GRAY);

		imshow("framecv", framecv); // for oberserving input
		waitKey(10);

		if(framecv.cols <= 0 || framecv.rows <= 0 || framecv.data == NULL )
		{
			printf("finish chess board detection \n");
			break;
		}

		imagePoints.clear();
		imageSize.width = framecv.cols;
		imageSize.height = framecv.rows;

		found = findChessboardCorners(
						framecv,
						boardSize,
						imagePoints,
						CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

		if(found)
		{
			cornerSubPix(
					framecv,
					imagePoints,
					Size(11,11),
					Size(-1,-1),
					TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

			if(found)
			{
				char key = 0;
				drawChessboardCorners( framecv, boardSize, Mat(imagePoints), found);
				imshow("framecv_xx", framecv);
				key = waitKey(0);
				if(key == 'c' || key == 'C') // not correct
					continue;
				else if(key == 'q' || key == 'Q')
					return 0;
				else if(key == 's' || key == 'S')
					flag_finish = 1;
			}

			printf("get a new chess board input\n");
			imagePointsSet.push_back(imagePoints);

		}
		else
		{
			printf("no found usable chess board\n");
		}

	}


	// calibrate cameras
	if(1)
    {
	    vector<Mat> rvecs, tvecs;
	    vector<float> reprojErrs;
	    double totalAvgErr = 0;

	    result = runCalibration(imagePointsSet, imageSize, boardSize, CHESSBOARD, squareSize,
                   aspectRatio, flags, cameraMatrix, distCoeffs,
                   rvecs, tvecs, reprojErrs, totalAvgErr);
    }

	// test calibrate
	if(1)
	{
        Mat view, rview, map1, map2;
        int i;
        Size imageSize2;

        imageSize2.width  = 2 * imageSize.width;
        imageSize2.height = 2 * imageSize.height;

        initUndistortRectifyMap(cameraMatrix,
        						distCoeffs,
								Mat(),
                                getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
                                imageSize, CV_16SC2, map1, map2);


        while(1)
        {
        	char key = 0;
        	video>>view;
        	remap(view, rview, map1, map2, INTER_LINEAR);

        	imshow("rview", rview);
        	key = waitKey(0);
        	if(key == 's')
        		break;
        	else if(key == 'q')
        		break;
        }
	}

	if(result == 0 )
	{
		save_result(pParam->output_path, cameraMatrix, distCoeffs);
	}

}


