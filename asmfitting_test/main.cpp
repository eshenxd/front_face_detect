/*
   front face detect by asm lib 
   code by Ethan
   2014/12/5
*/

#include "header.h"

#include "vjfacedetect.h"
#include "video_camera.h"

#include "asmlibrary.h"
#include "asmfitting.h"

using namespace std;

int count = 0;
static void DrawResult(IplImage* image, const asm_shape& shape)
{
	float rowx, rowy;
	float colx, coly;

	rowx = shape[59].x - shape[4].x;
	rowy = shape[59].y - shape[4].y;

	colx = shape[11].x - shape[54].x;
	coly = shape[11].y - shape[54].y;

	

	float tmp = shape[54].x - shape[59].x;


	cout << fabs(rowx - colx) / tmp << endl;
	if (fabs(rowx - colx) / tmp <= 0.15)
	{
		cout << "find a front face!" << endl;
		cvSaveImage("1.jpg", image, 0);
	}

	for (int j = 0; j < shape.NPoints(); j++)
		cvCircle(image, cvPoint(shape[j].x, shape[j].y), 2, CV_RGB(255, 0, 0));
}

int main(){
	asmfitting fit_asm;

	char* model_name = "point.amf";
	char* cascade_name = "face.xml";

	char* file_name = "test.jpg";
	int use_camera = 0;
	int image_or_video = -1;
	int i;
	int n_iteration = 100;
	int camera_idx = 0;


	if (fit_asm.Read(model_name) == false)
		return -1;

	if (init_detect_cascade(cascade_name) == false)
		return -1;


	asm_shape shape, detshape;
	bool flag = false;
	IplImage* image;
	int j = 0;

	if (open_camera(camera_idx) == false)
	{
		fprintf(stderr, "Can not open camera [%d]\n", camera_idx);
		exit(0);
	}

	cvNamedWindow("ASM-Search", 1);

	while (1)
	{
		// NOTE: when the parameter is set 1, we can read from camera
		image = read_from_camera();

		int nFaces;
		asm_shape *shapes = NULL, *detshapes = NULL;

		// step 1: detect face
		bool flag = detect_all_faces(&detshapes, nFaces, image);

		// step 2: initialize shape from detect box
		if (flag)
		{
			shapes = new asm_shape[nFaces];
			for (int i = 0; i < nFaces; i++)
			{
				InitShapeFromDetBox(shapes[i], detshapes[i], fit_asm.GetMappingDetShape(), fit_asm.GetMeanFaceWidth());
			}
		}
		else
		{
			fprintf(stderr, "This image doesnot contain any faces!\n");
			cvShowImage("ASM-Search", image);
			cvWaitKey(1);
			//exit(0);
			continue;
		}

		// step 3: image alignment fitting
		fit_asm.Fitting2(shapes, nFaces, image, n_iteration);

		// step 4: draw and show result in GUI
		for (int i = 0; i < nFaces; i++)
		{
			DrawResult(image, shapes[i]);
		}
		
		cvShowImage("ASM-Search", image);
		cvWaitKey(1);
	}

	close_camera();

	return 0;
}