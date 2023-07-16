
/*******************************************************************************************************************//**
 * @file program2.cpp
 * @brief C++ example of coin sorting/counting program using Canny edge detection and ellipse model fitting in OpenCV 
 * @author Reety Gyawali 1001803756
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1


int main(int argc, char **argv)
{
    // variables to track coin count and sizes (in pixels)
    int quarter = 0;
    int nickel = 0;
    int penny = 0;
    int dime = 0;
    double pennySizeMin = 44;
    double pennySizeMax = 50;
    double nickelSizeMin = 50;
    double nickelSizeMax = 57;
    double dimeSizeMin = 40;
    double dimeSizeMax = 44;
    double quarterSizeMin = 57;
    double quarterSizeMax = 60;
    
    cv::Mat imageIn;

    // validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1) 
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
        imageIn = cv::imread(argv[1], cv::IMREAD_COLOR);

        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
    }

    // convert the image to grayscale
    cv::Mat imageGray;
    cv::cvtColor(imageIn, imageGray, cv::COLOR_BGR2GRAY);

    // find the image edges
    cv::Mat imageEdges;
    cv::Canny(imageGray, imageEdges, 100, 200, 3);
    
    // erode and dilate the edges to remove noise
    int morphologySize = 1; 
    cv::Mat edgesDilated;
    cv::dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1, -1), morphologySize);
    cv::Mat edgesEroded;
    cv::erode(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1, -1), morphologySize);
    
    // locate the image contours (after applying a threshold or canny)
    std::vector<std::vector<cv::Point> > contours;
    //std::vector<int> hierarchy;
    //just finding external contours
    cv::findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // fit ellipses to contours containing sufficient inliers
    std::vector<cv::RotatedRect> fittedEllipses(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
        if(contours.at(i).size() > 5)
        {
            fittedEllipses[i] = cv::fitEllipse(contours[i]);
        }
    }

    // draw the ellipses
    cv::Mat imageEllipse = imageIn.clone();
    const int minEllipseInliers = 50;
    for(int i = 0; i < contours.size(); i++)
    {
        // draw any ellipse with sufficient inliers
        if(contours.at(i).size() > minEllipseInliers)
        {
            cv::Scalar color = cv::Scalar(0, 0, 0);
            // Measure the radius of the fitted ellipse (average of width and height)
            double radius = (fittedEllipses[i].size.width + fittedEllipses[i].size.height) / 4.0;

            //Use radius to sort into 4 categories (quarter, nickel, penny, dime)
            if (radius >= quarterSizeMin && radius <= quarterSizeMax) {
                quarter++;
                color = cv::Scalar(0, 255, 0);
            } else if (radius >= nickelSizeMin && radius <= nickelSizeMax) {
                nickel++;
                color = cv::Scalar(0, 255, 255);
            } else if (radius >= pennySizeMin && radius <=pennySizeMax) {
                penny++;
                color = cv::Scalar(0, 0, 255);
            } else if (radius >= dimeSizeMin && radius <= dimeSizeMax) {
                dime++;
                color = cv::Scalar(255, 0, 0);
            } else {
                //unknown
            }
            cv::ellipse(imageEllipse, fittedEllipses[i], color, 2);
        }
    }

    // display the images in two separate windows
    cv::imshow("imageIn", imageIn);
    cv::imshow("imageEllipse", imageEllipse);

    //Print coin count / total value to console
    std::cout <<  "Penny - " << penny << std::endl;
    std::cout <<  "Nickel - " << nickel << std::endl;
    std::cout <<  "Dime - " << dime << std::endl;
    std::cout <<  "Quarter - " << quarter << std::endl;
    double total = (0.01 * penny) + (0.05 * nickel) + (0.10 * dime) + (0.25 * quarter);
    std::cout <<  "Total - $" << total << std::endl;

    std::cout <<  "\nPress any key to terminate." << std::endl;
    cv::waitKey();
    
    cv::destroyAllWindows();
    return 0;
}

