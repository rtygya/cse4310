//pupil tracker code
//run canny
//remove noise dilate then erode
//run countour on edges to find shape
//fit a rectangle/ellipse

//make sure images are consistent not 45 degrees

/*******************************************************************************************************************//**
 * @file cv_ellipse.cpp
 * @brief C++ example of Canny edge detection and ellipse model fitting in OpenCV
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/

// include necessary dependencies
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1

/*******************************************************************************************************************//**
 * @brief program entry point
 * @param[in] argc number of command line arguments
 * @param[in] argv string array of command line arguments
 * @return return code (0 for normal termination)
 * @author Christoper D. McMurrough
 **********************************************************************************************************************/
int main(int argc, char **argv)
{
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

    // get the image size
    std::cout << "image width: " << imageIn.size().width << std::endl;
    std::cout << "image height: " << imageIn.size().height << std::endl;
    std::cout << "image channels: " << imageIn.channels() << std::endl;

    // convert the image to grayscale
    cv::Mat imageGray;
    cv::cvtColor(imageIn, imageGray, cv::COLOR_BGR2GRAY);

    // find the image edges
    cv::Mat imageEdges;
    const double cannyThreshold1 = 100;
    const double cannyThreshold2 = 200;
    const int cannyAperture = 3;
    cv::Canny(imageGray, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);
    
    // erode and dilate the edges to remove noise
    int morphologySize = 1; //erode, dilate, and fill
    cv::Mat edgesDilated;
    cv::dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1, -1), morphologySize);
    cv::Mat edgesEroded;
    cv::erode(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1, -1), morphologySize);
    
    // locate the image contours (after applying a threshold or canny)
    std::vector<std::vector<cv::Point> > contours;
    //std::vector<int> hierarchy;
    //just finding external contours
    cv::findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // draw the contours
    cv::Mat imageContours = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    cv::RNG rand(12345);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::drawContours(imageContours, contours, i, color);
    }

    // compute minimum area bounding rectangles
    std::vector<cv::RotatedRect> minAreaRectangles(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute a minimum area bounding rectangle for the contour
        minAreaRectangles[i] = cv::minAreaRect(contours[i]);
    }

    // draw the rectangles
    cv::Mat imageRectangles = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    for(int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
        cv::Point2f rectanglePoints[4];
        minAreaRectangles[i].points(rectanglePoints);
        for(int j = 0; j < 4; j++)
        {
            cv::line(imageRectangles, rectanglePoints[j], rectanglePoints[(j+1) % 4], color);
        }
    }

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
    cv::Mat imageEllipse = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    const int minEllipseInliers = 50;
    for(int i = 0; i < contours.size(); i++)
    {
        // draw any ellipse with sufficient inliers
        if(contours.at(i).size() > minEllipseInliers)
        {
            std::cout <<  contours.at(i).size() << std::endl;

            cv::Scalar color = cv::Scalar(rand.uniform(0, 256), rand.uniform(0,256), rand.uniform(0,256));
            cv::ellipse(imageEllipse, fittedEllipses[i], color, 2);
        }
    }

    // display the images
    cv::imshow("imageIn", imageIn);
    cv::imshow("imageGray", imageGray);
    cv::imshow("imageEdges", imageEdges);
    cv::imshow("edges dilated", edgesDilated);
    cv::imshow("edges eroded", edgesEroded);
    cv::imshow("imageContours", imageContours);
    cv::imshow("imageRectangles", imageRectangles);
    cv::imshow("imageEllipse", imageEllipse);
        
    cv::waitKey();
}

/* THIS IS THE CHATGPT CODE

#include <opencv2/opencv.hpp>
#include <iostream>
#include <iomanip>

int main() {
    // ... Previous code to read the input image and find contours ...

    // Define size ranges for each coin type (in pixels)
    double pennySizeMin = 15;
    double pennySizeMax = 18;
    double nickelSizeMin = 20;
    double nickelSizeMax = 23;
    double dimeSizeMin = 22;
    double dimeSizeMax = 25;
    double quarterSizeMin = 24;
    double quarterSizeMax = 27;

    // Draw ellipse contours and classify coins
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() >= 5) {
            cv::RotatedRect ellipse = cv::fitEllipse(contours[i]);
            cv::ellipse(image, ellipse, cv::Scalar(0, 255, 0), 2);

            // Measure the radius of the fitted ellipse (average of width and height)
            double radius = (ellipse.size.width + ellipse.size.height) / 4.0;

            // Classify the coin based on its radius
            std::string coinType;
            if (radius >= pennySizeMin && radius <= pennySizeMax) {
                coinType = "Penny";
            } else if (radius >= nickelSizeMin && radius <= nickelSizeMax) {
                coinType = "Nickel";
            } else if (radius >= dimeSizeMin && radius <= dimeSizeMax) {
                coinType = "Dime";
            } else if (radius >= quarterSizeMin && radius <= quarterSizeMax) {
                coinType = "Quarter";
            } else {
                coinType = "Unknown";
            }

            // Display the coin type on the image
            std::stringstream ss;
            ss << coinType << " - " << std::fixed << std::setprecision(2) << radius << " pixels";
            cv::putText(image, ss.str(), cv::Point(ellipse.center.x, ellipse.center.y + 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }
    }

    // ... Display and wait for user input as before ...

    return 0;
}
*/
