#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;

int main() {
    // Open the video file
    VideoCapture capture("input_video.mp4");
    if (!capture.isOpened()) {
        std::cerr << "Unable to open video source!" << std::endl;
        return 1;
    }

    // Create the background subtractor
    Ptr<BackgroundSubtractorMOG2> pMOG2 = createBackgroundSubtractorMOG2();

    // Create the blob detector
    SimpleBlobDetector::Params params;
    params.filterByArea = true;
    params.minArea = 100;
    params.maxArea = 5000;
    Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

    // Create the CRST tracker
    Ptr<TrackerCRST> tracker = TrackerCRST::create();

    // Create variables for counting vehicles
    int vehicleCount = 0;

    // Process each frame
    Mat frame, fgMask;
    while (capture.read(frame)) {
        // Apply background subtraction
        pMOG2->apply(frame, fgMask);

        // Perform morphological operations
        Mat morphed;
        morphologyEx(fgMask, morphed, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        // Find contours
        std::vector<std::vector<Point>> contours;
        findContours(morphed, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Process each contour
        for (const auto& contour : contours) {
            // Detect blobs
            std::vector<KeyPoint> keypoints;
            detector->detect(frame, keypoints);

            // Instantiate CRST tracker for each blob
            for (const auto& keypoint : keypoints) {
                Rect boundingBox(keypoint.pt.x - keypoint.size / 2, keypoint.pt.y - keypoint.size / 2, keypoint.size, keypoint.size);
                tracker->init(frame, boundingBox);
            }
        }

        // Update and draw bounding rectangles
        std::vector<Rect> boundingBoxes;
        tracker->update(frame, boundingBoxes);
        for (const auto& boundingBox : boundingBoxes) {
            rectangle(frame, boundingBox, Scalar(0, 255, 0), 2);
        }

        // Count the vehicles
        vehicleCount = boundingBoxes.size();

        // Display the frame with bounding rectangles and vehicle count
        putText(frame, "Vehicle Count: " + std::to_string(vehicleCount), Point(10, 30), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 0), 2);
        imshow("Vehicle Tracking", frame);

        // Check for the 'q' key to quit the program
        if (waitKey(1) == 'q') {
            break;
        }
    }

    // Release the video capture and destroy windows
    capture.release();
    destroyAllWindows();

    return 0;
}
