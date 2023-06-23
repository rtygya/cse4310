/*******************************************************************************************************************//**
 * @file program1.cpp
 * @brief C++ example of basic image manipulation program using OpenCV
 * @author Reety Gyawali (1001803756)
 **********************************************************************************************************************/

// Include necessary dependencies
#include <iostream>
#include <cstdio>
#include "opencv2/opencv.hpp"

// Configuration parameters
#define NUM_COMMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME "Hello world!"

cv::Mat imageIn;
cv::Mat imageOrig;
int mode = 0;
// Default eyedropper value is white
cv::Vec3b eyedropper_val(255,255,255);
bool draw = false;
bool topLeft = false;
cv::Point tl;
cv::Point br;

// Utility function for paint bucket that recursively converts target pixel and surrounding pixels to new color
void fill(int x, int y, cv::Vec3b oldColor, cv::Vec3b newColor)
{
	if (x >= imageIn.size().width || y >= imageIn.size().height || x < 0 || y < 0)
	{
		// Exit when coordinates exceed image dimensions
		return;
	}
	else if (imageIn.at<cv::Vec3b>(y, x) != oldColor)
	{
		// Exit if target pixel is not old color
		return;
	}
	else if (imageIn.at<cv::Vec3b>(y, x) == newColor)
	{
		// Exit is target pixel is already the new color
		return;
	}

	// Change color of target pixel to new color
	imageIn.at<cv::Vec3b>(y, x) = newColor;

	// Calling function on right, left, top, and bottom pixels
	fill(x+1, y, oldColor, newColor);
	fill(x-1, y, oldColor, newColor);
	fill(x, y+1, oldColor, newColor);
	fill(x, y-1, oldColor, newColor);
}

static void clickCallback(int event, int x, int y, int flags, void* param)
{
    // Image is now the image user clicked on
    imageIn = *(cv::Mat *)param;
    
    /*
		Right button down toggles through 5 modes
		Mode 1 --> Eyedropper
		Mode 2 --> Crop
		Mode 3 --> Pencil
		Mode 4 --> Paint bucket
		Mode 5 --> Reset
	*/ 

    if (event == cv::EVENT_RBUTTONDOWN)
    {        
        if (mode == 5)
		{
			mode = 1;
		}
		else
		{
			mode++;
		}

		switch (mode) {
			case 1:
				std::cout << "Eyedropper activated" << std::endl;
				break;
			case 2:
				std::cout << "Crop activated" << std::endl;
				break;
			case 3:
				std::cout << "Pencil activated" << std::endl;
				break;
			case 4:
				std::cout << "Paint bucket activated" << std::endl;
				break;
			case 5:
				std::cout << "Reset activated" << std::endl;
				break;
			default:
				std::cout << "Right click to toggle" << std::endl;
				break;
		}
    }
    else if (mode == 1)
    {    
		// Eyedropper    
		// On left click, store eyedropper value as current pixel's color
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			eyedropper_val = imageIn.at<cv::Vec3b>(y, x);
        	std::cout << eyedropper_val << std::endl;
		} 
    }
    else if (mode == 2) 
    {
		// Crop
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			// Record click location
			tl.x = x;
			tl.y = y;
		}
		else if (event == cv::EVENT_LBUTTONUP)
		{
			// Record release location
			br.x = x;
			br.y = y;
			// Extract, replace current image with rectangle ROI, and display 
			cv::Rect region(tl, br);
			cv::Mat imageROI = imageIn(region);
			imageROI.copyTo(imageIn);
			cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
			
		}
    }
    else if (mode == 3)
    {
		// Pencil
		// When left button is down and moving, target pixels are changed to eyedropper value 
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			draw = true;
			imageIn.at<cv::Vec3b>(y, x) = eyedropper_val;
			cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
		}
		else if (event == cv::EVENT_MOUSEMOVE)
		{
			if (draw == true)
			{
				imageIn.at<cv::Vec3b>(y, x) = eyedropper_val;
				cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
			}
		}
		else if (event == cv::EVENT_LBUTTONUP)
		{
			draw = false;
		}
    }
	else if (mode == 4)
	{
		// Paint bucket
		// On left button click, if target pixel is not the eyedropper value, fill the area with eyedropper value and display
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			cv::Vec3b oldColor = imageIn.at<cv::Vec3b>(y, x);
			if (oldColor != eyedropper_val)
			{
				fill(x, y, oldColor, eyedropper_val);
			}
			cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
		}
	}
	else if (mode == 5)
	{
		// Reset
		// On left double click, replace image with original and display
		if (event == cv::EVENT_LBUTTONDBLCLK)
		{
			imageOrig.copyTo(imageIn);
			cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
		}
	}
}

int main(int argc, char **argv)
{
    // Validate and parse the command line arguments
    if(argc != NUM_COMMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
		imageIn = cv::imread(argv[1], cv::IMREAD_COLOR);
		
		// Check for file error
		if(!imageIn.data)
		{
			std::cout << "Error while opening file " << argv[1] << std::endl;
			return 0;
		}
		else
		{
			imageOrig = imageIn.clone();
			cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
			cv::setMouseCallback(DISPLAY_WINDOW_NAME, clickCallback, &imageIn);
			std::cout << "Right click to toggle" << std::endl;
			cv::waitKey();
		}
    }
}
