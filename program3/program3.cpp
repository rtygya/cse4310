#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

// configuration parameters
#define NUM_COMNMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME "Vehicle Tracking"

bool intersects(const cv::Rect rect1, const cv::Rect rect2) {
    return (rect1 & rect2).area() > 0;
}

int main(int argc, char **argv) {
    // Keeping count of cars
    int countLeftToRight = 0;
    int countRightToLeft = 0;
    
    // Store video capture parameters
    std::string filename;

    // Validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <file_path> \n", argv[0]);
        return 0;
    }
    else
    {
        filename = argv[1];
    }
    
    // Open the video file
    cv::VideoCapture capture(filename);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open video source!" << std::endl;
        return 1;
    }

    // // Define parameters for background subtraction
    // int bgHistory = 200;
    // double bgThreshold = 60.0;
    // bool bgShadowDetection = false;

    // // Create the background subtractor
    // cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(bgHistory, bgThreshold, bgShadowDetection);

    // Define regions westbound and eastbound
    cv::Rect bottomRegion(0, capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);
    cv::Rect topRegion(0, 0, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);

    // Create kernel for morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7,7));

    // Create the blob detector
    cv::SimpleBlobDetector::Params params;
    params.minThreshold = 0;
    params.maxThreshold = 255;
    params.minRepeatability = 2;
    params.filterByArea = true;
    params.minArea = 10000;
    params.maxArea = 250000;
    params.blobColor = 255;
    params.filterByCircularity = false;
    params.filterByConvexity = true;
    params.minConvexity = 0.10;
    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

    // Create window
    cv::namedWindow(DISPLAY_WINDOW_NAME, cv::WINDOW_AUTOSIZE);

    /************************************************** START PROCESSING FRAMES *************************************************/
    
    cv::Mat frame;
    cv::Mat bgFrame;

    //  Reset frame number to 0 to capture background frame
    capture.set(cv::CAP_PROP_POS_FRAMES, 0);
    capture >> bgFrame;
    //cv::normalize(bgFrame, bgFrame, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    cv::cvtColor(bgFrame, bgFrame, cv::COLOR_BGR2GRAY);
    
    cv::GaussianBlur(bgFrame, bgFrame, cv::Size(5, 5), 0);

    // Create the tracker
   cv::Ptr<cv::Tracker> tracker = cv::TrackerCSRT::create();

   int frameCount = 0;
  
    while (1) {    
        capture >> frame;

        if (frame.empty())
        {
            break;
        }
        
        // Grayscaling and normalizing
        cv::Mat grayFrame;
        //cv::normalize(grayFrame, grayFrame, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        
        cv::GaussianBlur(grayFrame, grayFrame, cv::Size(5, 5), 0);
        
        // // Apply background subtraction
        // cv::Mat fgMask;
        // pMOG2->apply(grayFrame, fgMask);

        // Calculate absolute difference of current frame and the background frame
        cv::Mat fgMask;
        cv::absdiff(grayFrame, bgFrame, fgMask);
        
        // Threshold to binarize
        cv::threshold(fgMask, fgMask, 30, 255, cv::THRESH_BINARY);

        // Morphological operations (dilation and erosion)
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
        
        // Find contours in the foreground mask
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Detect blobs in a frame
        std::vector<cv::KeyPoint> keypoints;
        detector->detect(fgMask, keypoints);


        // Process each contour
        for (const auto& contour : contours) {

            // Calculate the contour area
            double contourArea = cv::contourArea(contour);
            
            // Filter contours based on area threshold
            if (contourArea > 17000 && contourArea < 160000) {
                // Create a bounding box around the contour
                cv::Rect boundingBox = cv::boundingRect(contour);
                
                // Check if the bounding box intersects with the top and bottom region and draw bounding box for the frame
                if (intersects(boundingBox, bottomRegion)) {
                    cv::rectangle(frame, boundingBox, cv::Scalar(0, 0, 255), 2);
                    if (frameCount % 35 == 0 )
                    {
                        countLeftToRight++;
                    }
                    
                } else if (intersects(boundingBox, topRegion)) {
                    cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
                    if (frameCount % 35 == 0 )
                    {
                        countRightToLeft++;
                    }
                }
                
            }
        
        }
        
        // Print the updated counts
        std::cout << "WESTBOUND COUNT: " << countRightToLeft << std::endl;
        std::cout << "EASTBOUND COUNT: " << countLeftToRight << std::endl;
        //std::cout << frameCount << std::endl;
        cv::imshow(DISPLAY_WINDOW_NAME, frame);
        frameCount++;

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
