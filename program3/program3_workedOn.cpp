#include <iostream>
#include <vector>
#include <cstdio>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

bool intersects(const cv::Rect rect1, const cv::Rect rect2) {
    return (rect1 & rect2).area() > 0;
}

int main() {
    // Keeping count of cars
    int countLeftToRight = 0;
    int countRightToLeft = 0;
    
    // Store video capture parameters
    std::string fileName;

    // Validate and parse the command line arguments
    if(argc != NUM_COMNMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <file_path> \n", argv[0]);
        return 0;
    }
    else
    {
        fileName = argv[1];
    }
    
    // Open the video file
    cv::VideoCapture capture("filename");
    if (!capture.isOpened()) {
        std::cerr << "Unable to open video source!" << std::endl;
        return 1;
    }

    // Define parameters for background subtraction
    int bgHistory = 200;
    double bgThreshold = 60.0;
    bool bgShadowDetection = false;

    // Create the background subtractor
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(bgHistory, bgThreshold, bgShadowDetection);

    // Define regions westbound and eastbound
    cv::Rect topRegion(0, capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);
    cv::Rect bottomRegion(0, 0, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);

    // Create kernel for morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));

    // Create the blob detector
    cv::SimpleBlobDetector::Params params;
    params.minThreshold = 10;
    params.maxThreshold = 255;
    params.filterByArea = true;
    params.minArea = 17000;
    params.maxArea = 16000;
    cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);

    // START PROCESSING FRAMES
    cv::Mat frame;
    while (capture.read(frame)) {        
        // Grayscaling and normalizing
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::normalize(grayFrame, grayFrame, 0, 255, cv::NORM_MINMAX, CV_8UC1);
        
        // Apply background subtraction
        cv::Mat fgMask;
        pMOG2->apply(grayFrame, fgMask);

        // Morphological operations (dilation and erosion)
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, kernel);
        
        // Find contours in the foreground mask
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Process each contour
        for (const auto& contour : contours) {
            // Calculate the contour area
            double contourArea = cv::contourArea(contour);

            // Detect blobs
            std::vector<cv::KeyPoint> keypoints;
            detector->detect(fgMask, keypoints);

            // Draw the keypoints
            cv::Mat imageKeypoints;
            cv::drawKeypoints(frame, keypoints, imageKeypoints, cv::Scalar(255,0,0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            
        /* Filter contours based on area threshold
            if (contourArea > 17000 && contourArea < 160000) {
                // Create a bounding box around the contour
                cv::Rect boundingBox = cv::boundingRect(contour);
                
                // Check if the bounding box intersects with the left or right region and draw bounding box for the frame
                if (intersects(boundingBox, topRegion)) {
                    cv::rectangle(frame, boundingBox, cv::Scalar(0, 0, 255), 2);
                } else if (intersects(boundingBox, bottomRegion)) {
                    cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
                }
            }
        */
        }
        
        // Display the frame with bounding boxes and count information
        cv::putText(frame, "Left to Right: " + std::to_string(countLeftToRight), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        cv::putText(frame, "Right to Left: " + std::to_string(countRightToLeft), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        cv::imshow("Vehicle Tracking", origFrame);

        // Check for the 'q' key to quit the program
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Release the video capture and destroy windows
    capture.release();
    cv::destroyAllWindows();

    return 0;
}
