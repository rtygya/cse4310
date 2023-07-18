/*******************************************************************************************************************//**
 * @file program3.cpp
 * @brief C++ example of traffic counting program in Open CV
 * @author Reety Gyawali (1001803756)
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <cstdio>
#include <string>
#include "opencv2/opencv.hpp"
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME "Video Frame"



int main(int argc, char **argv)
{
    // store video capture parameters
    std::string fileName;

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <file_path> \n", argv[0]);
        return 0;
    }
    else
    {
        fileName = argv[1];
    }

    // open the video file
    cv::VideoCapture capture(fileName);
    if(!capture.isOpened())
    {
        std::printf("Unable to open video source, terminating program! \n");
        return 0;
    }

    // create image window
    //cv::namedWindow("fgmask", cv::WINDOW_AUTOSIZE);
	cv::namedWindow(DISPLAY_WINDOW_NAME, cv::WINDOW_AUTOSIZE);

    // Create the background subtractor
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(200.0, 60.0, false);

    // Define variables for counting cars and tracking directions
    int countLeftToRight = 0;
    int countRightToLeft = 0;
    cv::Rect leftRegion(0, capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);
    cv::Rect rightRegion(0, 0, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);

    // Create kernel for morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20, 20));

    bool doCapture = true;
    while(doCapture)
    {
        // attempt to acquire and process an image frame
        cv::Mat captureFrame;
        cv::Mat grayFrame;

        bool captureSuccess = capture.read(captureFrame);
        if(captureSuccess)
        {
			// pre-process the raw image frame
            cv::cvtColor(captureFrame, grayFrame, cv::COLOR_BGR2GRAY);
            cv::normalize(grayFrame, grayFrame, 0, 255, cv::NORM_MINMAX, CV_8UC1);

			// apply foreground mask to image
			pMOG2->apply(grayFrame, fgMask);

            // Dilate then erode objects
            cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
        
            // Find contours in the foreground mask
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            
            
            // erode and dilate the edges to remove noise
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20, 20));
            cv::Mat edgesDilated;
            cv::dilate(imageEdges, edgesDilated, kernel);
            cv::dilate(edgesDilated, edgesDilated, kernel);
            cv::Mat edgesEroded;
            cv::erode(edgesDilated, edgesEroded, kernel);

            // locate the image contours (after applying a threshold or canny)
            std::vector<std::vector<cv::Point> > contours;
            cv::findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            // draw the rectangles
            for (const auto& contour : contours) {
                cv::Rect boundingBox = cv::boundingRect(contour);
                cv::rectangle(captureFrame, boundingBox, cv::Scalar(0, 0, 255));
            }

            cv::imshow("fgmask", fgMask);
			//cv::imshow(DISPLAY_WINDOW_NAME, captureFrame);
            //std::cout <<  frameCount << std::endl;

            // check for program termination
            if((char) cv::waitKey(1) == 'q')
            {
                doCapture = false;
            }
        }
        else
        {
            std::printf("Unable to acquire image frame! \n");
        }
    }

    // release program resources before returning
    capture.release();
    cv::destroyAllWindows();
}