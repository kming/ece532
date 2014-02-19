
/*******************************************************************
TrackWand is a custome pixel manipulation tracking algorithm.  

********************************************************************/


/*******************************************************************
TCV3501 - Computer Vision and Image Understanding
Farshid Tavakolizadeh (email@farshid.ws) http://farshid.ws
Copyrights goes to Mr. Wong Ya Ping (CVLab06)
Year: 18/1/2012
********************************************************************/
/********************************************************************
Noise Handing:
- using area to ignore the noise
- using magnitute limit to skip long distance points
- using 5x5 median filter to decrease the background noise

Control features:
- choosing line colors
- changing line thickness based on Y coordinates
- clearing the screen + saving the image
- exiting the program
********************************************************************/
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <iostream>
#include <cmath>

using namespace std;

#define blue  CV_RGB(0,0,255)
#define green CV_RGB(0,255,0)
#define red   CV_RGB(255,0,0)
#define white CV_RGB(255,255,255)
#define black CV_RGB(0,0,0)

struct coordinates {
	int x;
	int y;
};

void ClearScreen(IplImage* imgScribble, IplImage* imgDrawing)
{
    cvSet(imgScribble, black);
    cvSet(imgDrawing, white);
}

IplImage* GetThresholdedImage(IplImage* img, CvScalar& lowerBound, CvScalar& upperBound)
{
    // Convert the image into an HSV image
    IplImage* imgHSV = cvCreateImage(cvGetSize(img), 8, 3);
    cvCvtColor(img, imgHSV, CV_BGR2HSV);

    IplImage* imgThreshed = cvCreateImage(cvGetSize(img), 8, 1);

    cvInRangeS(imgHSV, lowerBound, upperBound, imgThreshed);

    cvReleaseImage(&imgHSV);
    return imgThreshed;
}

void TrackWand (CvMat *mat, coordinates *pos) 
{
	pos->x = -1;
	pos->y = -1;
	int xstep = 4;									// horizontal window size
	int ystep = 4;									// vertical window size
	int threshold = xstep * ystep * 30;				// number of pixels in the window * the total tolerance per pixel
	int intensity_min = xstep * ystep * 3 * 150;	// minimum intensity threshold, a minimum ensures that it is activated by a bright enough source, i.e a light, not a random object
	int intensity_max = xstep * ystep * 3 * 200;	// maximum intensity threshold, a maximum to ensure it isn't a bright white light
	int prev_sumofdiff = threshold;

	// Performance enhancement --> instead of a sliding window that shifts by 1 pixel, shifts over by the size of the window. (not as smooth, but much faster since it goes through a lot less pixels)
	for (int x = 0; x < mat->width-xstep; x = x+xstep)
	{
		for (int y = 0; y <mat->height-ystep; y = y+ystep)
		{
			// reset intensity and sum of difference calculations
			int sumofdiff = 0;
			int intensity = 0;

			// iterate over the whole window
			for (int offsetx = x;  offsetx < x+xstep; offsetx++)
			{
				for (int offsety = y;  offsety < y+ystep; offsety++) 
				{
					// Determine the RGB values for the pixel
					CvScalar scal = cvGet2D (mat, offsety, offsetx);
					// measures absolute difference of the blue component in the sliding window
					sumofdiff = sumofdiff + 255 - scal.val[0];

					// measure intensity of the whole sliding window
					intensity = intensity + scal.val[0] + scal.val[1] + scal.val[2];
				}
			}


			// Several checking factors:
			//  - Blue colour is dominant
			//  - Find the best match in the frame --> only 1
			//  - Intensity is high enough to be a light
			//  - Intensity isn't high enough to be a white light
			if ((sumofdiff < threshold) && (sumofdiff < prev_sumofdiff) && (intensity > intensity_min) && (intensity < intensity_max))
			{
				// the center of the sliding window is the x,y coordinates
				prev_sumofdiff = sumofdiff;
				pos->x = (int) ceil(x + xstep/2);
				pos->y = (int) ceil(y + ystep/2);
			}
		}
	}
}

int main()
{
    // controls
    double area_limit = 700;
    CvScalar lowerBound = cvScalar(20, 100, 100);  // yellow
    CvScalar upperBound = cvScalar(30, 255, 255);

    // defaults
    int lineThickness = 2;
    CvScalar lineColor = blue;


    CvCapture* capture = 0;
    capture = cvCaptureFromCAM(0);
    if(!capture)
    {
        cout << "Could not initialize capturing...\n";
        return -1;
    }

    // This image holds the "scribble" data...
    // the tracked positions of the pointer object
    IplImage* imgScribble = NULL;


    IplImage* imgColorPanel = 0;
    imgColorPanel = cvLoadImage( "cvPaint.panel",  CV_LOAD_IMAGE_COLOR ); // load the panel image. (This is a png image, not designed/included in the source code!) 
    if(!imgColorPanel)
    {
        cout << "cvPaint.panel is not found !!! \n";
        return -1;
    }

    IplImage* imgDrawing = 0;
    imgDrawing = cvCreateImage( cvSize(cvQueryFrame(capture)->width,cvQueryFrame(capture)->height),
                                cvQueryFrame(capture)->depth,     //Bit depth per channel
                                3  //number of channels
                              );
    cvSet(imgDrawing, white);

    CvFont font, fontbig;
    cvInitFont( &font, CV_FONT_HERSHEY_COMPLEX, 1, .6, 0, 2, CV_AA);
    cvInitFont( &fontbig, CV_FONT_HERSHEY_COMPLEX, 3, .6, 0, 3, CV_AA);

    int confirm_close = 10, confirm_clear = 20; // counters for clear and exit confirmation
    char buffer [50]; // buffer for cvPutText
    int image_num = 0; // to keep track of image numbers for saving
    int posX = 0;
    int posY = 0;

	

    while(true)
    {
        IplImage* frame = 0;
		IplImage* sod_frame = 0;
        frame = cvQueryFrame(capture);
        if(!frame)
            break;
        cvFlip(frame,NULL,1); // flip the frame to overcome mirroring problem

		
        // If this is the first frame, we need to initialize it
        if(imgScribble == NULL)
            imgScribble = cvCreateImage(cvGetSize(frame), 8, 3);

        // Median filter to decrease the background noise
        cvSmooth( frame, frame,
                  CV_MEDIAN,
                  5, 5 //parameters for filter, in this case it is filter size
                );

		// create matrix version of input frame
		//printf ("Convert to Matrix");
		CvMat *mat = cvCreateMat(frame->height, frame->width, CV_32FC3);
		cvConvert (frame, mat);
		
		// determine the coordinates from the image
		coordinates pos;
		pos.x = 0;
		pos.y = 0;
		
		// printf( " - MAT x: %d\n - MAT y: %d\n",mat->cols, mat->height);
		TrackWand(mat, &pos);
		
		//cvConvert (sum_of_diff, sod_frame);
		printf( "( %d, %d)\n",pos.x, pos.y);

		
        // Holds the thresholded image (tracked color -> white, the rest -> black)
        IplImage* imgThresh = GetThresholdedImage(frame,lowerBound,upperBound);

        // Calculate the moments to estimate the position of the object
        CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
        cvMoments(imgThresh, moments, 1);


        // The actual moment values
        double moment10 = cvGetSpatialMoment(moments, 1, 0);
        double moment01 = cvGetSpatialMoment(moments, 0, 1);
        double area = cvGetCentralMoment(moments, 0, 0);
		

        // Holding the last and current positions
        int lastX = posX;
        int lastY = posY;

		posX = pos.x;
        posY = pos.y;

		/*
        if(moment10/area>=0 && moment10/area < 1280 && moment01/area >=0 && moment01/area < 1280
           && area>area_limit )
        {
            posX = moment10/area;
            posY = moment01/area;
        }
		*/


        CvPoint cvpoint = cvPoint(150,30); // location of the text
        if(posX < 90 && posY > 400) // clear
        {
            lineColor = white; // white color works as eraser
            cvPutText( frame, "Eraser selected.", cvpoint, &font, white );
            sprintf (buffer, "Clearing the screen in %d",confirm_clear); // count-down for clearing the screen
            cvPutText( frame, buffer, cvPoint(150,70), &font, red );
            confirm_clear--;
            if(confirm_clear < 0) // confirm in 10 frames before clearing
            {
                confirm_clear = 20;
                sprintf (buffer, "d0%d.jpg",image_num++);
                cvSaveImage(buffer ,imgDrawing); // save the frame into an image
                ClearScreen(imgScribble,imgDrawing);
                cvPutText( frame, "Cleared the screen.", cvPoint(150,110), &font, white );
            }
        }
        else if(posX  > 540 && posY > 360)  // blue
        {
            lineColor = blue;
            cvPutText( frame, "Blue color selected.", cvpoint, &font, blue );
        }

        else if(posX  > 540 && posY > 200 && posY < 280) // green
        {
            lineColor = green;
            cvPutText( frame, "Green color selected.", cvpoint, &font, green );
        }

        else if(posX  > 540 && posY < 120) // red
        {
            lineColor = red;
            cvPutText( frame, "Red color selected.", cvpoint, &font, red );
        }

        else if(posX > 0 && posX  < 90 && posY > 0 && posY < 120) // exit
        {
            sprintf (buffer, "EXITING in %d",confirm_close);
            cvPutText( frame, buffer, cvpoint, &font, red );
            confirm_close--;
            if(confirm_close < 0) // confirm in 10 frames before exit
                break;
        }
        else if(posX < 90 && posY > 130 && posY < 390) // line thickness
        {
            lineThickness = 6 - ( posY/60-1 );  // change the thickness of line from 1 - 5 based on posY
        }


        sprintf (buffer, "%d",lineThickness);
        cvPutText( frame, buffer, cvPoint(40,255), &fontbig, lineColor );

		double diff_X = lastX-posX;
		double diff_Y = lastY-posY;
        double magnitude = sqrt(   pow(diff_X,2) + pow(diff_Y,2)   );
        // We want to draw a line only if its a valid position
        //if(lastX>0 && lastY>0 && posX>0 && posY>0)
        if(magnitude > 0 && magnitude < 100 && posX > 120 && posX<530)
        {
            // Draw a line from the previous point to the current point
            cvLine(imgDrawing, cvPoint(posX, posY), cvPoint(lastX, lastY), lineColor, lineThickness,CV_AA);
        }


        //cout << "position = " << posX << "\t" <<  posY << "\t";
        //cout << "moment = " << moment10 << "\t" <<  moment01 << "\n";
        //cout << "d->" << magnitude << endl;
        //cout << "area = " << area << endl;

        // Add the scribbling image and the frame...
        cvAdd(imgDrawing, imgScribble, imgDrawing);

        // Combine everything in frame
        cvAnd(frame, imgDrawing, frame);
        cvAnd(imgColorPanel, frame, frame);

        cvShowImage("Threshold", sod_frame);
        cvShowImage("Drawing", imgDrawing);
        cvShowImage("Video", frame);


        int c = cvWaitKey(10);
        if(c==27)  //ESC key
            break;
        //else if(c==49) // 1 key


        cvReleaseImage(&imgThresh);
		cvReleaseMat (&mat);
        delete moments;
    }
	
    cvReleaseCapture(&capture);
    cvReleaseImage(&imgColorPanel);
    cvReleaseImage(&imgScribble);

    return 0;
}