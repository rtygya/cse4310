/*******************************************************************************************************************//**
 * @file program5.cpp
 * @brief C++ example for template matching with OpenCV
 * @author Reety Gyawali
 **********************************************************************************************************************/

// include necessary dependencies
#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc_c.h"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 2

int main(int argc, char **argv)
{
    cv::Mat imageScene;
    cv::Mat imageTemplate;

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <scene_image> <template_image> \n", argv[0]);
        return 0;
    }
    else
    {
        imageScene = cv::imread(argv[1], cv::IMREAD_COLOR);
        imageTemplate = cv::imread(argv[2], cv::IMREAD_COLOR);

        // check for file error
        if(!imageScene.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
        if(!imageTemplate.data)
        {
            std::cout << "Error while opening file " << argv[2] << std::endl;
            return 0;
        }
    }

    // convert the images to grayscale
    cv::Mat imageSceneGray;
    cv::cvtColor(imageScene, imageSceneGray, cv::COLOR_BGR2GRAY);
    cv::Mat imageTemplateGray;
    cv::cvtColor(imageTemplate, imageTemplateGray, cv::COLOR_BGR2GRAY);

    // perform the matching step and normalize the searchResult
    cv::Mat searchResult;
    int match_method = CV_TM_CCORR_NORMED;
    cv::matchTemplate(imageSceneGray, imageTemplateGray, searchResult, match_method);
    cv::normalize(searchResult, searchResult, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

    // find the location of the best fit
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
    cv::rectangle(imageScene, matchLocation, cv::Point(matchLocation.x + imageTemplate.cols , matchLocation.y + imageTemplate.rows), CV_RGB(255,0,0), 3);

    // display the images
    cv::imshow("imageScene", imageScene);
    cv::imshow("imageTemplate", imageTemplate);
    cv::imshow("searchResult", searchResult);
    cv::waitKey();

    return 0;
}
