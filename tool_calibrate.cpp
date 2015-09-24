/*
 * this application will calibrate cameras with several images of chessboard
 */

#include "./calibrate.hpp"

void print_help()
{
	printf("This program will calibrate your camera, and generate camera_<date>.data and camera_<date>.yml \n");
	printf("camera_<date>.data will store the camera matrix and distort coefficient in binary \n");
	printf("\t it contains 3x3 double elements and 5 double elements \n\n");
	printf("camera_<date>.yml will store the same data in .yml file format\n");
	printf("\t it contains \"cameraMatrix\" and \"distCoeffs\" \n\n");
	printf("Usage: tool_calibrate  <options> \n");
	printf("******** options: **********************\n");
	printf("\t--camera        : set 1(default) for camera on line calibration\n");
	printf("\t--file_list     : for inputting images list\n");
	printf("\t--board_size    : set board size, the number of blocks in you chess board\n");
	printf("\t--square_size   : set the length of block, in meter\n");
	printf("\t--output_path   : set the output result path\n");
	printf("\t--view_distorted: set 1(default) for display the undistorted images, 0 for not\n");
	printf("\t--pattern_type  : set 0 for chess board, 1 for circles grid, 2 for asymmetric circles grid\n");
	printf("******** options end ******************\n");

	printf("you should set --board_size and --square_size, and be care of that --square_size should be in meters\n");


}

int main(int argc, char * argv[])
{
	Parameter param;
	const char* const short_options = "hc:f:b:s:o:v:p:";
	const struct option long_options[] =
	{
		{"help",           0, NULL, 'h'},
		{"camera",         1, NULL, 'c'},
		{"file_list",      1, NULL, 'f'},
		{"board_size",     1, NULL, 'b'},
		{"square_size",    1, NULL, 's'},
		{"output_path",    1, NULL, 'o'},
		{"view_distorted", 1, NULL, 'v'},
		{"pattern_type",   1, NULL, 'p'},
		{NULL,             0, NULL, 0},
	};

	int next_opt = -1;

	print_help();

	param.use_camera        = 1;
	param.camera_index      = -1;
	param.board_size.width  = 8;
	param.board_size.height = 6;
	param.square_size       = 0.040;
	param.view_distorted    = 1;
	param.pattern_type      = 0;

	sprintf(param.output_path, ".");

	for(int i = 0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");

	do
	{
		next_opt = getopt_long(argc, argv, short_options, long_options, NULL);
		printf("%s \n", optarg);

		switch(next_opt)
		{
		case 'h':
			print_help();
			break;
		case 'c':
			param.use_camera = 1;
			param.camera_index = atoi(optarg);
			break;
		case 'f':
			param.use_camera = 0;
			sprintf(param.image_list, "%s", optarg);
			break;
		case 'b':
		{
			try
			{
				char temp[10];
				char *p0 = temp;
				char *p1 = NULL;
				sprintf(temp, "%s", optarg);
				p1 = strtok(p0, "x");
				param.board_size.width = atoi(p1);

				sprintf(temp, "%s", optarg);
				strtok_r(p0, "x", &p1);
				param.board_size.height = atoi(p1);
			}
			catch(Exception e)
			{
				cout << e.msg << endl;
				return -1;
			}

		}
			break;

		case 's':
			param.square_size = atof(optarg);
			break;
		case 'o':
			sprintf(param.output_path, "%s", optarg);
			break;
		case 'v':
			param.view_distorted = atoi(optarg);
			break;
		case 'p':
			param.pattern_type = atoi(optarg);
			break;
		default:
			printf("finish configure \n");
			break;
		}
	}while(next_opt != -1);

	printf("************** configuration **************** \n");

	if(param.use_camera == 1)
		printf("camera        : %d \n", param.camera_index);
	else
		printf("image_list    : %s \n", param.image_list);

	printf("board_size    : %d x %d \n", param.board_size.width, param.board_size.height);
	printf("square_size   : %f \n", param.square_size);
	printf("out_path      : %s \n", param.output_path);
	printf("view_distorted: %d \n", param.view_distorted);
	printf("pattern_type  : %d \n", param.pattern_type);

	printf("*********** end of configuration **************** \n");

	if(param.use_camera)
		main_camera(&param);
	else
		main_file(&param);
	return 0;
}

