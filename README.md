This program will calibrate your camera, and generate camera_<date>.data and camera_<date>.yml 
camera_<date>.data will store the camera matrix and distort coefficient in binary 
	 it contains 3x3 double elements and 5 double elements 

camera_<date>.yml will store the same data in .yml file format
	 it contains "cameraMatrix" and "distCoeffs" 

Usage: tool_calibrate  <options> 
******** options: **********************
	--camera        : set 1(default) for camera on line calibration
	--file_list     : for inputting images list
	--board_size    : set board size, the number of blocks in you chess board
	--square_size   : set the length of block, in meter
	--output_path   : set the output result path
	--view_distorted: set 1(default) for display the undistorted images, 0 for not
	--pattern_type  : set 0 for chess board, 1 for circles grid, 2 for asymmetric circles grid
******** options end ******************

NOTES:
you should set --board_size and --square_size, and be care of that --square_size should be in meters
