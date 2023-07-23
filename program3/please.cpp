#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <path_to_input_video>" << endl;
        return -1;
    }

    // Open the video file
    VideoCapture cap(argv[1]);

    // Check if the video file is opened successfully
    if (!cap.isOpened()) {
        cout << "Error opening video file" << endl;
        return -1;
    }

    // Initialize variables for traffic counting
    int westbound_count = 0;
    int eastbound_count = 0;

    // Create background subtractor
    Ptr<BackgroundSubtractorMOG2> bg_subtractor = createBackgroundSubtractorMOG2();

    // Create the tracker
    Ptr<Tracker> tracker = TrackerKCF::create();

    // Rest of the code for object detection, bounding box drawing,
    // object tracking, and traffic counting logic
    while (true) {
        Mat frame;
        cap >> frame;

        if (frame.empty()) {
            break;
        }

        // Apply background subtraction
        Mat fg_mask;
        bg_subtractor->apply(frame, fg_mask);

        // Perform blob detection
        SimpleBlobDetector::Params blob_params;
        blob_params.filterByArea = true;
        blob_params.minArea = 100;
        blob_params.maxArea = 2000;
        Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(blob_params);

        vector<KeyPoint> keypoints;
        detector->detect(fg_mask, keypoints);

        // Create contours from blob keypoints
        vector<vector<Point>> contours;
        for (const auto& keypoint : keypoints) {
            int x = static_cast<int>(keypoint.pt.x);
            int y = static_cast<int>(keypoint.pt.y);

            contours.emplace_back(vector<Point>{Point(x, y)});
        }

        // Object tracking for each detected vehicle
        for (const auto& contour : contours) {
            Rect bounding_rect = boundingRect(contour);

            int x = bounding_rect.x;
            int y = bounding_rect.y;
            int w = bounding_rect.width;
            int h = bounding_rect.height;

            // Initialize the tracker for each new detection
            Rect2d bbox(x, y, w, h);
            tracker->init(frame, bbox);

            // Update the tracker for subsequent frames
            bool ok = tracker->update(frame, bbox);

            if (ok) {
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
        imshow("Traffic Counting", frame);

        if (waitKey(1) == 27) { // Press 'Esc' to exit
            break;
        }
    }

    return 0;
}
