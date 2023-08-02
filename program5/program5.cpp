/*********************************************************************************************************************
 * @file program5.cpp
 * @brief C++ example of simple screen scraping program using template matching with OpenCV
 * @author Reety Gyawali
 **********************************************************************************************************************/

// include necessary dependencies
#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc_c.h"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 2
#define DISPLAY_WINDOW_NAME "Screen Scraper"

int main(int argc, char **argv)
{
    cv::Mat imageTemplate;
    
    // Store video capture parameters
    std::string filename;

    // Validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <video> <template_image> \n", argv[0]);
        return 0;
    }
    else
    {
        filename = argv[1];
        imageTemplate = cv::imread(argv[2], cv::IMREAD_COLOR);

        // check for file error
        if(!imageTemplate.data)
        {
            std::cout << "Error while opening file " << argv[2] << std::endl;
            return 0;
        }
    }
    
    // Open the video file
    cv::VideoCapture capture(filename);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open video source!" << std::endl;
        return 1;
    }

    // Create window
    cv::namedWindow(DISPLAY_WINDOW_NAME, cv::WINDOW_AUTOSIZE);

    //Show template and convert to grayscale
    cv::namedWindow("Template", cv::WINDOW_AUTOSIZE);
    cv::imshow("Template", imageTemplate);
    // cv::Mat imageTemplateGray;
    // cv::cvtColor(imageTemplate, imageTemplateGray, cv::COLOR_BGR2GRAY);

    /************************************************** START PROCESSING FRAMES *************************************************/
    
    cv::Mat frame;
  
    while (1) {    
        capture >> frame;

        if (frame.empty())
        {
            break;
        }
        // convert frame to grayscale
        cv::Mat frameGray;
        cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);
    
        // perform the matching step and normalize the searchResult
        cv::Mat searchResult;
        int match_method = CV_TM_CCORR_NORMED;
        cv::matchTemplate(frameGray, imageTemplateGray, searchResult, match_method);
        cv::normalize(searchResult, searchResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

        // Find all matches above the threshold
        double threshold = 0.8; 
        std::vector<cv::Point> matchLocations;
        cv::findNonZero(searchResult > threshold, matchLocations);

        // Draw rectangles around all matching objects
        for (const auto& matchLocation : matchLocations) {
            cv::Point topLeft(matchLocation.x, matchLocation.y);
            cv::Point bottomRight(matchLocation.x + imageTemplateGray.cols, matchLocation.y + imageTemplateGray.rows);
            cv::rectangle(frame, topLeft, bottomRight, cv::Scalar(0, 255, 0), 1);
        }
    
        /* find the location of the best fit
        double minVal;
        double maxVal;
        cv::Point minLocation;
        cv::Point maxLocation;
        cv::Point matchLocation;
        cv::minMaxLoc(searchResult, &minVal, &maxVal, &minLocation, &maxLocation, cv::Mat());
        if(match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
        {
            matchLocation = minLocation;
        }
        else
        {
            matchLocation = maxLocation;
        }
    
        // annotate the scene image
        cv::rectangle(frame, matchLocation, cv::Point(matchLocation.x + imageTemplate.cols , matchLocation.y + imageTemplate.rows), CV_RGB(0,255,0), 1);
        */
        // display the frame
        cv::imshow(DISPLAY_WINDOW_NAME, frame);
        
        // Check for the 'q' key to quit the program
        if ((char) cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Release the video capture and destroy windows
    capture.release();
    cv::destroyAllWindows();

    return 0;
}
