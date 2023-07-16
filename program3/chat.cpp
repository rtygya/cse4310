#include <opencv2/opencv.hpp>

int main() {
    cv::VideoCapture cap("path/to/your/video.mp4");
    if (!cap.isOpened()) {
        std::cout << "Error opening video file" << std::endl;
        return -1;
    }

    cv::Ptr<cv::BackgroundSubtractor> bgSubtractor = cv::createBackgroundSubtractorMOG2();

    while (true) {
        cv::Mat frame;
        cap.read(frame);

        if (frame.empty()) {
            break;
        }

        // Apply background subtraction
        cv::Mat fgMask;
        bgSubtractor->apply(frame, fgMask);

        // Apply morphological operations (dilation and erosion)
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(20, 20));
        cv::dilate(fgMask, fgMask, kernel);
        cv::erode(fgMask, fgMask, kernel);

        // Find contours
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Draw bounding boxes around each contour
        for (const auto& contour : contours) {
            cv::Rect boundingBox = cv::boundingRect(contour);
            cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
        }

        // Display the frame with bounding boxes
        cv::imshow("Moving Objects", frame);

        if (cv::waitKey(30) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
