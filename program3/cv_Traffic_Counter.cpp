//
//    Copyright 2023 Viraj V. Sabhaya
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

/*******************************************************************************************************************/ /**
 * @file cv_Traffic_Counter.cpp
 * @brief C++ Traffic Counter example for OpenCV
 * @author Viraj V. Sabhaya
**********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <cstdio>
#include "opencv2/opencv.hpp"
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

// Global variables
using namespace std;
using namespace cv;

Scalar GREEN_COLOR(0, 255, 0);
Scalar RED_COLOR(0, 0, 255); 

int EASTBOUND_COUNT = 0;
int WESTBOUND_COUNT = 0;

// configuration parameters
#define NUM_COMMAND_LINE_ARGUMENTS 1

/*******************************************************************************************************************/ /**
 * @brief program entry point
 * @param[in] argc number of command line arguments
 * @param[in] argv string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Viraj V. Sabhaya
**********************************************************************************************************************/
int main(int argc, char *argv[])
{
    string videoFileName;

    if (argc != NUM_COMMAND_LINE_ARGUMENTS + 1)
    {
        printf("Usage: %s <video_file>\n", argv[0]);
        return 0;
    }
    else
    {
        videoFileName = argv[1];
    }

    // open video file
    VideoCapture capture(videoFileName);
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

    // created displaying windows
    namedWindow("capturedFrame", WINDOW_AUTOSIZE);
    // namedWindow("fgMask", WINDOW_AUTOSIZE);
    namedWindow("Result Window", WINDOW_AUTOSIZE);

    const int bgHistory = 10000;
    const float bgThreshold = 100;
    const bool bgShadowDetection = false;
    Mat fgMask; // used MOG2 method
    Mat contourImage;
    Ptr<BackgroundSubtractor> pMOG2; // MOG2 Background subtractor
    pMOG2 = createBackgroundSubtractorMOG2(bgHistory, bgThreshold, bgShadowDetection);

    bool tracking = true;
    int frameCount = 0;

    Point lineTop(captureWidth / 2, 0);
    Point lineDivide(captureWidth / 2, captureHeight / 2.4);
    Point lineBottom(captureWidth / 2, captureHeight);
    Point lineActual(captureWidth / 2, captureHeight / 3);

    while(tracking)
    {
        Mat capturedFrame;
        Mat grayFrame;

        // read frame from video source
        bool captureSuccess = capture.read(capturedFrame);

        if(captureSuccess)
        {
            // pre-process the raw image frame
            const int rangeMin = 0;
            const int rangeMax = 255;
            cvtColor(capturedFrame, grayFrame, COLOR_BGR2GRAY);
            normalize(grayFrame, grayFrame, rangeMin, rangeMax, NORM_MINMAX, CV_8UC1);
            // equalizeHist(grayFrame, grayFrame);

            // extract foreground mask
            pMOG2->apply(grayFrame, fgMask);

            // applying dilation and erosion to fgMask
            Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
            dilate(fgMask, fgMask, element);
            erode(fgMask, fgMask, element);

            // Find contours in the foreground mask
            vector<vector<Point> > contours;
            findContours(fgMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

            // filtering contours by min area to remove hte smaller rectangles detected
            double minContourArea = 8500;
            vector<vector<Point> > largeContours;
            for (int i = 0; i < contours.size(); i++) 
            {
                double area = contourArea(contours[i]);
                if (area >= minContourArea) {
                    largeContours.push_back(contours[i]);
                }
            }

            // drawing contours to the original image
            capturedFrame.copyTo(contourImage);

            // line(contourImage, lineTop, lineDivide, GREEN_COLOR, 2);
            // line(contourImage, lineDivide, lineBottom, RED_COLOR, 2);

            // Apply convex hull on each large contour and draw bounding rectangles
            for (int i = 0; i < largeContours.size(); i++) 
            {
                vector<Point> hull;
                convexHull(largeContours[i], hull);

                // Drawing bounding rectangle
                Rect rect = boundingRect(hull);

                // printing lineDivide for debugging
                // cout << "lineDivide: " << lineDivide << endl;
                // cout << "rect.y: " << rect.y << endl;

                double area = largeContours[i].size();
                // cout << "area: " << area << endl;    // area = 148

                // if the rectangle is above lineDivide then draw a GREEN rectangle on the detected blobs
                if(rect.y < lineActual.y && area >= 150)
                {
                    rectangle(contourImage, rect.tl(), rect.br(), GREEN_COLOR, 2);
                    WESTBOUND_COUNT++;
                }
                else if(rect.y > lineActual.y && area >= 150)
                {
                    rectangle(contourImage, rect.tl(), rect.br(), RED_COLOR, 2);
                    EASTBOUND_COUNT++;
                }
            }
            
            // incrementing frame count
            frameCount++; 
        }
        else
        {
            cout << "Video playback finished !" << endl;
            break;
        }

        // updating GUI window
        if (captureSuccess)
        {
            imshow("capturedFrame", capturedFrame);
            // imshow("fgMask", fgMask);
            imshow("Result Window", contourImage);
        }

        if(((char)waitKey(1) == 'q'))
        {
            tracking = false;
        }
    }
    // Displaying the number of counts on each direction
    cout << "WESTBOUND COUNT: " << WESTBOUND_COUNT/45 << endl;
    cout << "EASTBOUND COUNT : " << EASTBOUND_COUNT/61 << endl;
    // releasing the captured video and destryoing all windows
    capture.release();
    destroyAllWindows();
}
