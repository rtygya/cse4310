/*******************************************************************************************************************//**
 * @file program3.cpp
 * @brief C++ example of traffic counting program in Open CV
 * @author Reety Gyawali (1001803756)
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <cstdio>
#include "opencv2/opencv.hpp"
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace std;
using namespace cv;

int EASTBOUND_COUNT = 0;
int WESTBOUND_COUNT = 0;

// configuration parameters
#define NUM_COMMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME "Vehicle Tracking"

int main(int argc, char *argv[])
{
    string filename;

    if (argc != NUM_COMMAND_LINE_ARGUMENTS + 1)
    {
        printf("Usage: %s <path_file>\n", argv[0]);
        return 0;
    }
    else
    {
        filename = argv[1];
    }

    // open video file
    VideoCapture capture(filename);
    if (!capture.isOpened())
    {
        cout << "Unable to open video source, terminating program!" << endl;
        return 0;
    }
    
    int captureWidth = capture.get(CAP_PROP_FRAME_WIDTH);
    int captureHeight = capture.get(CAP_PROP_FRAME_HEIGHT);
    int captureFPS = capture.get(CAP_PROP_FPS);
    cout << "Video source opened successfully!" << endl;
    cout << "Width: " << captureWidth << endl;
    cout << "Height: " << captureHeight << endl;
    cout << "FPS: " << captureFPS << endl;

    // create window
    namedWindow(DISPLAY_WINDOW_NAME, WINDOW_AUTOSIZE);


    const int bgHistory = 200;
    const float bgThreshold = 500;
    const bool bgShadowDetection = false;
    Mat fgMask; 
    Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(bgHistory, bgThreshold, bgShadowDetection);

    bool tracking = true;
    int frameCount = 0;

    while(tracking)
    {
        Mat capturedFrame;
        Mat grayFrame;

        // read frame from video source
        bool captureSuccess = capture.read(capturedFrame);

        if(captureSuccess)
        {
            // grayscaling and normalizing
            cvtColor(capturedFrame, grayFrame, COLOR_BGR2GRAY);
            normalize(grayFrame, grayFrame, 0, 255, NORM_MINMAX, CV_8UC1);

            // extract foreground mask
            pMOG2->apply(grayFrame, fgMask);

            // incrementing frame count
            frameCount++; 
        }
        else
        {
            cout << "Unable to read frame ..." << endl;
        }

        // updating GUI window
        if (captureSuccess)
        {
            imshow(DISPLAY_WINDOW_NAME, capturedFrame);
        }
        

        if(((char)waitKey(1) == 'q'))
        {
            tracking = false;
        }
    }

    // release captured video and destroy all windows
    capture.release();
    destroyAllWindows();
}