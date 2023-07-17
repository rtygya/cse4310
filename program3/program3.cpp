#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

bool intersects(const cv::Rect rect1, const cv::Rect rect2) {
    return (rect1 & rect2).area() > 0;
}

int main() {
    // Open the video file
    cv::VideoCapture capture("../road_traffic.mp4");
    if (!capture.isOpened()) {
        std::cerr << "Unable to open video source!" << std::endl;
        return 1;
    }

    // Define parameters for background subtraction
    int bgHistory = 200;
    double bgThreshold = 300.0;
    bool bgShadowDetection = false;

    // Create the background subtractor
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(bgHistory, bgThreshold, bgShadowDetection);

    // Define variables for counting cars and tracking directions
    int countLeftToRight = 0;
    int countRightToLeft = 0;
    cv::Rect leftRegion(0, capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);
    cv::Rect rightRegion(0, 0, capture.get(cv::CAP_PROP_FRAME_WIDTH), capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2);

    // Create structuring element for morphological operations
    cv::Mat structuringElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

    // Start processing frames
    cv::Mat frame;
    while (capture.read(frame)) {
        // Apply background subtraction
        cv::Mat fgMask;
        pMOG2->apply(frame, fgMask);

        // Perform morphological operations (erosion and dilation) on the foreground mask
        cv::dilate(fgMask, fgMask, structuringElement);
        //cv::dilate(fgMask, fgMask, structuringElement);
        cv::erode(fgMask, fgMask, structuringElement);
        
        // Find contours in the foreground mask
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Process each contour
        for (const auto& contour : contours) {
            // Calculate the contour area
            double contourArea = cv::contourArea(contour);
            
            // Filter contours based on area threshold
            if (contourArea > 17000 && contourArea < 160000) {
                // Create a bounding box around the contour
                cv::Rect boundingBox = cv::boundingRect(contour);
                //std::cout << contourArea << std::endl;
                // Draw the bounding box on the frame
                cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);

                // Check if the bounding box intersects with the left or right region
                if (intersects(boundingBox, leftRegion)) {
                    countLeftToRight++;
                } else if (intersects(boundingBox, rightRegion)) {
                    countRightToLeft++;
                }
            }
        }

        // Display the frame with bounding boxes and count information
        cv::putText(frame, "Left to Right: " + std::to_string(countLeftToRight), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        cv::putText(frame, "Right to Left: " + std::to_string(countRightToLeft), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        cv::imshow("Vehicle Tracking", frame;

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
