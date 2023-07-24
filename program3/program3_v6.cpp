#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <file_path" << endl;
        return 0;
    }

    // Open the video file
    VideoCapture cap(argv[1]);

    // Check if the video file is opened successfully
    if (!cap.isOpened()) {
        cout << "Error opening video file" << endl;
        return 1;
    }

    // Initialize variables for traffic counting
    int westbound_count = 0;
    int eastbound_count = 0;

    // Define parameters for background subtraction
    int bgHistory = 200;
    double bgThreshold = 60.0;
    bool bgShadowDetection = false;

    // Create the background subtractor
    cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(bgHistory, bgThreshold, bgShadowDetection);

    // Create the tracker
    Ptr<Tracker> tracker = TrackerCSRT::create();

    // Rest of the code for object detection, bounding box drawing,
    // object tracking, and traffic counting logic
    while (true) {
        Mat frame;
        cap >> frame;

        if (frame.empty()) {
            break;
        }

        Mat grayFrame;
        cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        normalize(grayFrame, grayFrame, 0, 255, cv::NORM_MINMAX, CV_8UC1);

        // Apply background subtraction
        Mat fg_mask;
        pMOG2->apply(grayFrame, fg_mask);

        // find the image edges
        cv::Mat imageEdges;
        const double cannyThreshold1 = 100;
        const double cannyThreshold2 = 200;
        const int cannyAperture = 3;
        cv::Canny(grayFrame, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);
        
        // erode and dilate the edges to remove noise
        int morphologySize = 1; //erode, dilate, and fill
        Mat edgesDilated;
        dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1, -1), morphologySize);
        Mat edgesEroded;
        erode(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1, -1), morphologySize);
        
        // locate the image contours (after applying a threshold or canny)
        vector<vector<Point> > contours;
        //std::vector<int> hierarchy;
        //just finding external contours
        findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

        // Perform blob detection
        SimpleBlobDetector::Params blob_params;
        blob_params.filterByArea = true;
        blob_params.minArea = 1000;
        blob_params.maxArea = 200000;
        Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(blob_params);

        vector<KeyPoint> keypoints;
        detector->detect(fg_mask, keypoints);

        // Create contours from blob keypoints
        //vector<vector<Point>> contours;
        Mat imageKeypoints;
        drawKeypoints(fg_mask, keypoints, imageKeypoints, cv::Scalar(0,0,255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        // for (const auto& keypoint : keypoints) {
        //     int x = static_cast<int>(keypoint.pt.x);
        //     int y = static_cast<int>(keypoint.pt.y);

        //     contours.emplace_back(vector<Point>{Point(x, y)});
            
        // }

        // Object tracking for each detected vehicle
        for (const auto& contour : contours) {
            Rect bounding_rect = boundingRect(contour);

            int x = bounding_rect.x;
            int y = bounding_rect.y;
            int w = bounding_rect.width;
            int h = bounding_rect.height;

            // Initialize the tracker for each new detection
            Rect bbox(x, y, w, h);
            tracker->init(frame, bbox);

            // Update the tracker for subsequent frames
            bool tracking = tracker->update(frame, bbox);

            if (tracking) {
                string direction = (bbox.x < frame.cols / 2) ? "Westbound" : "Eastbound";

                if (direction == "Westbound") {
                    westbound_count++;
                    rectangle(frame, Point(bbox.x, bbox.y), Point(bbox.x + bbox.width, bbox.y + bbox.height), Scalar(0, 255, 0), 2); // Green
                } else if (direction == "Eastbound") {
                    eastbound_count++;
                    rectangle(frame, Point(bbox.x, bbox.y), Point(bbox.x + bbox.width, bbox.y + bbox.height), Scalar(0, 0, 255), 2); // Red
                }
            }
        }

        // Print the updated counts
        cout << "WESTBOUND COUNT: " << westbound_count << endl;
        cout << "EASTBOUND COUNT: " << eastbound_count << endl;

        // Display the frame with the bounding boxes and counts
        imshow("Traffic Counting", fg_mask);

        // Check for the 'q' key to quit the program
        if ((char) cv::waitKey(1) == 'q') {
            break;
        }
    }

    // Release the video capture and destroy windows
    cap.release();
    cv::destroyAllWindows();
}
