
// include necessary dependencies
#include <iostream>
#include <cstdio>
#include "opencv2/opencv.hpp"

// configuration parameters
#define NUM_COMMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME "Hello world!"

cv::Mat imageIn;
cv::Mat imageOrig;
int mode = 0;
cv::Vec3b eyedropper_val(255,255,255);
bool draw = false;
bool topLeft = false;
cv::Point tl;
cv::Point br;

//Utility function for paint bucket does this, this, and this
void fill(int x, int y, cv::Vec3b oldColor, cv::Vec3b newColor)
{
	if (x >= imageIn.size().width || y >= imageIn.size().height || x < 0 || y < 0)
	{
		return;
	}
	else if (imageIn.at<cv::Vec3b>(y, x) != oldColor)
	{
		return;
	}
	else if (imageIn.at<cv::Vec3b>(y, x) == newColor)
	{
		return;
	}

	imageIn.at<cv::Vec3b>(y, x) = newColor;

	fill(x+1, y, oldColor, newColor);
	fill(x-1, y, oldColor, newColor);
	fill(x, y+1, oldColor, newColor);
	fill(x, y-1, oldColor, newColor);
}

static void clickCallback(int event, int x, int y, int flags, void* param)
{
    // cast userdata to a cv::Mat
    imageIn = *(cv::Mat *)param;
    
    // handle the mouse event types
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
		//Eyedropper    
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			eyedropper_val = imageIn.at<cv::Vec3b>(y, x);
        	std::cout << eyedropper_val << std::endl;
		} 
    }
    else if (mode == 2) 
    {
		//Crop
		if (event == cv::EVENT_LBUTTONDOWN)
		{
			//Record click location
			tl.x = x;
			tl.y = y;
		}
		else if (event == cv::EVENT_LBUTTONUP)
		{
			//Record release location
			br.x = x;
			br.y = y;
			//Extract, copy, and display rectangle ROI
			cv::Rect region(tl, br);
			cv::Mat imageROI = imageIn(region);
			imageROI.copyTo(imageIn);
			cv::imshow(DISPLAY_WINDOW_NAME, imageIn);
			
		}
    }
    else if(mode == 3)
    {
		//Pencil
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
		//Paint bucket
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
		//Reset
		//On left double click, replace image with original and display
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
